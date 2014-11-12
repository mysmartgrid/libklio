#include <libklio/config.h>

#ifdef ENABLE_REDIS3M

#include <libklio/sensor-factory.hpp>
#include "redis-store.hpp"


using namespace klio;

const std::string RedisStore::DEFAULT_REDIS_HOST = "redis-server1";
const unsigned int RedisStore::DEFAULT_REDIS_PORT = 6379;
const unsigned int RedisStore::DEFAULT_REDIS_DB = 0;

const std::string RedisStore::SENSORS_KEY = "libklio:sensors";
const std::string RedisStore::NOARG = "";

const std::string RedisStore::SET = "SET";
const std::string RedisStore::GET = "GET";
const std::string RedisStore::EXISTS = "EXISTS";
const std::string RedisStore::DEL = "DEL";

const std::string RedisStore::HSET = "HSET";
const std::string RedisStore::HGET = "HGET";
const std::string RedisStore::HDEL = "HDEL";

const std::string RedisStore::SADD = "SADD";
const std::string RedisStore::SMEMBERS = "SMEMBERS";
const std::string RedisStore::SISMEMBER = "SISMEMBER";
const std::string RedisStore::SREM = "SREM";

const std::string RedisStore::SELECT = "SELECT";
const std::string RedisStore::FLUSHDB = "FLUSHDB";
const std::string RedisStore::SAVE = "SAVE";

void RedisStore::open() {

    if (!_connection) {
        _connection = redis3m::connection::create(_host, _port);
        select(_db);
    }
}

void RedisStore::close() {

    save();
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
    flushdb();
    close();
}

const std::string RedisStore::str() {

    std::ostringstream oss;
    oss << "Redis store " << _host << ":" << _port;
    return oss.str();
}

void RedisStore::add_sensor_record(const Sensor::Ptr sensor) {

    const std::string sensor_key = check_sensor_existence(sensor, false);
    sadd(SENSORS_KEY, sensor_key);

    hset(sensor_key, "uuid", sensor->uuid_string());
    hset(sensor_key, "external_id", sensor->external_id());
    hset(sensor_key, "name", sensor->name());
    hset(sensor_key, "description", sensor->description());
    hset(sensor_key, "unit", sensor->unit());
    hset(sensor_key, "timezone", sensor->timezone());
}

void RedisStore::remove_sensor_record(const Sensor::Ptr sensor) {

    const std::string sensor_key = check_sensor_existence(sensor, true);
    srem(SENSORS_KEY, sensor_key);

    hdel(sensor_key, "uuid");
    hdel(sensor_key, "external_id");
    hdel(sensor_key, "name");
    hdel(sensor_key, "description");
    hdel(sensor_key, "unit");
    hdel(sensor_key, "timezone");

    const std::vector<timestamp_t> timestamps = get_timestamps(sensor);

    for (std::vector<timestamp_t>::const_iterator timestamp = timestamps.begin(); timestamp != timestamps.end(); timestamp++) {

        del(compose_reading_key(sensor, *timestamp));
    }
    del(compose_timestamps_key(sensor));
}

void RedisStore::update_sensor_record(const Sensor::Ptr sensor) {

    const std::string sensor_key = check_sensor_existence(sensor, true);

    hset(sensor_key, "external_id", sensor->external_id());
    hset(sensor_key, "name", sensor->name());
    hset(sensor_key, "description", sensor->description());
    hset(sensor_key, "unit", sensor->unit());
    hset(sensor_key, "timezone", sensor->timezone());
}

std::vector<Sensor::Ptr> RedisStore::get_sensor_records() {

    std::vector<Sensor::Ptr> sensors;

    const std::vector<std::string> sensor_keys = smembers(SENSORS_KEY);

    for (std::vector<std::string>::const_iterator sensor_key = sensor_keys.begin(); sensor_key != sensor_keys.end(); sensor_key++) {

        sensors.push_back(sensor_factory->createSensor(
                hget(*sensor_key, "uuid"),
                hget(*sensor_key, "external_id"),
                hget(*sensor_key, "name"),
                hget(*sensor_key, "description"),
                hget(*sensor_key, "unit"),
                hget(*sensor_key, "timezone")));
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
                get_double_value(reading_key)));
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

    if (exists(reading_key)) {
        reading.first = timestamp;
        reading.second = get_double_value(reading_key);
    }
    return reading;
}

void RedisStore::add_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    check_sensor_existence(sensor, true);
    const std::string timestamps_key = compose_timestamps_key(sensor);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        const timestamp_t timestamp = (*it).first;
        const double value = (*it).second;

        try {
            std::string reading_key = compose_reading_key(sensor, timestamp);
            set(reading_key, std::to_string(value));
            sadd(timestamps_key, std::to_string(timestamp));

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, timestamp, value);
        }
    }
}

void RedisStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    add_reading_records(sensor, readings, ignore_errors);
}

void RedisStore::clear_buffers() {
    Store::clear_buffers();
}

const std::string RedisStore::check_sensor_existence(const Sensor::Ptr sensor, const bool should_exist) {

    const std::string sensor_key = compose_sensor_key(sensor);

    //sensor exists
    if (sismember(SENSORS_KEY, sensor_key)) {

        //it should not exist
        if (!should_exist) {
            throw StoreException("Sensor already exists.");
        }

        //sensor does not exist, but it should
    } else if (should_exist) {
        throw StoreException("Sensor not found.");
    }
    return sensor_key;
}

const double RedisStore::get_double_value(const std::string& key) {
    return atof(get(key).c_str());
}

const std::vector<timestamp_t> RedisStore::get_timestamps(const Sensor::Ptr sensor) {

    std::vector<timestamp_t> timestamps;
    const std::string key = compose_timestamps_key(sensor);
    const std::vector<std::string> members = smembers(key);

    for (std::vector<std::string>::const_iterator it = members.begin(); it != members.end(); it++) {

        timestamps.push_back(atol((*it).c_str()));
    }
    return timestamps;
}

void RedisStore::set(const std::string& key, const std::string& value) {

    run(SET, key, value, NOARG);
}

const std::string RedisStore::get(const std::string& key) {

    return run(GET, key, NOARG, NOARG).str();
}

const bool RedisStore::exists(const std::string& key) {

    return run(EXISTS, key, NOARG, NOARG).integer() == 1;
}

void RedisStore::del(const std::string& key) {

    run(DEL, key, NOARG, NOARG);
}

void RedisStore::hset(const std::string& key, const std::string& field, const std::string& value) {

    run(HSET, key, field, value);
}

const std::string RedisStore::hget(const std::string& key, const std::string& field) {

    return run(HGET, key, field, NOARG).str();
}

void RedisStore::hdel(const std::string& key, const std::string& field) {

    run(HDEL, key, field, NOARG);
}

void RedisStore::sadd(const std::string& key, const std::string& value) {

    run(SADD, key, value, NOARG);
}

const std::vector<std::string> RedisStore::smembers(const std::string& key) {

    const std::vector<redis3m::reply> replies = run(SMEMBERS, key, NOARG, NOARG).elements();

    std::vector<std::string> values;
    for (std::vector<redis3m::reply>::const_iterator reply = replies.begin(); reply != replies.end(); reply++) {
        values.push_back((*reply).str());
    }
    return values;
}

const bool RedisStore::sismember(const std::string& key, const std::string& value) {

    return (run(SISMEMBER, key, value, NOARG).integer() == 1);
}

void RedisStore::srem(const std::string& key, const std::string& value) {

    run(SREM, key, value, NOARG);
}

void RedisStore::select(const unsigned int index) {

    std::ostringstream oss;
    oss << index;
    run(SELECT, oss.str(), NOARG, NOARG);
}

void RedisStore::flushdb() {

    run(FLUSHDB, NOARG, NOARG, NOARG);
}

void RedisStore::save() {

    run(SAVE, NOARG, NOARG, NOARG);
}

redis3m::reply RedisStore::run(const std::string& command, const std::string& arg1, const std::string& arg2, const std::string& arg3) {

    std::vector<std::string> args;
    args.push_back(command);

    if (!arg1.empty()) {
        args.push_back(arg1);
    }

    if (!arg2.empty()) {
        args.push_back(arg2);
    }

    if (!arg3.empty()) {
        args.push_back(arg3);
    }

    return _connection->run(args);
}

const std::string RedisStore::compose_sensor_key(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << "libklio:sensor:" << sensor->uuid_string();
    return oss.str();
}

const std::string RedisStore::compose_timestamps_key(const Sensor::Ptr sensor) {

    std::ostringstream oss;
    oss << compose_sensor_key(sensor) << ":readings";
    return oss.str();
}

const std::string RedisStore::compose_reading_key(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    std::ostringstream oss;
    oss << compose_sensor_key(sensor) << ":reading:" << timestamp;
    return oss.str();
}

#endif /* ENABLE_REDIS3M */