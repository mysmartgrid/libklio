#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/sqlite3/sqlite3-transaction.hpp>
#include <libklio/common.hpp>
#include "sqlite3-store.hpp"
#include "sqlite3-transaction.hpp"


using namespace klio;

void SQLite3Store::open() {

    if (_db == NULL) {

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
        Store::open();
    }
}

void SQLite3Store::close() {

    if (_db != NULL) {

        Store::close();
        _insert_sensor_stmt = NULL;
        _remove_sensor_stmt = NULL;
        _update_sensor_stmt = NULL;
        _select_sensor_stmt = NULL;
        _select_sensor_by_external_id_stmt = NULL;
        _select_sensor_by_name_stmt = NULL;
        _select_sensors_stmt = NULL;
        _select_all_sensor_uuids_stmt = NULL;

        for (boost::unordered_map<const std::string, sqlite3_stmt*>::const_iterator it = _statements.begin(); it != _statements.end(); ++it) {
            sqlite3_stmt* stmt = (*it).second;
            finalize(&stmt);
        }
        _statements.clear();

        sqlite3_close(_db);
        _db = NULL;
    }
}

void SQLite3Store::check_integrity() {

    VersionInfo::Ptr info = VersionInfo::Ptr(new VersionInfo());
    std::string db_version;
    std::ostringstream oss;
    sqlite3_stmt* stmt = NULL;
    std::string result;

    try {
        if (!bfs::exists(_path)) {
            result = "File does not exist.";

        } else if (bfs::is_directory(_path)) {
            result = "Path is a directory.";

        } else if (!bfs::is_regular_file(_path)) {
            result = "This is not a regular file.";

        } else if (bfs::file_size(_path) == 0) {
            result = "File is empty.";

        } else {
            stmt = prepare("PRAGMA integrity_check;");
            execute(stmt, SQLITE_ROW);
            result = std::string((char*) sqlite3_column_text(stmt, 0));
            finalize(&stmt);

            if (result == "ok") {

                if (has_table("sensors") && has_table("properties")) {

                    stmt = prepare("SELECT value FROM properties WHERE name = 'version'");
                    execute(stmt, SQLITE_ROW);
                    db_version = std::string((char*) sqlite3_column_text(stmt, 0));
                    finalize(&stmt);

                    std::vector<std::string> db_version_digits;
                    boost::split(db_version_digits, db_version, boost::is_any_of("."));

                    std::vector<std::string> klio_version_digits;
                    boost::split(klio_version_digits, info->getVersion(), boost::is_any_of("."));

                    const std::string db_digit0 = db_version_digits.at(0);
                    const std::string db_digit1 = db_version_digits.at(1);
                    const std::string klio_digit0 = klio_version_digits.at(0);
                    const std::string klio_digit1 = klio_version_digits.at(1);

                    //Only two first digits must match
                    if (db_digit0 > klio_digit0 || (db_digit0 == klio_digit0 && db_digit1 > klio_digit1)) {

                        result = "oldklio";

                    } else if (db_digit0 < klio_digit0 || (db_digit0 == klio_digit0 && db_digit1 < klio_digit1)) {
                        result = "olddb";
                    }
                } else {
                    result = "olddb";
                }
            } else {
                result = "corruptdb";
            }
        }
    } catch (std::exception const& e) {
        finalize(&stmt);
        result == e.what();
    }

    if (result == "ok") {
        return;

    } else if (result == "oldklio") {
        oss << std::endl <<
                "The store was created using a more recent version of libKlio. " <<
                "Please install the version " << db_version << " of the library.";

    } else if (result == "olddb") {
        oss << std::endl <<
                "The store was created using an old version of libKlio (" << db_version << "). " <<
                "Please use the command " << std::endl <<
                " $ klio-store -a upgrade -s <FILE>" << std::endl
                << "to upgrade it to database version " << info->getVersion();

    } else if (result == "corruptdb") {
        oss << "The database is corrupt.";

    } else {
        oss << std::endl << result;
    }
    throw StoreException(oss.str());
}

void SQLite3Store::initialize() {

    //Create table sensors if it does not exist
    sqlite3_stmt* stmt = prepare("CREATE TABLE IF NOT EXISTS sensors(uuid VARCHAR(16) PRIMARY KEY, external_id VARCHAR(32), name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INTEGER, device_type_id INTEGER)");

    try {
        execute(stmt, SQLITE_DONE);
        finalize(&stmt);

        stmt = prepare("CREATE UNIQUE INDEX IF NOT EXISTS sensors_external_id_idx ON sensors (external_id)");
        execute(stmt, SQLITE_DONE);
        finalize(&stmt);

        //Create table properties if it does not exist
        stmt = prepare("CREATE TABLE IF NOT EXISTS properties(name VARCHAR(32) PRIMARY KEY, value VARCHAR(32))");
        execute(stmt, SQLITE_DONE);
        finalize(&stmt);

        VersionInfo::Ptr info = VersionInfo::Ptr(new VersionInfo());
        stmt = prepare("INSERT OR IGNORE INTO properties (name, value) VALUES(?1, ?2)");
        sqlite3_bind_text(stmt, 1, "version", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, info->getVersion().c_str(), -1, SQLITE_TRANSIENT);
        execute(stmt, SQLITE_DONE);

    } catch (std::exception const& e) {
        finalize(&stmt);
        throw;
    }
    finalize(&stmt);
}

void SQLite3Store::upgrade() {

    VersionInfo::Ptr info = VersionInfo::Ptr(new VersionInfo());
    sqlite3_stmt* stmt = prepare("INSERT OR REPLACE INTO properties (name, value) VALUES(?1, ?2)");

    try {
        sqlite3_bind_text(stmt, 1, "version", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, info->getVersion().c_str(), -1, SQLITE_TRANSIENT);

        const Transaction::Ptr transaction = auto_start_transaction();

        execute(stmt, SQLITE_DONE);
        finalize(&stmt);

        if (!has_column("sensors", "device_type_id")) {
            stmt = prepare("ALTER TABLE sensors ADD COLUMN device_type_id INTEGER");
            execute(stmt, SQLITE_DONE);
            finalize(&stmt);

            stmt = prepare("UPDATE sensors SET device_type_id = 0");
            execute(stmt, SQLITE_DONE);
            finalize(&stmt);
        }

        stmt = prepare("CREATE UNIQUE INDEX IF NOT EXISTS sensors_external_id_idx ON sensors (external_id)");
        execute(stmt, SQLITE_DONE);
        finalize(&stmt);

        auto_commit_transaction(transaction);

    } catch (std::exception const& e) {
        finalize(&stmt);
        throw;
    }
}

void SQLite3Store::prepare() {

    _insert_sensor_stmt = get_statement("INSERT INTO sensors (uuid, external_id, name, description, unit, timezone, device_type_id) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)");
    _remove_sensor_stmt = get_statement("DELETE FROM sensors WHERE uuid = ?1");
    _update_sensor_stmt = get_statement("UPDATE sensors SET external_id = ?2, name = ?3, description = ?4, unit = ?5, timezone = ?6, device_type_id = ?7 WHERE uuid = ?1");
    _select_sensor_stmt = get_statement("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE uuid = ?1");
    _select_sensor_by_external_id_stmt = get_statement("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE external_id = ?1");
    _select_sensor_by_name_stmt = get_statement("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors WHERE name = ?1");
    _select_sensors_stmt = get_statement("SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors");
    _select_all_sensor_uuids_stmt = get_statement("SELECT uuid FROM sensors");

    Store::prepare();
}

void SQLite3Store::dispose() {

    bfs::remove(_path);
    Store::dispose();
}

Transaction::Ptr SQLite3Store::create_transaction() {

    return SQLite3Transaction::Ptr(new SQLite3Transaction(_db));
}

bool SQLite3Store::has_table(const std::string& name) {

    bool found = false;
    sqlite3_stmt* stmt = get_statement("SELECT name FROM sqlite_master WHERE type='table' AND name=?");

    try {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        found = (sqlite3_step(stmt) == SQLITE_ROW);

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return found;
}

bool SQLite3Store::has_column(const std::string& table, const std::string& column) {

    bool found = false;
    sqlite3_stmt* stmt = get_statement("PRAGMA table_info(sensors)");

    try {
        while (SQLITE_ROW == sqlite3_step(stmt)) {
            if (std::string((char*) sqlite3_column_text(stmt, 1)) == column) {
                found = true;
                break;
            }
        }

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return found;
}

const std::string SQLite3Store::str() {

    std::ostringstream oss;
    oss << "SQLite3 database, stored in file " << _path;
    return oss.str();
};

void SQLite3Store::add_sensor_record(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "CREATE TABLE '" << sensor->uuid_string() << "'(timestamp INTEGER PRIMARY KEY, value DOUBLE)";
    sqlite3_stmt* create_table_stmt = prepare(oss.str());

    try {
        sqlite3_bind_text(_insert_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_insert_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_insert_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_insert_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_insert_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_insert_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(_insert_sensor_stmt, 7, sensor->device_type()->id());

        execute(_insert_sensor_stmt, SQLITE_DONE);
        execute(create_table_stmt, SQLITE_DONE);

    } catch (std::exception const& e) {
        reset(_insert_sensor_stmt);
        finalize(&create_table_stmt);
        throw;
    }
    reset(_insert_sensor_stmt);
    finalize(&create_table_stmt);
}

void SQLite3Store::remove_sensor_record(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "DROP TABLE '" << sensor->uuid_string() << "'";
    sqlite3_stmt* drop_table_stmt = prepare(oss.str());

    try {
        sqlite3_bind_text(_remove_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);

        execute(_remove_sensor_stmt, SQLITE_DONE);
        execute(drop_table_stmt, SQLITE_DONE);

    } catch (std::exception const& e) {
        reset(_remove_sensor_stmt);
        finalize(&drop_table_stmt);
        throw;
    }
    reset(_remove_sensor_stmt);
    finalize(&drop_table_stmt);
}

void SQLite3Store::update_sensor_record(const Sensor::Ptr sensor) {

    try {
        sqlite3_bind_text(_update_sensor_stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_update_sensor_stmt, 2, sensor->external_id().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_update_sensor_stmt, 3, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_update_sensor_stmt, 4, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_update_sensor_stmt, 5, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(_update_sensor_stmt, 6, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(_update_sensor_stmt, 7, sensor->device_type()->id());

        execute(_update_sensor_stmt, SQLITE_DONE);

    } catch (std::exception const& e) {
        reset(_update_sensor_stmt);
        throw;
    }
    reset(_update_sensor_stmt);
}

std::vector<Sensor::Ptr> SQLite3Store::get_sensor_records() {

    std::vector<Sensor::Ptr> sensors;

    try {
        while (SQLITE_ROW == sqlite3_step(_select_sensors_stmt)) {
            sensors.push_back(parse_sensor(_select_sensors_stmt));
        }

    } catch (std::exception const& e) {
        reset(_select_sensors_stmt);
        throw;
    }
    reset(_select_sensors_stmt);
    return sensors;
}

readings_t_Ptr SQLite3Store::get_all_reading_records(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "'";
    sqlite3_stmt* stmt = get_statement(oss.str());

    readings_t_Ptr readings;
    try {
        readings = get_readings_records(stmt);

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return readings;
}

readings_t_Ptr SQLite3Store::get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' WHERE timestamp BETWEEN ? AND ?";
    sqlite3_stmt* stmt = get_statement(oss.str());

    readings_t_Ptr readings;
    try {
        sqlite3_bind_int(stmt, 1, begin);
        sqlite3_bind_int(stmt, 2, end);

        readings = get_readings_records(stmt);

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return readings;
}

readings_t_Ptr SQLite3Store::get_readings_records(sqlite3_stmt* stmt) {

    readings_t_Ptr readings(new readings_t());

    while (SQLITE_ROW == sqlite3_step(stmt)) {

        readings->insert(
                std::pair<timestamp_t, double>(
                time_converter->convert_from_epoch(sqlite3_column_int(stmt, 0)),
                sqlite3_column_double(stmt, 1)
                ));
    }
    return readings;
}

unsigned long int SQLite3Store::get_num_readings_value(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "SELECT count(*) FROM '" << sensor->uuid_string() << "'";
    sqlite3_stmt* stmt = get_statement(oss.str());

    int num;
    try {
        num = sqlite3_step(stmt) == SQLITE_ROW ? sqlite3_column_int(stmt, 0) : 0;

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return num;
}

reading_t SQLite3Store::get_last_reading_record(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' ORDER BY timestamp DESC LIMIT 1";
    sqlite3_stmt* stmt = get_statement(oss.str());

    std::pair<timestamp_t, double> reading;
    try {
        if (sqlite3_step(stmt) == SQLITE_ROW) {

            reading = std::pair<timestamp_t, double>(
                    time_converter->convert_from_epoch(sqlite3_column_int(stmt, 0)),
                    sqlite3_column_double(stmt, 1)
                    );
        }

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return reading;
}

reading_t SQLite3Store::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' WHERE timestamp = ?";
    sqlite3_stmt* stmt = get_statement(oss.str());

    std::pair<timestamp_t, double> reading;
    try {
        sqlite3_bind_int(stmt, 1, timestamp);

        if (sqlite3_step(stmt) == SQLITE_ROW) {

            reading = std::pair<timestamp_t, double>(
                    time_converter->convert_from_epoch(sqlite3_column_int(stmt, 0)),
                    sqlite3_column_double(stmt, 1)
                    );
        }

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
    reset(stmt);
    return reading;
}

void SQLite3Store::add_reading_records(const Sensor::Ptr sensor, const readings_t& readings) {

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        add_reading_record(sensor, (*it).first, (*it).second, "INSERT");
    }
}

void SQLite3Store::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings) {

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        add_reading_record(sensor, (*it).first, (*it).second, "INSERT OR REPLACE");
    }
}

void SQLite3Store::add_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const std::string& operation) {

    std::ostringstream oss;
    oss << operation << " INTO '" << sensor->uuid_string() << "' (timestamp, value) VALUES (?, ?)";
    sqlite3_stmt* stmt = get_statement(oss.str());

    try {
        sqlite3_bind_int(stmt, 1, time_converter->convert_to_epoch(timestamp));
        sqlite3_bind_double(stmt, 2, value);

        execute(stmt, SQLITE_DONE);
        reset(stmt);

    } catch (std::exception const& e) {
        reset(stmt);
        throw;
    }
}

sqlite3_stmt *SQLite3Store::prepare(const std::string& stmt_str) {

    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(
            _db, //Database handle
            stmt_str.c_str(), //SQL statement, UTF-8 encoded
            -1, //Maximum length of zSql in bytes. -1 means: read complete string.
            &stmt, //OUT: Statement handle
            NULL //OUT: Pointer to unused portion of zSql
            );

    if (rc != SQLITE_OK) {

        std::ostringstream oss;
        oss << "Can't prepare SQL statement: " << stmt_str << ". Error: " << sqlite3_errmsg(_db) << ", Error code: " << rc;
        finalize(&stmt);
        throw StoreException(oss.str());
    }
    return stmt;
}

sqlite3_stmt *SQLite3Store::get_statement(const std::string& sql) {

    if (_statements.count(sql) == 0) {
        _statements[sql] = prepare(sql);
    }
    return _statements[sql];
}

int SQLite3Store::execute(sqlite3_stmt *stmt, const int expected_code) {

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

Sensor::Ptr SQLite3Store::parse_sensor(sqlite3_stmt* stmt) {

    return sensor_factory->createSensor(
            std::string((char*) sqlite3_column_text(stmt, 0)), //uuid
            std::string((char*) sqlite3_column_text(stmt, 1)), //external id
            std::string((char*) sqlite3_column_text(stmt, 2)), //name
            std::string((char*) sqlite3_column_text(stmt, 3)), //description
            std::string((char*) sqlite3_column_text(stmt, 4)), //unit
            std::string((char*) sqlite3_column_text(stmt, 5)), //timezone
            DeviceType::get_by_id(sqlite3_column_int(stmt, 6))); //device type
}
