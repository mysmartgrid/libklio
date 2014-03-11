#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/sqlite3/transaction.hpp>
#include "sqlite3-store.hpp"


using namespace klio;

const klio::SensorFactory::Ptr SQLite3Store::sensor_factory(new klio::SensorFactory());
const klio::TimeConverter::Ptr SQLite3Store::time_converter(new klio::TimeConverter());

void SQLite3Store::open() {

    int rc = sqlite3_open(_path.c_str(), &db);
    if (rc) {
        std::ostringstream oss;
        oss << "Can't open database: ";

        if (db) {
            oss << sqlite3_errmsg(db);
            close();

        } else {
            oss << "Not enough memory.";
        }
        throw StoreException(oss.str());
    }
}

void SQLite3Store::close() {

    if (db) {
        sqlite3_close(db);
    }
}

void SQLite3Store::start_transaction() {

    if (_sub_transactions++ == 0) {
        _transaction = klio::Transaction::Ptr(new Transaction(db));
    }
}

void SQLite3Store::commit_transaction() {

    if (--_sub_transactions == 0) {
        _transaction->commit();
    }
}

void SQLite3Store::check_integrity() {

    std::ostringstream oss;

    try {
        if (!bfs::exists(_path)) {
            oss << _path << " does not exist.";

        } else if (bfs::is_directory(_path)) {
            oss << _path << " is a directory.";

        } else if (!bfs::is_regular_file(_path)) {
            oss << _path << " is not a regular file.";

        } else if (bfs::file_size(_path) == 0) {
            oss << _path << " is an empty file.";

        } else {
            sqlite3_stmt* stmt = prepare("PRAGMA integrity_check;");
            execute(stmt, SQLITE_ROW);
            std::string result = std::string((char*) sqlite3_column_text(stmt, 0));
            finalize(stmt);

            if (result == "ok") {
                return;

            } else {
                oss << "Database is corrupt.";
            }
        }

    } catch (const bfs::filesystem_error& ex) {
        oss << ex.what();
    }
    throw StoreException(oss.str());
}

void SQLite3Store::initialize() {

    //Create table sensors if it does not exist
    sqlite3_stmt* stmt = prepare("CREATE TABLE IF NOT EXISTS sensors(uuid VARCHAR(16) PRIMARY KEY, external_id VARCHAR(32), name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INTEGER, device_type_id INTEGER);");
    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    insert_sensor_stmt = prepare("INSERT INTO sensors (uuid, external_id, name, description, unit, timezone, device_type_id) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);");
    remove_sensor_stmt = prepare("DELETE FROM sensors WHERE uuid = ?1");
    update_sensor_stmt = prepare("UPDATE sensors SET external_id = ?2, name = ?3, description = ?4, unit = ?5, timezone = ?6, device_type_id = ?7 WHERE uuid = ?1;");
    select_sensor_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE uuid = ?1;");
    select_sensor_by_external_id_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE external_id = ?1;");
    select_sensor_by_name_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE name = ?1;");
    select_sensors_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors;");
    select_all_sensor_uuids_stmt = prepare("SELECT uuid FROM sensors;");
}

void SQLite3Store::dispose() {

    finalize(insert_sensor_stmt);
    finalize(remove_sensor_stmt);
    finalize(update_sensor_stmt);
    finalize(select_sensor_stmt);
    finalize(select_sensor_by_external_id_stmt);
    finalize(select_sensor_by_name_stmt);
    finalize(select_sensors_stmt);
    finalize(select_all_sensor_uuids_stmt);

    close();
    bfs::remove(_path);
}

const std::string SQLite3Store::str() {

    std::ostringstream oss;
    oss << "SQLite3 database, stored in file " << _path;
    return oss.str();
};

void SQLite3Store::add_sensor(klio::Sensor::Ptr sensor) {

    LOG("Adding sensor: " << sensor->str());

    std::ostringstream oss;
    oss << "CREATE TABLE '" << sensor->uuid_string() << "'(timestamp INTEGER PRIMARY KEY, value DOUBLE);";

    start_transaction();

    sqlite3_bind_text(insert_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insert_sensor_stmt, 7, sensor->device_type()->id());

    execute(insert_sensor_stmt, SQLITE_DONE);
    reset(insert_sensor_stmt);

    sqlite3_stmt* stmt = prepare(oss.str());
    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    commit_transaction();
}

void SQLite3Store::remove_sensor(const klio::Sensor::Ptr sensor) {

    LOG("Removing sensor: " << sensor->str());

    std::ostringstream oss;
    oss << "DROP TABLE '" << sensor->uuid_string() << "'";

    start_transaction();

    sqlite3_bind_text(remove_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    execute(remove_sensor_stmt, SQLITE_DONE);
    reset(remove_sensor_stmt);

    sqlite3_stmt* stmt = prepare(oss.str());
    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    commit_transaction();
}

void SQLite3Store::update_sensor(klio::Sensor::Ptr sensor) {

    LOG("Updating sensor: " << sensor->str());

    start_transaction();

    sqlite3_bind_text(update_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(update_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(update_sensor_stmt, 7, sensor->device_type()->id());

    execute(update_sensor_stmt, SQLITE_DONE);
    reset(update_sensor_stmt);

    commit_transaction();
}

klio::Sensor::Ptr SQLite3Store::get_sensor(const klio::Sensor::uuid_t& uuid) {

    LOG("Attempting to load sensor " << uuid);

    sqlite3_bind_text(select_sensor_stmt, 1, boost::uuids::to_string(uuid).c_str(), -1, SQLITE_TRANSIENT);
    execute(select_sensor_stmt, SQLITE_ROW);
    klio::Sensor::Ptr sensor = parse_sensor(select_sensor_stmt);
    reset(select_sensor_stmt);

    return sensor;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors_by_external_id(const std::string& external_id) {

    LOG("Attempting to load sensors " << external_id);

    std::vector<klio::Sensor::Ptr> sensors;
    sqlite3_bind_text(select_sensor_by_external_id_stmt, 1, external_id.c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(select_sensor_by_external_id_stmt)) {
        sensors.push_back(parse_sensor(select_sensor_by_external_id_stmt));
    }
    reset(select_sensor_by_external_id_stmt);

    return sensors;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors_by_name(const std::string& name) {

    LOG("Attempting to load sensors " << name);

    std::vector<klio::Sensor::Ptr> sensors;
    sqlite3_bind_text(select_sensor_by_name_stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(select_sensor_by_name_stmt)) {
        sensors.push_back(parse_sensor(select_sensor_by_name_stmt));
    }
    reset(select_sensor_by_name_stmt);

    return sensors;
}

std::vector<klio::Sensor::uuid_t> SQLite3Store::get_sensor_uuids() {

    LOG("Retrieving UUIDs from store.");

    std::vector<klio::Sensor::uuid_t> uuids;

    while (SQLITE_ROW == sqlite3_step(select_all_sensor_uuids_stmt)) {

        const unsigned char* select_uuid = sqlite3_column_text(select_all_sensor_uuids_stmt, 0);
        boost::uuids::uuid u;
        std::stringstream ss;
        ss << select_uuid;
        ss >> u;
        uuids.push_back(u);
    }
    reset(select_all_sensor_uuids_stmt);

    return uuids;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors() {

    LOG("Attempting to load sensors");

    std::vector<klio::Sensor::Ptr> sensors;

    while (SQLITE_ROW == sqlite3_step(select_sensors_stmt)) {
        sensors.push_back(parse_sensor(select_sensors_stmt));
    }
    reset(select_sensors_stmt);

    return sensors;
}

void SQLite3Store::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    LOG("Adding to sensor: " << sensor->str() << " time=" << timestamp << " value=" << value);

    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";

    start_transaction();

    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        insert_reading_record(stmt, timestamp, value);

    } catch (klio::StoreException e) {
        finalize(stmt);
        throw;
    }
    finalize(stmt);

    commit_transaction();
}

void SQLite3Store::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Adding " << readings->size() << " readings to sensor: " << sensor->str());

    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";

    start_transaction();

    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

            insert_reading_record(stmt, (*it).first, (*it).second);
        }
    } catch (klio::StoreException e) {
        finalize(stmt);
        throw;
    }
    finalize(stmt);

    commit_transaction();
}

void SQLite3Store::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    std::ostringstream oss;
    oss << "INSERT OR REPLACE INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";

    start_transaction();

    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

            insert_reading_record(stmt, (*it).first, (*it).second);
        }

    } catch (klio::StoreException e) {
        finalize(stmt);
        throw;
    }
    finalize(stmt);

    commit_transaction();
}

void SQLite3Store::insert_reading_record(sqlite3_stmt* stmt, timestamp_t timestamp, double value) {

    sqlite3_bind_int(stmt, 1, time_converter->convert_to_epoch(timestamp));
    sqlite3_bind_double(stmt, 2, value);
    execute(stmt, SQLITE_DONE);
    reset(stmt);
}

readings_t_Ptr SQLite3Store::get_all_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving all readings of sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "';";

    readings_t_Ptr readings(new readings_t());
    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        while (SQLITE_ROW == sqlite3_step(stmt)) {

            int epoch = sqlite3_column_int(stmt, 0);
            double value = sqlite3_column_double(stmt, 1);
            readings->insert(
                    std::pair<timestamp_t, double>(
                    time_converter->convert_from_epoch(epoch),
                    value
                    ));
        }
        finalize(stmt);

    } catch (klio::StoreException e) {
        finalize(stmt);
        throw;
    }
    return readings;
}

unsigned long int SQLite3Store::get_num_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT count(*) FROM '" << sensor->uuid_string() << "';";

    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        int num = sqlite3_step(stmt) == SQLITE_ROW ? sqlite3_column_int(stmt, 0) : 0;
        finalize(stmt);
        return num;

    } catch (std::exception const& e) {
        finalize(stmt);
        throw;
    }
}

reading_t SQLite3Store::get_last_reading(klio::Sensor::Ptr sensor) {

    LOG("Retrieving last readings of sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' " <<
            "ORDER BY timestamp DESC LIMIT 1;";

    std::pair<timestamp_t, double> reading;
    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        if (sqlite3_step(stmt) == SQLITE_ROW) {

            reading = std::pair<timestamp_t, double>(
                    time_converter->convert_from_epoch(sqlite3_column_int(stmt, 0)),
                    sqlite3_column_double(stmt, 1)
                    );
        }
        finalize(stmt);
        return reading;

    } catch (std::exception const& e) {
        finalize(stmt);
        throw;
    }
}

sqlite3_stmt *SQLite3Store::prepare(const std::string& stmt_str) {

    sqlite3_stmt* stmt;
    const char* pz_tail;

    int rc = sqlite3_prepare_v2(
            db, //Database handle
            stmt_str.c_str(), //SQL statement, UTF-8 encoded
            -1, //Maximum length of zSql in bytes. -1 means: read complete string.
            &stmt, //OUT: Statement handle
            &pz_tail //OUT: Pointer to unused portion of zSql
            );

    if (rc != SQLITE_OK) {

        std::ostringstream oss;
        oss << "Can't prepare SQL statement: " << stmt_str << ". Error: " << sqlite3_errmsg(db) << ", Error code: " << rc;
        finalize(stmt);
        throw StoreException(oss.str());
    }
    return stmt;
}

int SQLite3Store::execute(sqlite3_stmt *stmt, int expected_code) {

    int rc = sqlite3_step(stmt);

    if (rc != expected_code) {

        std::ostringstream oss;
        oss << "Can't execute SQL statement. Error: " << sqlite3_errmsg(db) << ", Error code: " << rc;
        reset(stmt);
        throw StoreException(oss.str());
    }
    return rc;
}

void SQLite3Store::reset(sqlite3_stmt *stmt) {

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
}

void SQLite3Store::finalize(sqlite3_stmt *stmt) {

    reset(stmt);
    sqlite3_finalize(stmt);
}

int SQLite3Store::execute(std::string stmt, int (*callback)(void*, int, char**, char**), void *arg) {

    char* err_msg = 0;
    int rc = sqlite3_exec(db, stmt.c_str(), callback, arg, &err_msg);

    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't execute statement: " << stmt << " error: " << err_msg << ", error code " << rc;
        throw StoreException(oss.str());
    }
    return rc;
}

klio::Sensor::Ptr SQLite3Store::parse_sensor(sqlite3_stmt* stmt) {

    return sensor_factory->createSensor(
            std::string((char*) sqlite3_column_text(stmt, 0)), //uuid
            std::string((char*) sqlite3_column_text(stmt, 1)), //external id
            std::string((char*) sqlite3_column_text(stmt, 2)), //name
            std::string((char*) sqlite3_column_text(stmt, 3)), //description
            std::string((char*) sqlite3_column_text(stmt, 4)), //unit
            std::string((char*) sqlite3_column_text(stmt, 5)), //timezone
            DeviceType::get_by_id(sqlite3_column_int(stmt, 6))); //device type
}