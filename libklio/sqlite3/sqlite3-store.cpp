#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/sqlite3/transaction.hpp>
#include "sqlite3-store.hpp"


using namespace klio;


/**
 * CHANGES
 *
 * Ely: This class has been refactored. It is 33% smaller now.
 * 
 * Some method names adopted the cammel format, like addSensor, while others adopted
 * the C default format, like add_reading. After talking with Mathias,
 * I standardized the C one.
 * 
 * I don't know if this format should be adopted in the whole library.
 */

// database statements //TODO: Change these constants to real prepared statements
const std::string create_sensors_table_stmt(
        "CREATE TABLE IF NOT EXISTS sensors(uuid VARCHAR(16) PRIMARY KEY, name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INTEGER);"
        );

const std::string insert_sensor_stmt(
        "INSERT INTO sensors (uuid, name, description, unit, timezone) VALUES (?1, ?2, ?3, ?4, ?5);"
        );

const std::string remove_sensor_stmt(
        "DELETE FROM sensors WHERE uuid = ?1"
        );

const std::string update_sensor_stmt(
        "UPDATE sensors SET name = ?2, description = ?3, unit = ?4, timezone = ?5 WHERE uuid = ?1;"
        );

const std::string select_sensor_stmt(
        "SELECT uuid, name, description, unit, timezone FROM sensors WHERE uuid = ?1;"
        );

const std::string select_sensor_by_name_stmt(
        "SELECT uuid, name, description, unit, timezone FROM sensors WHERE name = ?1;"
        );

const std::string select_all_sensor_uuids_stmt(
        "SELECT uuid FROM sensors;"
        );


void SQLite3Store::open() {

    int rc = sqlite3_open(_path.c_str(), &db);
    if (rc) {
        std::ostringstream oss;
        oss << "Can't open database: " << sqlite3_errmsg(db);
        sqlite3_close(db);
        throw StoreException(oss.str());
    }
}

void SQLite3Store::close() {

    sqlite3_close(db);
}

void SQLite3Store::initialize() {

    //Create table sensors if it does not exist
    sqlite3_stmt* stmt = prepare(create_sensors_table_stmt);
    execute(stmt, SQLITE_DONE);
    finalize(stmt);
}

void SQLite3Store::dispose() {
    //TODO: complete this method
    close();
}


const std::string SQLite3Store::str() {

    std::ostringstream oss;
    oss << "SQLite3 database, stored in file " << _path;
    return oss.str();
};

void SQLite3Store::add_sensor(klio::Sensor::Ptr sensor) {

    LOG("Adding sensor: " << sensor->str());

    Transaction transaction(db);

    sqlite3_stmt* stmt = prepare(insert_sensor_stmt);
    sqlite3_bind_text(stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);

    execute(stmt, SQLITE_DONE);
    reset(stmt);

    std::ostringstream oss;
    oss << "CREATE TABLE '" << sensor->uuid_string() << "'(timestamp INTEGER PRIMARY KEY, value DOUBLE);";

    stmt = prepare(oss.str());
    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    transaction.commit();
}

void SQLite3Store::remove_sensor(const klio::Sensor::Ptr sensor) {

    LOG("Removing sensor: " << sensor->str());

    Transaction transaction(db);

    sqlite3_stmt* stmt = prepare(remove_sensor_stmt);
    sqlite3_bind_text(stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);

    execute(stmt, SQLITE_DONE);
    reset(stmt);

    std::ostringstream oss;
    oss << "DROP TABLE '" << sensor->uuid_string() << "'";

    stmt = prepare(oss.str());
    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    transaction.commit();
}

void SQLite3Store::update_sensor(klio::Sensor::Ptr sensor) {

    LOG("Updating sensor: " << sensor->str());

    Transaction transaction(db);

    sqlite3_stmt* stmt = prepare(update_sensor_stmt);
    sqlite3_bind_text(stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);

    execute(stmt, SQLITE_DONE);
    finalize(stmt);

    transaction.commit();
}

klio::Sensor::Ptr SQLite3Store::get_sensor(const klio::Sensor::uuid_t& uuid) {

    LOG("Attempting to load sensor " << uuid);

    sqlite3_stmt* stmt = prepare(select_sensor_stmt);
    sqlite3_bind_text(stmt, 1, to_string(uuid).c_str(), -1, SQLITE_TRANSIENT);

    execute(stmt, SQLITE_ROW);
    
    const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
    const unsigned char* select_name = sqlite3_column_text(stmt, 1);
    const unsigned char* select_description = sqlite3_column_text(stmt, 2);
    const unsigned char* select_unit = sqlite3_column_text(stmt, 3);
    const unsigned char* select_timezone = sqlite3_column_text(stmt, 4);

    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor(sensor_factory->createSensor(
            std::string((char*) select_uuid),
            std::string((char*) select_name),
            std::string((char*) select_description),
            std::string((char*) select_unit),
            std::string((char*) select_timezone)));

    finalize(stmt);
    return sensor;
}

/**
 * CHANGES
 * 
 * Ely: This method has been renamed. 
 * It was called getSensorById. Actually, it selects sensors by name.
 */
std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensors_by_name(const std::string& name) {

    LOG("Attempting to load sensor " << name);

    sqlite3_stmt* stmt = prepare(select_sensor_by_name_stmt);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<klio::Sensor::Ptr> sensors;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    while (SQLITE_ROW == sqlite3_step(stmt)) {

        const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
        const unsigned char* select_name = sqlite3_column_text(stmt, 1);
        const unsigned char* select_desc = sqlite3_column_text(stmt, 2);
        const unsigned char* select_unit = sqlite3_column_text(stmt, 3);
        const unsigned char* select_timezone = sqlite3_column_text(stmt, 4);

        klio::Sensor::Ptr current(sensor_factory->createSensor(
                std::string((char*) select_uuid),
                std::string((char*) select_name),
                std::string((char*) select_desc),
                std::string((char*) select_unit),
                std::string((char*) select_timezone)));

        sensors.push_back(current);
    }

    finalize(stmt);

    return sensors;
}

std::vector<klio::Sensor::uuid_t> SQLite3Store::get_sensor_uuids() {

    LOG("Retrieving UUIDs from store.");

    std::vector<klio::Sensor::uuid_t> uuids;

    sqlite3_stmt* stmt = prepare(select_all_sensor_uuids_stmt);

    while (SQLITE_ROW == sqlite3_step(stmt)) {

        const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
        boost::uuids::uuid u;
        std::stringstream ss;
        ss << select_uuid;
        ss >> u;
        uuids.push_back(u);
    }

    finalize(stmt);

    return uuids;
}

/**
 * Dummy helper for sqlite3_exec - dumps the return values.
 */
static int empty_callback(void *not_used, int argc, char **argv, char **az_col_name) {

    for (int i = 0; i < argc; i++) {
        printf("%s,\t", argv[i]);
    }
    printf("\n");
    return 0;
}

void SQLite3Store::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    Transaction transaction(db);

    insert_reading_record(sensor, timestamp, value);

    transaction.commit();
}

void SQLite3Store::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    Transaction transaction(db);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        klio::timestamp_t timestamp = (*it).first;
        double value = (*it).second;

        insert_reading_record(sensor, timestamp, value);
    }
    transaction.commit();
}

void SQLite3Store::insert_reading_record(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    LOG("Adding to sensor: " << sensor->str() << " time=" << timestamp << " value=" << value);

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' (timestamp, value) ";
    oss << "VALUES (" << tc->convert_to_epoch(timestamp) << ", " << value << ");";

    execute(oss.str(), empty_callback, NULL);
}

void SQLite3Store::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    Transaction transaction(db);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        klio::timestamp_t timestamp = (*it).first;
        double value = (*it).second;

        std::ostringstream oss;

        // see http://stackoverflow.com/questions/2717590/sqlite-upsert-on-duplicate-key-update
        // The SQL statement is constructed as two:
        // 1. If the timestamp exists, the "or ignore" part of stmt 1 silently ignores the insert statement.
        oss << "INSERT OR IGNORE INTO '" << sensor->uuid_string() << "' (timestamp, value) ";
        oss << "VALUES (" << tc->convert_to_epoch(timestamp) << ", " << value << ");";

        // 2. Instead, the update statement inserts the last received value.
        oss << "UPDATE '" << sensor->uuid_string() << "' ";
        oss << "SET value='" << value << "' WHERE timestamp LIKE '";
        oss << tc->convert_to_epoch(timestamp) << "';";

        execute(oss.str(), empty_callback, NULL);
    }
    transaction.commit();
}

static int get_all_readings_callback(void *store, int argc, char **argv, char **az_col_name) {

    std::map<timestamp_t, double>* datastore;
    datastore = reinterpret_cast<std::map<timestamp_t, double>*> (store);
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    long epoch = atol(argv[0]);
    double value = atof(argv[1]);

    datastore->insert(
            std::pair<timestamp_t, double>(
            tc->convert_from_epoch(epoch),
            value
            ));
    return 0;
}

readings_t_Ptr SQLite3Store::get_all_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving all readings of sensor " << sensor->str());

    readings_t_Ptr readings(new readings_t());

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "';";

    execute(oss.str(), get_all_readings_callback, readings.get());

    return readings;
}

static int get_num_readings_callback(void *num, int argc, char **argv, char **azColName) {

    long int* numreadings = (long int*) num;
    *numreadings = atol(argv[0]);

    return 0;
}

unsigned long int SQLite3Store::get_num_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT count(*) FROM '" << sensor->uuid_string() << "';";

    long int num;
    execute(oss.str(), get_num_readings_callback, &num);

    return num;
}

static int get_last_reading_callback(void *pair, int argc, char **argv, char **azColName) {

    std::pair<timestamp_t, double>* datastore;
    datastore = reinterpret_cast<std::pair <timestamp_t, double> *> (pair);
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    long epoch = atol(argv[0]);
    double value = atof(argv[1]);
    datastore->first = tc->convert_from_epoch(epoch);
    datastore->second = value;

    return 0;
}

std::pair<timestamp_t, double> SQLite3Store::get_last_reading(klio::Sensor::Ptr sensor) {

    LOG("Retrieving last readings of sensor " << sensor->str());

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' ";
    oss << "ORDER BY timestamp DESC LIMIT 1;";

    std::pair<timestamp_t, double> reading;
    execute(oss.str(), get_last_reading_callback, &reading);

    return reading;
}

sqlite3_stmt *SQLite3Store::prepare(const std::string stmt_str) {

    sqlite3_stmt* stmt;
    const char* pz_tail;

    int rc = sqlite3_prepare_v2(
            db, /* Database handle */
            stmt_str.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes. -1 means: read complete string. */
            &stmt, /* OUT: Statement handle */
            &pz_tail /* OUT: Pointer to unused portion of zSql */
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
        finalize(stmt);
        throw StoreException(oss.str());
    }
    return rc;
}

void SQLite3Store::reset(sqlite3_stmt *stmt) {

    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
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
