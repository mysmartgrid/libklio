#include <libklio/config.h>

#ifdef ENABLE_REDIS3M

#include <libklio/sensor-factory.hpp>
#include "redis-store.hpp"


using namespace klio;

const std::string RedisStore::DEFAULT_REDIS_HOST = "redis-server1";
const unsigned int RedisStore::DEFAULT_REDIS_PORT = 6379;
const unsigned int RedisStore::DEFAULT_REDIS_DB = 0;
const std::string RedisStore::OK = "OK";

const std::string RedisStore::SENSORS_KEY = "libklio:sensors";
const std::string RedisStore::SENSOR_KEY = "libklio:sensor:";
const std::string RedisStore::READINGS_KEY = ":readings";
const std::string RedisStore::READING_KEY = ":reading:";

const std::string RedisStore::SET = "SET";
const std::string RedisStore::GET = "GET";
const std::string RedisStore::EXISTS = "EXISTS";
const std::string RedisStore::DEL = "DEL";

const std::string RedisStore::SADD = "SADD";
const std::string RedisStore::SMEMBERS = "SMEMBERS";
const std::string RedisStore::SREM = "SREM";

const std::string RedisStore::HMSET = "HMSET";
const std::string RedisStore::HMGET = "HMGET";
const std::string RedisStore::HDEL = "HDEL";

const std::string RedisStore::SELECT = "SELECT";
const std::string RedisStore::FLUSHDB = "FLUSHDB";
const std::string RedisStore::SAVE = "SAVE";

void RedisStore::open() {

    if (!_connection) {
        _connection = connection::create(_host, _port);
        run_select(_db);
        _transaction = create_transaction_handler();
    }
}

void RedisStore::close() {

    //FIXME: close connection
    if (_connection && _transaction) {
        _transaction->rollback();
    }
    clear_buffers();
}

void RedisStore::check_integrity() {

    if (!_connection->is_valid()) {
        std::ostringstream oss;
        oss << "The Redis database connection is invalid.";
        throw StoreException(oss.str());
    }
}

void RedisStore::initialize() {
}

void RedisStore::dispose() {

    if (_connection) {
        run_flushdb();
    }
    close();
}

Transaction::Ptr RedisStore::get_transaction_handler() {

    return _auto_commit ? create_transaction_handler() : _transaction;
}

RedisTransaction::Ptr RedisStore::create_transaction_handler() {

    return RedisTransaction::Ptr(new RedisTransaction(_connection));
}

const std::string RedisStore::str() {

    std::ostringstream oss;
    oss << "Redis store " << _host << ":" << _port << " db:" << _db;
    return oss.str();
}

void RedisStore::add_sensor_record(const Sensor::Ptr sensor) {

    const std::string key = check_sensor_existence(sensor, false);

    run_hmset_sensor(key, sensor);
    run_sadd(SENSORS_KEY, key);
}

void RedisStore::remove_sensor_record(const Sensor::Ptr sensor) {

    const std::string key = check_sensor_existence(sensor, true);

    run_hdel_sensor(key);
    run_del_readings(sensor);
    run_srem(SENSORS_KEY, key);
}

void RedisStore::update_sensor_record(const Sensor::Ptr sensor) {

    const std::string key = check_sensor_existence(sensor, true);
    
    run_hmset_sensor(key, sensor);
}

std::vector<Sensor::Ptr> RedisStore::get_sensor_records() {

    std::vector<Sensor::Ptr> sensors;
    const std::vector<reply> keys = run_smembers(SENSORS_KEY);

    for (std::vector<reply>::const_iterator key = keys.begin(); key != keys.end(); key++) {

        sensors.push_back(
                run_hmget_sensor((*key).str())
                );
    }
    return sensors;
}

readings_t_Ptr RedisStore::get_all_reading_records(const Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    check_sensor_existence(sensor, true);
    readings_t_Ptr readings(new readings_t());

    const std::vector<timestamp_t> timestamps = get_timestamps(sensor);

    for (std::vector<timestamp_t>::const_iterator timestamp = timestamps.begin(); timestamp != timestamps.end(); timestamp++) {

        std::string reading_key = compose_reading_key(sensor, *timestamp);

        readings->insert(
                std::pair<timestamp_t, double>(
                *timestamp,
                get_double_value(reading_key)
                ));
    }
    return readings;
}

readings_t_Ptr RedisStore::get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    //TODO: make this method more efficient
    check_sensor_existence(sensor, true);
    readings_t_Ptr readings(new readings_t());
    const std::vector<timestamp_t> timestamps = get_timestamps(sensor);

    for (std::vector<timestamp_t>::const_iterator timestamp = timestamps.begin(); timestamp != timestamps.end(); timestamp++) {

        if (*timestamp >= begin && *timestamp <= end) {

            std::string reading_key = compose_reading_key(sensor, *timestamp);

            readings->insert(
                    std::pair<timestamp_t, double>(
                    *timestamp,
                    get_double_value(reading_key)
                    ));
        }
    }
    return readings;
}

unsigned long int RedisStore::get_num_readings_value(const Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    return get_all_readings(sensor)->size();
}

reading_t RedisStore::get_last_reading_record(const Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    readings_t_Ptr readings = get_all_readings(sensor);

    if (readings->size() > 0) {
        return *(readings->end());

    } else {
        return std::pair<timestamp_t, double>(0, 0);
    }
}

reading_t RedisStore::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    check_sensor_existence(sensor, true);

    std::pair<timestamp_t, double> reading = std::pair<timestamp_t, double>(0, 0);
    std::string reading_key = compose_reading_key(sensor, timestamp);

    if (run_exists(reading_key)) {
        reading.first = timestamp;
        reading.second = get_double_value(reading_key);
    }
    return reading;
}

void RedisStore::add_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    check_sensor_existence(sensor, true);

    command sadd = command(SADD);
    sadd << compose_timestamps_key(sensor);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        const timestamp_t timestamp = (*it).first;
        const double value = (*it).second;

        try {
            run_set(compose_reading_key(sensor, timestamp), value);

            sadd << std::to_string(timestamp);

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, timestamp, value);
        }
    }
    _connection->run(sadd);
}

void RedisStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    add_reading_records(sensor, readings, ignore_errors);
}

const std::string RedisStore::check_sensor_existence(const Sensor::Ptr sensor, const bool should_exist) {

    //sensor exists
    if (_sensors_buffer.count(sensor->uuid()) > 0) {

        //it should not exist
        if (!should_exist) {
            throw StoreException("Sensor already exists.");
        }

        //sensor does not exist, but it should
    } else if (should_exist) {
        throw StoreException("Sensor not found.");
    }
    return compose_sensor_key(sensor);
}

const double RedisStore::get_double_value(const std::string& key) {

    return atof(run_get(key).c_str());
}

const std::vector<timestamp_t> RedisStore::get_timestamps(const Sensor::Ptr sensor) {

    std::vector<timestamp_t> timestamps;
    const std::string key = compose_timestamps_key(sensor);
    const std::vector<reply> members = run_smembers(key);

    for (std::vector<reply>::const_iterator member = members.begin(); member != members.end(); member++) {

        timestamps.push_back(atol((*member).str().c_str()));
    }
    return timestamps;
}

void RedisStore::run_set(const std::string& key, const double& value) {

    _connection->run(command(SET)(key) (std::to_string(value)));
}

const std::string RedisStore::run_get(const std::string& key) {

    return _connection->run(command(GET)(key)).str();
}

const bool RedisStore::run_exists(const std::string& key) {

    return _connection->run(command(EXISTS)(key)).integer() > 0;
}

void RedisStore::run_del(const std::string& key) {

    _connection->run(command(DEL)(key));
}

void RedisStore::run_del_readings(const Sensor::Ptr sensor) {

    const std::vector<timestamp_t> timestamps = get_timestamps(sensor);

    command del = command(DEL);

    for (std::vector<timestamp_t>::const_iterator timestamp = timestamps.begin(); timestamp != timestamps.end(); timestamp++) {

        del << compose_reading_key(sensor, *timestamp);
    }
    del << compose_timestamps_key(sensor);
    _connection->run(del);
}

void RedisStore::run_hmset_sensor(const std::string& key, const Sensor::Ptr sensor) {

    _connection->run(command(HMSET)(key)
            ("uuid") (sensor->uuid_string())
            ("external_id") (sensor->external_id())
            ("name") (sensor->name())
            ("description") (sensor->description())
            ("unit") (sensor->unit())
            ("timezone") (sensor->timezone())
            );
}

const Sensor::Ptr RedisStore::run_hmget_sensor(const std::string& key) {

    const std::vector<reply> replies = _connection->run(command(HMGET)(key)
            ("uuid") ("external_id")
            ("name") ("description")
            ("unit") ("timezone")).elements();

    return sensor_factory->createSensor(
            replies.at(0).str(),
            replies.at(1).str(),
            replies.at(2).str(),
            replies.at(3).str(),
            replies.at(4).str(),
            replies.at(5).str()
            );
}

void RedisStore::run_hdel_sensor(const std::string& key) {

    _connection->run(command(HDEL)(key)
            ("uuid") ("external_id")
            ("name") ("description")
            ("unit") ("timezone")
            );
}

void RedisStore::run_sadd(const std::string& key, const std::string& value) {

    _connection->run(command(SADD)(key) (value));
}

const std::vector<reply> RedisStore::run_smembers(const std::string& key) {

    return _connection->run(command(SMEMBERS)(key)).elements();
}

void RedisStore::run_srem(const std::string& key, const std::string& value) {

    _connection->run(command(SREM)(key) (value));
}

void RedisStore::run_select(const unsigned int index) {

    reply status = _connection->run(command(SELECT)(std::to_string(index)));
    if (status.str() != OK) {
        throw StoreException("DB cannot be selected.");
    }
}

void RedisStore::run_flushdb() {

    _connection->run(command(FLUSHDB));
}

const std::string RedisStore::compose_sensor_key(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << SENSOR_KEY << sensor->uuid_string();
    return oss.str();
}

const std::string RedisStore::compose_timestamps_key(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << compose_sensor_key(sensor) << READINGS_KEY;
    return oss.str();
}

const std::string RedisStore::compose_reading_key(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    std::ostringstream oss;
    oss << compose_sensor_key(sensor) << READING_KEY << timestamp;
    return oss.str();
}

#endif /* ENABLE_REDIS3M */