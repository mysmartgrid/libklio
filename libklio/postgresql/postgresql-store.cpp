#include <libklio/config.h>

#ifdef ENABLE_POSTGRESQL

#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/common.hpp>
#include "postgresql-transaction.hpp"
#include "postgresql-store.hpp"


using namespace klio;

const std::string PostgreSQLStore::CREATE_SENSORS_TABLE_STMT = "CREATE TABLE IF NOT EXISTS sensors (uuid VARCHAR(16), external_id VARCHAR(32), name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone INT, device_type_id INT, PRIMARY KEY(uuid))";
const std::string PostgreSQLStore::CREATE_READINGS_TABLE_STMT = "CREATE TABLE IF NOT EXISTS readings (uuid VARCHAR(16), timestamp INT, value REAL, PRIMARY KEY(uuid, timestamp))";
const std::string PostgreSQLStore::DROP_SENSORS_TABLE_STMT = "DROP TABLE sensors";
const std::string PostgreSQLStore::DROP_READINGS_TABLE_STMT = "DROP TABLE readings";
const std::string PostgreSQLStore::INSERT_SENSOR_STMT = "INSERT INTO sensors (uuid, external_id, name, description, unit, timezone, device_type_id) VALUES ($1, $2, $3, $4, $5, $6, $7)";
const std::string PostgreSQLStore::DELETE_SENSOR_STMT = "DELETE FROM sensors WHERE uuid = $1";
const std::string PostgreSQLStore::UPDATE_SENSOR_STMT = "UPDATE sensors SET external_id = $2, name = $3, description = $4, unit = $5, timezone = $6, device_type_id = $7 WHERE uuid = $1";
const std::string PostgreSQLStore::SELECT_SENSORS_STMT = "DECLARE CURSOR sensor_cursor FOR SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors";
const std::string PostgreSQLStore::SELECT_READINGS_STMT = "DECLARE CURSOR reading_cursor FOR SELECT timestamp, value FROM readings WHERE uuid = $1";
const std::string PostgreSQLStore::SELECT_TIMEFRAME_READINGS_STMT = "DECLARE CURSOR reading_cursor FOR SELECT timestamp, value FROM readings WHERE uuid = $1 AND timestamp BETWEEN $2 AND $3";
const std::string PostgreSQLStore::COUNT_READINGS_STMT = "DECLARE CURSOR sensor_cursor FOR SELECT COUNT(*) FROM readings WHERE uuid = $1";
const std::string PostgreSQLStore::SELECT_LAST_READING_STMT = "DECLARE CURSOR reading_cursor FOR SELECT timestamp, value FROM readings WHERE uuid = $1 ORDER BY timestamp DESC LIMIT 1";
const std::string PostgreSQLStore::SELECT_READING_STMT = "DECLARE CURSOR reading_cursor FOR SELECT timestamp, value FROM readings WHERE uuid = $1 AND timestamp = $2";
const std::string PostgreSQLStore::INSERT_READING_STMT = "INSERT INTO readings (uuid, timestamp, value) VALUES ($1, $2, $3)";
const std::string PostgreSQLStore::UPDATE_READING_STMT = "UPDATE readings SET timestamp = $2, value = $3 WHERE uuid = $1";
const std::string PostgreSQLStore::FETCH_SENSORS_STMT = "FETCH ALL IN sensor_cursor";
const std::string PostgreSQLStore::FETCH_READINGS_STMT = "FETCH ALL IN reading_cursor";

void PostgreSQLStore::open() {

    if (_connection == NULL) {
        open_db();

    } else {
        LOG("Store is already open.");
    }
}

void PostgreSQLStore::close() {

    if (_connection == NULL) {
        LOG("Store is already closed.");

    } else {

        if (_connection && _transaction) {
            _transaction->rollback();
        }

        clear_buffers();
        close_db();
    }
}

void PostgreSQLStore::open_db() {

    _connection = PQconnectdb(_info.c_str());

    if (PQstatus(_connection) == CONNECTION_OK) {

        if (_transaction) {
            _transaction->connection(_connection);

        } else {
            _transaction = create_transaction_handler();
        }

    } else {
        std::ostringstream oss;
        oss << PQerrorMessage(_connection);

        close_db();

        if (_transaction) {
            _transaction->connection(NULL);
        }
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::close_db() {

    PQfinish(_connection);

    if (PQstatus(_connection) != CONNECTION_OK) {
        _connection = NULL;

        if (_transaction) {
            _transaction->connection(NULL);
        }

    } else {
        std::ostringstream oss;
        oss << "Can't close the database.";
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::check_integrity() {
}

void PostgreSQLStore::initialize() {

    execute(CREATE_SENSORS_TABLE_STMT);
    execute(CREATE_READINGS_TABLE_STMT);
}

void PostgreSQLStore::prepare() {

    Store::prepare();
    //TODO
}

void PostgreSQLStore::dispose() {

    execute(DROP_SENSORS_TABLE_STMT);
    execute(DROP_READINGS_TABLE_STMT);
    close();
}

Transaction::Ptr PostgreSQLStore::get_transaction_handler() {

    return _auto_commit ? create_transaction_handler() : _transaction;
}

PostgreSQLTransaction::Ptr PostgreSQLStore::create_transaction_handler() {

    return PostgreSQLTransaction::Ptr(new PostgreSQLTransaction(_connection));
}

const std::string PostgreSQLStore::str() {

    std::ostringstream oss;
    oss << "PostgreSQL database: " << _info;
    return oss.str();
};

void PostgreSQLStore::add_sensor_record(const Sensor::Ptr sensor) {

    const char *paramValues[7];
    paramValues[0] = sensor->uuid_string().c_str();
    paramValues[1] = sensor->external_id().c_str();
    paramValues[2] = sensor->name().c_str();
    paramValues[3] = sensor->description().c_str();
    paramValues[4] = sensor->unit().c_str();
    paramValues[5] = sensor->timezone().c_str();
    paramValues[6] = std::to_string(sensor->device_type()->id()).c_str();

    execute(INSERT_SENSOR_STMT, paramValues);
}

void PostgreSQLStore::remove_sensor_record(const Sensor::Ptr sensor) {

    const char *paramValues[1];
    paramValues[0] = sensor->uuid_string().c_str();

    execute(DELETE_SENSOR_STMT, paramValues);
}

void PostgreSQLStore::update_sensor_record(const Sensor::Ptr sensor) {

    const char *paramValues[7];
    paramValues[0] = sensor->uuid_string().c_str();
    paramValues[1] = sensor->external_id().c_str();
    paramValues[2] = sensor->name().c_str();
    paramValues[3] = sensor->description().c_str();
    paramValues[4] = sensor->unit().c_str();
    paramValues[5] = sensor->timezone().c_str();
    paramValues[6] = std::to_string(sensor->device_type()->id()).c_str();

    execute(UPDATE_SENSOR_STMT, paramValues);
}

std::vector<Sensor::Ptr> PostgreSQLStore::get_sensor_records() {

    std::vector<Sensor::Ptr> sensors;

    PGresult* result = select(SELECT_SENSORS_STMT, FETCH_SENSORS_STMT);

    for (int row = 0; row < PQntuples(result); row++) {
        sensors.push_back(parse_sensor(result, row));
    }
    PQclear(result);
    return sensors;
}

readings_t_Ptr PostgreSQLStore::get_all_reading_records(const Sensor::Ptr sensor) {

    readings_t_Ptr readings;

    const char *paramValues[1];
    paramValues[0] = sensor->uuid_string().c_str();

    PGresult* result = select(SELECT_READINGS_STMT, FETCH_SENSORS_STMT, paramValues);

    for (int row = 0; row < PQntuples(result); row++) {
        readings->insert(parse_reading(result, row));
    }
    PQclear(result);
    return readings;
}

readings_t_Ptr PostgreSQLStore::get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    readings_t_Ptr readings;

    const char *paramValues[3];
    paramValues[0] = sensor->uuid_string().c_str();
    paramValues[1] = std::to_string(begin).c_str();
    paramValues[2] = std::to_string(end).c_str();

    PGresult* result = select(SELECT_TIMEFRAME_READINGS_STMT, FETCH_SENSORS_STMT, paramValues);

    for (int row = 0; row < PQntuples(result); row++) {
        readings->insert(parse_reading(result, row));
    }
    PQclear(result);
    return readings;
}

unsigned long int PostgreSQLStore::get_num_readings_value(const Sensor::Ptr sensor) {

    const char *paramValues[1];
    paramValues[0] = sensor->uuid_string().c_str();

    PGresult* result = select(COUNT_READINGS_STMT, FETCH_SENSORS_STMT, paramValues);

    int num = atoi(PQgetvalue(result, 0, 0));
    PQclear(result);
    return num;
}

reading_t PostgreSQLStore::get_last_reading_record(const Sensor::Ptr sensor) {

    const char *paramValues[1];
    paramValues[0] = sensor->uuid_string().c_str();

    PGresult* result = select(SELECT_LAST_READING_STMT, FETCH_SENSORS_STMT, paramValues);

    std::pair<timestamp_t, double> reading = parse_reading(result, 0);
    PQclear(result);
    return reading;
}

reading_t PostgreSQLStore::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    const char *paramValues[2];
    paramValues[0] = sensor->uuid_string().c_str();
    paramValues[1] = std::to_string(timestamp).c_str();

    PGresult* result = select(SELECT_READING_STMT, FETCH_SENSORS_STMT, paramValues);

    std::pair<timestamp_t, double> reading = parse_reading(result, 0);
    PQclear(result);
    return reading;
}

void PostgreSQLStore::add_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    add_reading_records(INSERT_READING_STMT, sensor, readings, ignore_errors);
}

void PostgreSQLStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    add_reading_records(UPDATE_READING_STMT, sensor, readings, ignore_errors);
}

void PostgreSQLStore::add_reading_records(const std::string statement, const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    const char *paramValues[3];
    paramValues[0] = sensor->uuid_string().c_str();

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        paramValues[1] = std::to_string(time_converter->convert_to_epoch((*it).first)).c_str();
        paramValues[2] = std::to_string((*it).second).c_str();

        try {
            execute(statement, paramValues);

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, (*it).first, (*it).second);
        }
    }
}

void PostgreSQLStore::execute(const std::string statement) {

    PGresult *result = PQexec(_connection, statement.c_str());
    check(result, PGRES_COMMAND_OK);
    PQclear(result);
}

void PostgreSQLStore::execute(const std::string statement, const char *paramValues[]) {

    PGresult *result = PQexecParams(_connection,
            statement.c_str(),
            1, /* one param */
            NULL, /* let the backend deduce param type */
            paramValues,
            NULL, /* don't need param lengths since text */
            NULL, /* default to all text params */
            1); /* ask for binary results */

    check(result, PGRES_COMMAND_OK);
    PQclear(result);
}

PGresult* PostgreSQLStore::select(const std::string select_statement, const std::string fetch_statement) {

    PGresult *result = PQexec(_connection, select_statement.c_str());
    check(result, PGRES_COMMAND_OK);
    PQclear(result);

    result = PQexec(_connection, fetch_statement.c_str());
    check(result, PGRES_TUPLES_OK);

    return result;
}

PGresult* PostgreSQLStore::select(const std::string select_statement, const std::string fetch_statement, const char *paramValues[]) {

    PGresult *result = PQexecParams(_connection,
            select_statement.c_str(),
            1, /* one param */
            NULL, /* let the backend deduce param type */
            paramValues,
            NULL, /* don't need param lengths since text */
            NULL, /* default to all text params */
            1); /* ask for binary results */

    check(result, PGRES_COMMAND_OK);
    PQclear(result);

    result = PQexec(_connection, fetch_statement.c_str());
    check(result, PGRES_TUPLES_OK);

    return result;
}

void PostgreSQLStore::check(PGresult *result, ExecStatusType expected_return) {

    ExecStatusType rc = PQresultStatus(result);

    if (rc != expected_return) {

        std::ostringstream oss;
        oss << "Can't execute SQL statement. Error: " << PQerrorMessage(_connection) << ", Error code: " << rc;
        PQclear(result);

        throw StoreException(oss.str());
    }
}

Sensor::Ptr PostgreSQLStore::parse_sensor(PGresult* result, int row) {

    return sensor_factory->createSensor(
            std::string((char*) PQgetvalue(result, row, 0)), //uuid
            std::string((char*) PQgetvalue(result, row, 1)), //external id
            std::string((char*) PQgetvalue(result, row, 2)), //name
            std::string((char*) PQgetvalue(result, row, 3)), //description
            std::string((char*) PQgetvalue(result, row, 4)), //unit
            std::string((char*) PQgetvalue(result, row, 5)), //timezone
            DeviceType::get_by_id(atoi(PQgetvalue(result, row, 6))) //device type
            );
}

reading_t PostgreSQLStore::parse_reading(PGresult* result, int row) {

    return std::pair<timestamp_t, double>(
            time_converter->convert_from_epoch(atoi(PQgetvalue(result, row, 0))),
            atof(PQgetvalue(result, row, 1))
            );
}

#endif /* ENABLE_POSTGRESQL */