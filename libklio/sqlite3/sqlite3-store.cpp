#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/sqlite3/transaction.hpp>
#include "sqlite3-store.hpp"


using namespace klio;

// database statements
const std::string createSensorTableStmt(
        "CREATE TABLE sensors(uuid VARCHAR(16) PRIMARY KEY, name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INTEGER);");
//TODO: Change this to real prepared statement
const std::string insertSensorStmt(
        "INSERT INTO sensors (uuid, name, description, unit, timezone) VALUES (?1,?2,?3,?4,?5)"
        );
//TODO: Change this to real prepared statement
const std::string selectSensorStmt(
        "SELECT uuid,name,description,unit,timezone FROM sensors where (uuid=?1)"
        );
//TODO: Change this to real prepared statement
const std::string selectSensorByNameStmt(
        "SELECT uuid,name,description,unit,timezone FROM sensors where (name=?1)"
        );
const std::string selectAllSensorUUIDsStmt(
        "SELECT uuid FROM sensors"
        );
//TODO: Change this to real prepared statement
const std::string remove_sensorStmt(
        "DELETE FROM sensors WHERE (uuid=?1)"
        );

// Opens a Database file.

void SQLite3Store::open() {
    int rc = sqlite3_open(_path.c_str(), &db);
    if (rc) {
        std::ostringstream oss;
        oss << "Can't open database: " << sqlite3_errmsg(db);
        sqlite3_close(db);
        throw StoreException(oss.str());
    }
}

void SQLite3Store::initialize() {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;

    if (has_table("sensors")) {
        LOG("Sensors table already exists, skipping creation.");
        return;
    }

    rc = sqlite3_prepare(
            db, /* Database handle */
            createSensorTableStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't create sensors creation command: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't create sensors table: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_finalize(stmt);
}

void SQLite3Store::close() {
    sqlite3_close(db);
}

bool SQLite3Store::has_table(std::string name) {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;
    std::ostringstream oss;
    oss << "select * from sqlite_master where name='" << name << "';";
    std::string hasSensorTableStmt(oss.str());

    rc = sqlite3_prepare(
            db, /* Database handle */
            hasSensorTableStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't create sensors query command: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) { // sqlite3_step has finished, there was no result row.
        sqlite3_finalize(stmt);
        return false;
    } else { // there was a result row - the table exists.
        sqlite3_finalize(stmt);
        return true;
    }
}

const std::string SQLite3Store::str() {
    std::ostringstream oss;
    oss << "SQLite3 database, stored in file " << _path;
    return oss.str();
};

void SQLite3Store::add_sensor(klio::Sensor::Ptr sensor) {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;

    Transaction transaction(db);

    LOG("Adding to store: " << sensor->str());

    checkSensorTable();

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            insertSensorStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare sensor insertion statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_bind_text(stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sensor->name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sensor->description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, sensor->unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, sensor->timezone().c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute sensor insertion statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    std::ostringstream oss;
    oss << "CREATE TABLE '" << sensor->uuid_string() << "'(timestamp INTEGER PRIMARY KEY, value DOUBLE);";
    std::string createValueTableStmt(oss.str());

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            createValueTableStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare value table insertion statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute value table insertion statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    transaction.commit();
}

void SQLite3Store::checkSensorTable() {
    if (!has_table("sensors")) {
        std::ostringstream oss;
        oss << "table sensors is missing in " << str();
        throw StoreException(oss.str());
    }
}

void SQLite3Store::remove_sensor(const klio::Sensor::Ptr sensor) {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;

    checkSensorTable();
    Transaction transaction(db);

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            remove_sensorStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare sensor remove statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_bind_text(stmt, 1, sensor->uuid_string().c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute sensor remove statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);

    std::ostringstream oss;
    oss << "DROP TABLE '" << sensor->uuid_string() << "';";
    std::string dropValueTableStmt(oss.str());

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            dropValueTableStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare drop value table statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute drop value table statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    transaction.commit();

}

std::vector<klio::Sensor::uuid_t> SQLite3Store::get_sensor_uuids() {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;
    std::vector<klio::Sensor::uuid_t> uuids;

    LOG("Retrieving UUIDs from store.");
    checkSensorTable();
    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            selectAllSensorUUIDsStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare sensor select statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    //sqlite3_bind_text(stmt, 1, to_string(uuid).c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(stmt)) {
        const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
        //std::cout << " -> " << select_uuid  << std::endl;
        // type conversion: uuid_string to real uuid type
        boost::uuids::uuid u;
        std::stringstream ss;
        ss << select_uuid;
        ss >> u;
        uuids.push_back(u);
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return uuids;
}

std::vector<klio::Sensor::Ptr> SQLite3Store::get_sensor_by_id(const std::string& sensor_id) {
    std::vector<klio::Sensor::Ptr> retval;
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    LOG("Attempting to load sensor " << sensor_id);
    checkSensorTable();

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            selectSensorByNameStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare sensor select statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_bind_text(stmt, 1, sensor_id.c_str(), -1, SQLITE_TRANSIENT);

    while (SQLITE_ROW == sqlite3_step(stmt)) {
        const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
        const unsigned char* select_name = sqlite3_column_text(stmt, 1);
        const unsigned char* select_desc = sqlite3_column_text(stmt, 2);
        const unsigned char* select_unit = sqlite3_column_text(stmt, 3);
        const unsigned char* select_timezone = sqlite3_column_text(stmt, 4);
        //std::cout << " -> " << select_uuid << " . " << select_name << " . " << select_desc << " . "<< select_timezone << std::endl;
        klio::Sensor::Ptr current(sensor_factory->createSensor(
                std::string((char*) select_uuid),
                std::string((char*) select_name),
                std::string((char*) select_desc),
                std::string((char*) select_unit),
                std::string((char*) select_timezone)));
        retval.push_back(current);
        //std::cout << "klio: Found sensor " << current->str() << std::endl;
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    //for( std::vector<klio::Sensor::Ptr>::iterator it = retval.begin(); 
    //    it < retval.end(); it++) {
    //  std::cout << "klio: Sensor id: " << (*it)->name() 
    //    << " - use count " << it->use_count() << std::endl;
    //}
    return retval;
}

klio::Sensor::Ptr SQLite3Store::get_sensor(const klio::Sensor::uuid_t& uuid) {
    int rc;
    sqlite3_stmt* stmt;
    const char* pzTail;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    LOG("Attempting to load sensor " << uuid);
    checkSensorTable();

    rc = sqlite3_prepare_v2(
            db, /* Database handle */
            selectSensorStmt.c_str(), /* SQL statement, UTF-8 encoded */
            -1, /* Maximum length of zSql in bytes - read complete string. */
            &stmt, /* OUT: Statement handle */
            &pzTail /* OUT: Pointer to unused portion of zSql */
            );
    if (rc != SQLITE_OK) {
        std::ostringstream oss;
        oss << "Can't prepare sensor select statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }

    sqlite3_bind_text(stmt, 1, to_string(uuid).c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) { // there MUST be only one result - otherwise, ve would have several primary IDs stored.
        std::ostringstream oss;
        oss << "Can't execute sensor select statement: " << sqlite3_errmsg(db) << ", error code " << rc;
        sqlite3_finalize(stmt);
        throw StoreException(oss.str());
    }
    const unsigned char* select_uuid = sqlite3_column_text(stmt, 0);
    const unsigned char* select_name = sqlite3_column_text(stmt, 1);
    const unsigned char* select_desc = sqlite3_column_text(stmt, 2);
    const unsigned char* select_unit = sqlite3_column_text(stmt, 3);
    const unsigned char* select_timezone = sqlite3_column_text(stmt, 4);
    //std::cout << " -> " << select_uuid << " . " << select_name << " . " << select_timezone << std::endl;

    klio::Sensor::Ptr retval(sensor_factory->createSensor(
            std::string((char*) select_uuid),
            std::string((char*) select_name),
            std::string((char*) select_desc),
            std::string((char*) select_unit),
            std::string((char*) select_timezone)));
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    return retval;
}

/**
 * Dummy helper for sqlite3_exec - dumps the return values.
 */
static int empty_callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++)
        printf("%s,\t", argv[i]);
    printf("\n");
    return 0;
}

void SQLite3Store::add_reading(klio::Sensor::Ptr sensor,
        timestamp_t timestamp, double value) {
    int rc;

    LOG("Adding to sensor: " << sensor->str()
            << " time=" << timestamp
            << " value=" << value);

    checkSensorTable();

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    std::ostringstream oss;
    oss << "INSERT INTO '" << sensor->uuid_string() << "' ";
    oss << "(timestamp, value) VALUES";
    oss << "(" << tc->convert_to_epoch(timestamp) << ", " << value << ");";
    std::string insertStmt = oss.str();

    //std::cout << "Using SQL: " << insertStmt << std::endl;

    char* zErrMsg = 0;
    rc = sqlite3_exec(db, insertStmt.c_str(), empty_callback, NULL, &zErrMsg);
    if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute value insertion statement: " << zErrMsg << ", error code " << rc;
        throw StoreException(oss.str());
    }
}

void SQLite3Store::add_description(klio::Sensor::Ptr sensor, const std::string& description) {
    int rc;

    LOG("Adding description " << description << " to sensor: " << sensor->str());

    checkSensorTable();

    // TODO: Insert statement
    std::ostringstream oss;
    oss << "UPDATE sensors ";
    oss << "SET description='" << description << "' ";
    oss << "WHERE uuid='" << sensor->uuid_string() << "'";
    std::string updateStmt(oss.str());


    char* zErrMsg = 0;
    rc = sqlite3_exec(db, updateStmt.c_str(), empty_callback, NULL, &zErrMsg);
    if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute description update statement: " << zErrMsg << ", error code " << rc;
        throw StoreException(oss.str());
    }

}

void SQLite3Store::update_readings(
        klio::Sensor::Ptr sensor, const readings_t& readings) {
    int rc;
    checkSensorTable();
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    Transaction transaction(db);
    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        klio::timestamp_t timestamp = (*it).first;
        double value = (*it).second;
        std::ostringstream oss;
        // see http://stackoverflow.com/questions/2717590/sqlite-upsert-on-duplicate-key-update
        // The SQL statement is constructed as two:
        // 1. If the timestamp exists, the "or ignore" part of stmt 1 silently 
        // ignores the insert statement.
        oss << "INSERT OR IGNORE INTO '" << sensor->uuid_string() << "' ";
        oss << "(timestamp, value) VALUES";
        oss << "(" << tc->convert_to_epoch(timestamp) << ", " << value << ");";
        // 2. Instead, the update statement inserts the last received value.
        oss << "UPDATE '" << sensor->uuid_string() << "' SET value='";
        oss << value << "' WHERE timestamp LIKE '";
        oss << tc->convert_to_epoch(timestamp) << "';";

        std::string insertStmt = oss.str();
        //std::cout << "Using SQL: " << insertStmt << std::endl;

        char* zErrMsg = 0;
        rc = sqlite3_exec(db, insertStmt.c_str(), empty_callback, NULL, &zErrMsg);
        if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
            std::ostringstream oss;
            oss << "Can't execute value insertion statement: " << zErrMsg << ", error code " << rc;
            throw StoreException(oss.str());
        }
    }
    transaction.commit();

}

void SQLite3Store::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
    int rc;
    checkSensorTable();
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    Transaction transaction(db);
    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        klio::timestamp_t timestamp = (*it).first;
        double value = (*it).second;
        std::ostringstream oss;
        oss << "INSERT INTO '" << sensor->uuid_string() << "' ";
        oss << "(timestamp, value) VALUES";
        oss << "(" << tc->convert_to_epoch(timestamp) << ", " << value << ");";
        std::string insertStmt = oss.str();

        //std::cout << "Using SQL: " << insertStmt << std::endl;

        char* zErrMsg = 0;
        rc = sqlite3_exec(db, insertStmt.c_str(), empty_callback, NULL, &zErrMsg);
        if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
            std::ostringstream oss;
            oss << "Can't execute value insertion statement: " << zErrMsg << ", error code " << rc;
            throw StoreException(oss.str());
        }
    }
    transaction.commit();
}

/**
 * Dummy helper for sqlite3_exec - dumps the return values.
 */
static int get_all_readings_callback(void *store, int argc, char **argv, char **azColName) {
    std::map<timestamp_t, double>* datastore;
    datastore = reinterpret_cast<std::map<timestamp_t, double>*> (store);
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    long epoch = atol(argv[0]);
    double value = atof(argv[1]);
    datastore->insert(
            std::pair<timestamp_t, double>(
            tc->convert_from_epoch(epoch),
            value
            )
            );
    //std::cout << "SQLite3Callback: " << tc->convert_from_epoch(epoch) << " - " << value 
    //  << ", size: " << datastore->size() << ", store " << datastore << std::endl;
    return 0;
}

readings_t_Ptr SQLite3Store::get_all_readings(
        klio::Sensor::Ptr sensor) {
    //std::map<timestamp_t, double> retval;//(new std::map<timestamp_t, double>());
    readings_t_Ptr retval(new readings_t());
    int rc;

    LOG("Retrieving all readings of sensor " << sensor->str());
    checkSensorTable();

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' ";
    std::string selectStmt = oss.str();
    //std::cout << "Using SQL: " << selectStmt << std::endl;

    char* zErrMsg = 0;
    rc = sqlite3_exec(db, selectStmt.c_str(), get_all_readings_callback, retval.get(), &zErrMsg);
    if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute reading select statement: " << zErrMsg << ", error code " << rc;
        throw StoreException(oss.str());
    }
    //std::cout << "Got " << retval->size() << " readings." << std::endl;

    return retval;
}

static int get_last_readings_callback(void *pair, int argc, char **argv, char **azColName) {
    std::pair<timestamp_t, double>* datastore;
    datastore = reinterpret_cast<std::pair <timestamp_t, double> *> (pair);
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    long epoch = atol(argv[0]);
    double value = atof(argv[1]);
    datastore->first = tc->convert_from_epoch(epoch);
    datastore->second = value;
    //std::cout << epoch << " - " << value << std::endl;
    return 0;
}

std::pair<timestamp_t, double> SQLite3Store::get_last_reading(
        klio::Sensor::Ptr sensor) {
    std::pair<timestamp_t, double> retval;
    int rc;

    LOG("Retrieving last readings of sensor " << sensor->str());
    checkSensorTable();

    std::ostringstream oss;
    oss << "SELECT timestamp, value FROM '" << sensor->uuid_string() << "' ";
    oss << "ORDER BY timestamp DESC LIMIT 1;";
    std::string selectStmt = oss.str();
    //std::cout << "Using SQL: " << selectStmt << std::endl;

    char* zErrMsg = 0;
    rc = sqlite3_exec(db, selectStmt.c_str(), get_last_readings_callback, &retval, &zErrMsg);
    if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute reading select statement: " << zErrMsg << ", error code " << rc;
        throw StoreException(oss.str());
    }
    return retval;
}

static int get_num_readings_callback(void *num, int argc, char **argv, char **azColName) {
    long int* numreadings = (long int*) num;
    *numreadings = atol(argv[0]);
    return 0;
}

unsigned long int SQLite3Store::get_num_readings(klio::Sensor::Ptr sensor) {
    long int retval;
    int rc;

    LOG("Retrieving number of readings for sensor " << sensor->str());
    checkSensorTable();

    std::ostringstream oss;
    oss << "SELECT Count(*) FROM '" << sensor->uuid_string() << "' ";
    std::string selectStmt = oss.str();
    //std::cout << "Using SQL: " << selectStmt << std::endl;

    char* zErrMsg = 0;
    rc = sqlite3_exec(db, selectStmt.c_str(), get_num_readings_callback, &retval, &zErrMsg);
    if (rc != SQLITE_OK) { // sqlite3_step has finished, no further result lines available
        std::ostringstream oss;
        oss << "Can't execute reading select statement: " << zErrMsg << ", error code " << rc;
        throw StoreException(oss.str());
    }
    return retval;
}

void SQLite3Store::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {

    sensor = get_sensor(sensor->uuid());

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}
