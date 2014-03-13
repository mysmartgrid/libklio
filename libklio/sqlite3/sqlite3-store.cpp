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

    if (_db) {
        return;
    }

    int rc = sqlite3_open(_path.c_str(), &_db);
    if (rc) {
        std::ostringstream oss;
        oss << "Can't open database: ";

        if (_db) {
            oss << sqlite3_errmsg(_db);
            sqlite3_close(_db);
            _db = NULL;

        } else {
            oss << "Not enough memory.";
        }
        throw StoreException(oss.str());
    }
}

void SQLite3Store::close() {

    if (_db) {
        finalize(&_insert_sensor_stmt);
        finalize(&_remove_sensor_stmt);
        finalize(&_update_sensor_stmt);
        finalize(&_select_sensor_stmt);
        finalize(&_select_sensor_by_external_id_stmt);
        finalize(&_select_sensor_by_name_stmt);
        finalize(&_select_sensors_stmt);
        finalize(&_select_all_sensor_uuids_stmt);
        sqlite3_close(_db);
        _db = NULL;
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
            finalize(&stmt);

            if (result == "ok") {

                if (has_table("sensors")) {
                    return;

                } else {
                    oss << "Database has not been initialized.";
                }

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
    sqlite3_stmt* stmt = prepare("CREATE TABLE IF NOT EXISTS sensors(uuid VARCHAR(16) PRIMARY KEY, external_id VARCHAR(32), name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INTEGER, device_type_id INTEGER)");
    execute(stmt, SQLITE_DONE);
    finalize(&stmt);
}

void SQLite3Store::prepare() {

    _insert_sensor_stmt = prepare("INSERT INTO sensors (uuid, external_id, name, description, unit, timezone, device_type_id) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)");
    _remove_sensor_stmt = prepare("DELETE FROM sensors WHERE uuid = ?1");
    _update_sensor_stmt = prepare("UPDATE sensors SET external_id = ?2, name = ?3, description = ?4, unit = ?5, timezone = ?6, device_type_id = ?7 WHERE uuid = ?1");
    _select_sensor_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE uuid = ?1");
    _select_sensor_by_external_id_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE external_id = ?1");
    _select_sensor_by_name_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE name = ?1");
    _select_sensors_stmt = prepare("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors");
    _select_all_sensor_uuids_stmt = prepare("SELECT uuid FROM sensors");
}

void SQLite3Store::dispose() {

    close();
    bfs::remove(_path);
}

bool SQLite3Store::has_table(std::string name) {

    sqlite3_stmt* stmt = prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    finalize(&stmt);
    return found;
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
    sqlite3_stmt* create_table_stmt = prepare(oss.str());

    sqlite3_bind_text(_insert_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_insert_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_insert_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_insert_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_insert_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_insert_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(_insert_sensor_stmt, 7, sensor->device_type()->id());

    Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));
    execute(_insert_sensor_stmt, SQLITE_DONE);
    execute(create_table_stmt, SQLITE_DONE);
    transaction->commit();

    reset(_insert_sensor_stmt);
    finalize(&create_table_stmt);
}

void SQLite3Store::remove_sensor(const klio::Sensor::Ptr sensor) {

    LOG("Removing sensor: " << sensor->str());

    std::ostringstream oss;
    oss << "DROP TABLE '" << sensor->uuid_string() << "'";
    sqlite3_stmt* drop_table_stmt = prepare(oss.str());

    sqlite3_bind_text(_remove_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);

    Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));
    execute(_remove_sensor_stmt, SQLITE_DONE);
    execute(drop_table_stmt, SQLITE_DONE);
    transaction->commit();

    reset(_remove_sensor_stmt);
    finalize(&drop_table_stmt);
}

void SQLite3Store::update_sensor(klio::Sensor::Ptr sensor) {

    LOG("Updating sensor: " << sensor->str());

    sqlite3_bind_text(_update_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_update_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_update_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_update_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_update_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(_update_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(_update_sensor_stmt, 7, sensor->device_type()->id());

    Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));
    execute(_update_sensor_stmt, SQLITE_DONE);
    transaction->commit();

    reset(_update_sensor_stmt);
}

klio::Sensor::Ptr SQLite3Store::get_sensor(const klio::Sensor::uuid_t& uuid) {

    LOG("Attempting to load sensor " << uuid);

    sqlite3_bind_text(_select_sensor_stmt, 1, boost::uuids::to_string(uuid).c_str(), -1, SQLITE_TRANSIENT);
    execute(_select_sensor_stmt, SQLITE_ROW);
    klio::Sensor::Ptr sensor = parse_sensor(_select_sensor_stmt);
    reset(_select_sensor_stmt);

    return sensor;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors_by_external_id(const std::string& external_id) {

    LOG("Attempting to load sensors " << external_id);

    std::vector<klio::Sensor::Ptr> sensors;
    sqlite3_bind_text(_select_sensor_by_external_id_stmt, 1, external_id.c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(_select_sensor_by_external_id_stmt)) {
        sensors.push_back(parse_sensor(_select_sensor_by_external_id_stmt));
    }
    reset(_select_sensor_by_external_id_stmt);

    return sensors;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors_by_name(const std::string& name) {

    LOG("Attempting to load sensors " << name);

    std::vector<klio::Sensor::Ptr> sensors;
    sqlite3_bind_text(_select_sensor_by_name_stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(_select_sensor_by_name_stmt)) {
        sensors.push_back(parse_sensor(_select_sensor_by_name_stmt));
    }
    reset(_select_sensor_by_name_stmt);

    return sensors;
}

std::vector<klio::Sensor::uuid_t> SQLite3Store::get_sensor_uuids() {

    LOG("Retrieving UUIDs from store.");

    std::vector<klio::Sensor::uuid_t> uuids;

    while (SQLITE_ROW == sqlite3_step(_select_all_sensor_uuids_stmt)) {

        const unsigned char* select_uuid = sqlite3_column_text(_select_all_sensor_uuids_stmt, 0);
        boost::uuids::uuid u;
        std::stringstream ss;
        ss << select_uuid;
        ss >> u;
        uuids.push_back(u);
    }
    reset(_select_all_sensor_uuids_stmt);

    return uuids;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors() {

    LOG("Attempting to load sensors");

    std::vector<klio::Sensor::Ptr> sensors;

    while (SQLITE_ROW == sqlite3_step(_select_sensors_stmt)) {
        sensors.push_back(parse_sensor(_select_sensors_stmt));
    }
    reset(_select_sensors_stmt);

    return sensors;
}

void SQLite3Store::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    LOG("Adding to sensor: " << sensor->str() << " time=" << timestamp << " value=" << value);

    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = prepare(oss.str());

    Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));
    try {
        insert_reading_record(stmt, timestamp, value);

    } catch (klio::StoreException e) {
        finalize(&stmt);
        throw;
    }
    transaction->commit();

    finalize(&stmt);
}

void SQLite3Store::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    LOG("Adding " << readings->size() << " readings to sensor: " << sensor->str());

    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));

        for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

            insert_reading_record(stmt, (*it).first, (*it).second);
        }
        transaction->commit();

    } catch (klio::StoreException e) {
        finalize(&stmt);
        throw;
    }
    finalize(&stmt);
}

void SQLite3Store::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    std::ostringstream oss;
    oss << "INSERT OR REPLACE INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        Transaction::Ptr transaction(Transaction::Ptr(new Transaction(_db)));

        for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

            insert_reading_record(stmt, (*it).first, (*it).second);
        }
        transaction->commit();

    } catch (klio::StoreException e) {
        finalize(&stmt);
        throw;
    }
    finalize(&stmt);
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
    sqlite3_stmt* stmt = prepare(oss.str());

    readings_t_Ptr readings(new readings_t());
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

    } catch (klio::StoreException e) {
        finalize(&stmt);
        throw;
    }
    finalize(&stmt);
    return readings;
}

unsigned long int SQLite3Store::get_num_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT count(*) FROM '" << sensor->uuid_string() << "';";
    sqlite3_stmt* stmt = prepare(oss.str());

    try {
        int num = sqlite3_step(stmt) == SQLITE_ROW ? sqlite3_column_int(stmt, 0) : 0;
        finalize(&stmt);
        return num;

    } catch (std::exception const& e) {
        finalize(&stmt);
        throw;
    }
}

reading_t SQLite3Store::get_last_reading(klio::Sensor::Ptr sensor) {

    LOG("Retrieving last readings of sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' ORDER BY timestamp DESC LIMIT 1;";
    sqlite3_stmt* stmt = prepare(oss.str());

    std::pair<timestamp_t, double> reading;

    try {
        if (sqlite3_step(stmt) == SQLITE_ROW) {

            reading = std::pair<timestamp_t, double>(
                    time_converter->convert_from_epoch(sqlite3_column_int(stmt, 0)),
                    sqlite3_column_double(stmt, 1)
                    );
        }

    } catch (std::exception const& e) {
        finalize(&stmt);
        throw;
    }
    finalize(&stmt);
    return reading;
}

sqlite3_stmt *SQLite3Store::prepare(const std::string& stmt_str) {

    sqlite3_stmt* stmt;
    const char* pz_tail;

    int rc = sqlite3_prepare_v2(
            _db, //Database handle
            stmt_str.c_str(), //SQL statement, UTF-8 encoded
            -1, //Maximum length of zSql in bytes. -1 means: read complete string.
            &stmt, //OUT: Statement handle
            &pz_tail //OUT: Pointer to unused portion of zSql
            );

    if (rc != SQLITE_OK) {

        std::ostringstream oss;
        oss << "Can't prepare SQL statement: " << stmt_str << ". Error: " << sqlite3_errmsg(_db) << ", Error code: " << rc;
        finalize(&stmt);
        throw StoreException(oss.str());
    }
    return stmt;
}

int SQLite3Store::execute(sqlite3_stmt *stmt, int expected_code) {

    int rc = sqlite3_step(stmt);

    if (rc != expected_code) {

        std::ostringstream oss;
        oss << "Can't execute SQL statement. Error: " << sqlite3_errmsg(_db) << ", Error code: " << rc;
        reset(stmt);
        throw StoreException(oss.str());
    }
    return rc;
}

void SQLite3Store::reset(sqlite3_stmt *stmt) {

    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
}

void SQLite3Store::finalize(sqlite3_stmt **stmt) {

    sqlite3_finalize(*stmt);
    *stmt = NULL;
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