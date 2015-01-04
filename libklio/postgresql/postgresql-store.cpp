#include <libklio/config.h>

#ifdef ENABLE_POSTGRESQL

#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <libklio/postgresql/postgresql-store.hpp>


using namespace klio;

const std::string PostgreSQLStore::DEFAULT_CONNECTION_INFO = "host=postgresql-server1 port=5432 dbname=kliostore user=kliouser password=12test34";

const char* PostgreSQLStore::HAS_TABLE_STMT = "HAS_TABLE";
const char* PostgreSQLStore::INSERT_SENSOR_STMT = "INSERT_SENSOR";
const char* PostgreSQLStore::DELETE_SENSOR_STMT = "DELETE_SENSOR";
const char* PostgreSQLStore::UPDATE_SENSOR_STMT = "UPDATE_SENSOR";
const char* PostgreSQLStore::SELECT_SENSORS_STMT = "SELECT_SENSORS";
const char* PostgreSQLStore::INSERT_READING_STMT = "INSERT_READING";
const char* PostgreSQLStore::COPY_READINGS_STMT = "COPY_READINGS";
const char* PostgreSQLStore::UPDATE_READING_STMT = "UPDATE_READING";
const char* PostgreSQLStore::SELECT_READINGS_STMT = "SELECT_READINGS";
const char* PostgreSQLStore::SELECT_TIMEFRAME_READINGS_STMT = "SELECT_TIMEFRAME_READINGS";
const char* PostgreSQLStore::COUNT_READINGS_STMT = "COUNT_READINGS";
const char* PostgreSQLStore::SELECT_LAST_READING_STMT = "SELECT_LAST_READING";
const char* PostgreSQLStore::SELECT_READING_STMT = "SELECT_READING";

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
        finalize_statements();

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
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::close_db() {

    try {
        PQfinish(_connection);
        clear_connection_references();

    } catch (std::exception const& e) {
        clear_connection_references();
        std::ostringstream oss;
        oss << "Can't close the database.";
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::clear_connection_references() {

    _connection = NULL;
    if (_transaction) {
        _transaction->connection(NULL);
    }
}

void PostgreSQLStore::check_integrity() {

    if (!(has_table("sensors") && has_table("readings"))) {
        std::ostringstream oss;
        oss << "Database is corrupt. Tables are missing.";
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::initialize() {

    execute("CREATE TABLE IF NOT EXISTS sensors (uuid VARCHAR(36), external_id VARCHAR(36), name VARCHAR(100), description VARCHAR(255), unit VARCHAR(20), timezone VARCHAR(30), device_type_id INTEGER, PRIMARY KEY(uuid))");
    execute("CREATE TABLE IF NOT EXISTS readings (uuid VARCHAR(36), timestamp INTEGER, value FLOAT8, PRIMARY KEY(uuid, timestamp))");
}

void PostgreSQLStore::prepare() {

    prepare_statements();
    Store::prepare();
}

void PostgreSQLStore::dispose() {

    execute("DROP TABLE readings");
    execute("DROP TABLE sensors");
    close();
}

Transaction::Ptr PostgreSQLStore::get_transaction_handler() {

    return _auto_commit ? create_transaction_handler() : _transaction;
}

PostgreSQLTransaction::Ptr PostgreSQLStore::create_transaction_handler() {

    return PostgreSQLTransaction::Ptr(new PostgreSQLTransaction(_connection));
}

const bool PostgreSQLStore::has_table(const char* name) {

    const char* params[1];
    params[0] = name;

    PGresult* result = NULL;
    try {
        result = execute(HAS_TABLE_STMT, params, 1, PGRES_TUPLES_OK);
        clear(result);
        return true;

    } catch (std::exception const& e) {
        clear(result);
        return false;
    }
}

const std::string PostgreSQLStore::str() {

    std::ostringstream oss;
    oss << "PostgreSQL database";

    if (_connection) {
        oss << ": " << PQuser(_connection) << "@" << PQdb(_connection);
    }

    return oss.str();
};

void PostgreSQLStore::add_sensor_record(const Sensor::Ptr sensor) {

    const char* params[7];
    params[0] = sensor->uuid_string().c_str();
    params[1] = sensor->external_id().c_str();
    params[2] = sensor->name().c_str();
    params[3] = sensor->description().c_str();
    params[4] = sensor->unit().c_str();
    params[5] = sensor->timezone().c_str();
    params[6] = std::to_string(sensor->device_type()->id()).c_str();

    execute(INSERT_SENSOR_STMT, params, 7);
}

void PostgreSQLStore::remove_sensor_record(const Sensor::Ptr sensor) {

    const char* params[1];
    params[0] = sensor->uuid_string().c_str();

    execute(DELETE_SENSOR_STMT, params, 1);
}

void PostgreSQLStore::update_sensor_record(const Sensor::Ptr sensor) {

    const char* params[7];
    params[0] = sensor->uuid_string().c_str();
    params[1] = sensor->external_id().c_str();
    params[2] = sensor->name().c_str();
    params[3] = sensor->description().c_str();
    params[4] = sensor->unit().c_str();
    params[5] = sensor->timezone().c_str();
    params[6] = std::to_string(sensor->device_type()->id()).c_str();

    execute(UPDATE_SENSOR_STMT, params, 7);
}

std::vector<Sensor::Ptr> PostgreSQLStore::get_sensor_records() {

    PGresult* result = NULL;
    try {
        result = execute(SELECT_SENSORS_STMT, NULL, 0, PGRES_TUPLES_OK);

        std::vector<Sensor::Ptr> sensors;
        int num_rows = PQntuples(result);

        for (int row = 0; row < num_rows; row++) {
            sensors.push_back(parse_sensor(result, row));
        }
        clear(result);
        return sensors;

    } catch (std::exception const& e) {
        clear(result);
        throw e;
    }
}

readings_t_Ptr PostgreSQLStore::get_all_reading_records(const Sensor::Ptr sensor) {

    const char* params[1];
    params[0] = sensor->uuid_string().c_str();

    return get_reading_records(SELECT_READINGS_STMT, params, 1);
}

readings_t_Ptr PostgreSQLStore::get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    const char* params[3];
    params[0] = sensor->uuid_string().c_str();
    params[1] = std::to_string(begin).c_str();
    params[2] = std::to_string(end).c_str();

    return get_reading_records(SELECT_TIMEFRAME_READINGS_STMT, params, 3);
}

unsigned long int PostgreSQLStore::get_num_readings_value(const Sensor::Ptr sensor) {

    const char* params[1];
    params[0] = sensor->uuid_string().c_str();

    PGresult* result = NULL;
    try {
        result = execute(COUNT_READINGS_STMT, params, 1, PGRES_TUPLES_OK);
        unsigned long int num = get_long_value(result, 0, 0);

        clear(result);
        return num;

    } catch (std::exception const& e) {
        clear(result);
        throw e;
    }
}

reading_t PostgreSQLStore::get_last_reading_record(const Sensor::Ptr sensor) {

    const char* params[1];
    params[0] = sensor->uuid_string().c_str();

    return *(get_reading_records(SELECT_LAST_READING_STMT, params, 1)->begin());
}

reading_t PostgreSQLStore::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    const char* params[2];
    params[0] = sensor->uuid_string().c_str();
    params[1] = std::to_string(timestamp).c_str();

    return *(get_reading_records(SELECT_READING_STMT, params, 2)->begin());
}

void PostgreSQLStore::add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors) {

    const std::string timestamp_str = std::to_string(time_converter->convert_to_epoch(timestamp));
    const std::string value_str = std::to_string(value);

    const char* params[3];
    params[0] = sensor->uuid_string().c_str();
    params[1] = timestamp_str.c_str();
    params[2] = value_str.c_str();

    try {
        execute(INSERT_READING_STMT, params, 3);

    } catch (std::exception const& e) {
        handle_reading_insertion_error(ignore_errors, timestamp, value);
    }
}

void PostgreSQLStore::add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    std::ostringstream oss;
    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        oss << sensor->uuid_string() << "," <<
                time_converter->convert_to_epoch((*it).first) << "," <<
                (*it).second << std::endl;
    }
    const std::string buffer = oss.str();
    oss.clear();

    clear_results();
    //FIXME
    //check(result, PGRES_COMMAND_OK);

    PGresult* result = NULL;
    try {
        result = execute(COPY_READINGS_STMT, NULL, 0, PGRES_COPY_IN);
        clear(result);
        result = NULL;

        int code = PQputCopyData(_connection, buffer.c_str(), buffer.length());
        check(code);

        code = PQputCopyEnd(_connection, NULL);
        check(code);

    } catch (std::exception const& e) {
        clear(result);
        handle_reading_insertion_error(ignore_errors, sensor);
    }
}

void PostgreSQLStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    const char* params[3];
    params[0] = sensor->uuid_string().c_str();

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        const std::string timestamp = std::to_string(time_converter->convert_to_epoch((*it).first));
        const std::string value = std::to_string((*it).second);

        params[1] = timestamp.c_str();
        params[2] = value.c_str();

        try {
            execute(UPDATE_READING_STMT, params, 3);

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, (*it).first, (*it).second);
        }
    }
}

void PostgreSQLStore::prepare_statements() {

    prepare_statement(HAS_TABLE_STMT,
            "SELECT 1 FROM pg_catalog.pg_class WHERE relname = $1 AND relkind = 'r'", 1);

    prepare_statement(INSERT_SENSOR_STMT,
            "INSERT INTO sensors (uuid, external_id, name, description, unit, timezone, device_type_id) VALUES ($1::varchar, $2::varchar, $3::varchar, $4::varchar, $5::varchar, $6::varchar, $7::integer)", 7);

    prepare_statement(DELETE_SENSOR_STMT,
            "DELETE FROM sensors WHERE uuid = $1::varchar", 1);

    prepare_statement(UPDATE_SENSOR_STMT,
            "UPDATE sensors SET external_id = $2::varchar, name = $3::varchar, description = $4::varchar, unit = $5::varchar, timezone = $6::varchar, device_type_id = $7::integer WHERE uuid = $1::varchar", 7);

    prepare_statement(SELECT_SENSORS_STMT,
            "SELECT uuid, external_id, name, description, unit, timezone, device_type_id FROM sensors", 0);

    prepare_statement(INSERT_READING_STMT,
            "INSERT INTO readings (uuid, timestamp, value) VALUES ($1::varchar, $2::integer, $3::float8)", 3);

    prepare_statement(COPY_READINGS_STMT,
            "COPY readings (uuid, timestamp, value) FROM STDIN (FORMAT csv, DELIMITER ',')", 0);

    prepare_statement(UPDATE_READING_STMT,
            "UPDATE readings SET timestamp = $2::integer, value = $3::float8 WHERE uuid = $1::varchar", 3);

    prepare_statement(SELECT_READINGS_STMT,
            "SELECT timestamp, value FROM readings WHERE uuid = $1::varchar", 1);

    prepare_statement(SELECT_TIMEFRAME_READINGS_STMT,
            "SELECT timestamp, value FROM readings WHERE uuid = $1::varchar AND timestamp BETWEEN $2::integer AND $3::integer", 3);

    prepare_statement(COUNT_READINGS_STMT,
            "SELECT COUNT(*) FROM readings WHERE uuid = $1::varchar", 3);

    prepare_statement(SELECT_LAST_READING_STMT,
            "SELECT timestamp, value FROM readings WHERE uuid = $1::varchar ORDER BY timestamp DESC LIMIT 1", 1);

    prepare_statement(SELECT_READING_STMT,
            "SELECT timestamp, value FROM readings WHERE uuid = $1::varchar AND timestamp = $2::integer", 1);
}

void PostgreSQLStore::prepare_statement(const char* statement_name, const char* statement, const int num_params) {

    PGresult* result = NULL;
    try {
        result = PQprepare(_connection, statement_name, statement, num_params, NULL);
        check(result, PGRES_COMMAND_OK);
        clear(result);

    } catch (std::exception const& e) {
        clear(result);
        throw e;
    }
}

void PostgreSQLStore::finalize_statements() {

    execute("DEALLOCATE ALL");
}

readings_t_Ptr PostgreSQLStore::get_reading_records(const char* statement_name, const char* params[], const int num_params) {

    //Previous insertions
    clear_results();

    PGresult* result = NULL;
    try {
        result = execute(statement_name, params, num_params, PGRES_TUPLES_OK);

        readings_t_Ptr readings(new readings_t());
        int num_rows = PQntuples(result);

        for (int row = 0; row < num_rows; row++) {
            readings->insert(parse_reading(result, row));
        }

        clear(result);
        return readings;

    } catch (std::exception const& e) {
        clear(result);
        throw e;
    }
}

void PostgreSQLStore::execute(const char* statement) {

    PGresult* result = NULL;
    try {
        result = PQexec(_connection, statement);
        check(result, PGRES_COMMAND_OK);
        clear(result);

    } catch (std::exception const& e) {
        clear(result);
        throw e;
    }
}

void PostgreSQLStore::execute(const char* statement_name, const char* params[], const int num_params) {

    if (_synchronous) {

        PGresult* result = NULL;
        try {
            result = execute(statement_name, params, num_params, PGRES_COMMAND_OK);
            clear(result);

        } catch (std::exception const& e) {
            clear(result);
            throw e;
        }
    } else {
        int code = PQsendQueryPrepared(_connection, statement_name, num_params, params, NULL, NULL, 0);
        check(code);
    }
}

PGresult* PostgreSQLStore::execute(const char* statement_name, const char* params[], const int num_params, const ExecStatusType expected_status) {

    PGresult* result = PQexecPrepared(_connection, statement_name, num_params, params, NULL, NULL, 0);
    check(result, expected_status);
    return result;
}

void PostgreSQLStore::check(PGresult* result, const ExecStatusType expected_status) {

    if (result) {

        ExecStatusType status = PQresultStatus(result);

        if (status != expected_status) {

            std::ostringstream oss;
            oss << "Can't execute SQL statement. Error: " << PQerrorMessage(_connection) << ", Error code: " << status;
            clear(result);
            throw StoreException(oss.str());
        }
    } else {
        std::ostringstream oss;
        oss << "Can't execute SQL statement.";
        throw EnvironmentException(oss.str());
    }
}

void PostgreSQLStore::check(const int result) {

    if (result < 0) {
        std::ostringstream oss;
        oss << "Can't execute SQL statement. Invalid return code: " << result;
        throw StoreException(oss.str());
    }
}

void PostgreSQLStore::clear_results() {

    PGresult* result = NULL;
    do {
        clear(result);
        result = PQgetResult(_connection);
    } while (result);
}

void PostgreSQLStore::clear(PGresult* result) {

    if (result) {
        PQclear(result);
    }
}

Sensor::Ptr PostgreSQLStore::parse_sensor(PGresult* result, const int row) {

    return sensor_factory->createSensor(
            get_string_value(result, row, 0), //uuid
            get_string_value(result, row, 1), //external id
            get_string_value(result, row, 2), //name
            get_string_value(result, row, 3), //description
            get_string_value(result, row, 4), //unit
            get_string_value(result, row, 5), //timezone
            DeviceType::get_by_id(get_long_value(result, row, 6)) //device type
            );
}

reading_t PostgreSQLStore::parse_reading(PGresult* result, const int row) {

    return std::pair<timestamp_t, double>(
            time_converter->convert_from_epoch(get_long_value(result, row, 0)),
            get_double_value(result, row, 1)
            );
}

std::string PostgreSQLStore::get_string_value(PGresult* result, const int row, const int col) {

    return std::string(PQgetvalue(result, row, col));
}

unsigned long int PostgreSQLStore::get_long_value(PGresult* result, const int row, const int col) {

    return atol(PQgetvalue(result, row, col));
}

double PostgreSQLStore::get_double_value(PGresult* result, const int row, const int col) {

    return atof(PQgetvalue(result, row, col));
}

#endif /* ENABLE_POSTGRESQL */