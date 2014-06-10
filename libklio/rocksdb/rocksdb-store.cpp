#include <libklio/config.h>

#ifdef ENABLE_ROCKSDB

#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include "rocksdb-store.hpp"


using namespace klio;

void RocksDBStore::open() {

    if (bfs::exists(_path) && bfs::is_directory(_path) && _db_buffer.empty()) {

        std::vector<Sensor::uuid_t> uuids = get_sensor_uuids();

        for (std::vector<Sensor::uuid_t>::iterator uuid = uuids.begin(); uuid != uuids.end(); uuid++) {

            const std::string uuid_str = boost::uuids::to_string(*uuid);
            open_db(false, false, compose_sensor_properties_path(uuid_str));
            open_db(true, false, compose_sensor_readings_path(uuid_str));
        }
    }
}

void RocksDBStore::close() {

    Store::flush();
    clear_buffers();

    for (std::map<std::string, rocksdb::DB*>::const_iterator it = _db_buffer.begin(); it != _db_buffer.end(); ++it) {
        delete (*it).second;
    }
}

void RocksDBStore::check_integrity() {

    if (!bfs::exists(compose_sensors_path())) {
        std::ostringstream oss;
        oss << "The database path is incomplete.";
        throw StoreException(oss.str());
    }
}

void RocksDBStore::initialize() {

    bfs::remove_all(_path);
    create_directory(_path.string());
    create_directory(compose_sensors_path());
}

void RocksDBStore::dispose() {

    clear_buffers();
    bfs::remove_all(_path);
}

const std::string RocksDBStore::str() {

    std::ostringstream oss;
    oss << "RocksDB database, stored in path " << _path.string();
    return oss.str();
};

void RocksDBStore::add_sensor(const Sensor::Ptr sensor) {

    const std::string uuid = sensor->uuid_string();
    create_directory(compose_sensor_path(uuid));
    create_directory(compose_sensor_properties_path(uuid));
    create_directory(compose_sensor_readings_path(uuid));

    put_sensor(true, sensor);
}

void RocksDBStore::remove_sensor(const Sensor::Ptr sensor) {

    remove_db(compose_sensor_properties_path(sensor->uuid_string()));
    remove_db(compose_sensor_readings_path(sensor->uuid_string()));
    bfs::remove_all(compose_sensor_path(sensor->uuid_string()));
}

void RocksDBStore::update_sensor(const Sensor::Ptr sensor) {

    put_sensor(false, sensor);
}

void RocksDBStore::put_sensor(const bool create, const Sensor::Ptr sensor) {

    rocksdb::DB* db = open_db(create, create,
            compose_sensor_properties_path(sensor->uuid_string()));

    put_value(db, "external_id", sensor->external_id());
    put_value(db, "name", sensor->name());
    put_value(db, "description", sensor->description());
    put_value(db, "unit", sensor->unit());
    put_value(db, "timezone", sensor->timezone());
}

Sensor::Ptr RocksDBStore::get_sensor(const Sensor::uuid_t& uuid) {

    const std::string uuid_str = boost::uuids::to_string(uuid);
    const std::string db_path = compose_sensor_properties_path(uuid_str);
    rocksdb::DB* db = _db_buffer[db_path];

    if (db) {
        SensorFactory::Ptr factory(new SensorFactory());
        return factory->createSensor(uuid,
                get_value(db, "external_id"),
                get_value(db, "name"),
                get_value(db, "description"),
                get_value(db, "unit"),
                get_value(db, "timezone"));

    } else {
        std::ostringstream err;
        err << "Sensor " << uuid_str << " could not be found.";
        throw StoreException(err.str());
    }
}

std::vector<Sensor::Ptr> RocksDBStore::get_sensors_by_external_id(const std::string& external_id) {

    std::vector<Sensor::Ptr> found;
    std::vector<Sensor::Ptr> sensors = get_sensors();

    for (std::vector<Sensor::Ptr>::iterator sensor = sensors.begin(); sensor != sensors.end(); sensor++) {

        if ((*sensor)->external_id() == external_id) {
            found.push_back(*sensor);
        }
    }
    return found;
}

std::vector<Sensor::Ptr> RocksDBStore::get_sensors_by_name(const std::string& name) {

    std::vector<Sensor::Ptr> found;
    std::vector<Sensor::Ptr> sensors = get_sensors();

    for (std::vector<Sensor::Ptr>::iterator sensor = sensors.begin(); sensor != sensors.end(); sensor++) {

        if ((*sensor)->name() == name) {
            found.push_back(*sensor);
        }
    }
    return found;
}

std::vector<Sensor::uuid_t> RocksDBStore::get_sensor_uuids() {

    std::vector<Sensor::uuid_t> uuids;
    bfs::directory_iterator end;

    for (bfs::directory_iterator it(compose_sensors_path()); it != end; it++) {

        boost::uuids::uuid u;
        std::stringstream ss;
        ss << it->path().filename().string();
        ss >> u;

        uuids.push_back(u);
    }
    return uuids;
}

std::vector<Sensor::Ptr> RocksDBStore::get_sensors() {

    std::vector<Sensor::Ptr> sensors;
    std::vector<Sensor::uuid_t> uuids = get_sensor_uuids();

    for (std::vector<Sensor::uuid_t>::iterator uuid = uuids.begin(); uuid != uuids.end(); uuid++) {

        sensors.push_back(get_sensor(*uuid));
    }
    return sensors;
}

void RocksDBStore::add_reading(const Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    put_value(db, std::to_string(timestamp), std::to_string(value));
}

void RocksDBStore::add_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        add_reading(sensor, (*it).first, (*it).second);
    }
}

void RocksDBStore::update_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    add_readings(sensor, readings);
}

readings_t_Ptr RocksDBStore::get_all_readings(const Sensor::Ptr sensor) {

    readings_t_Ptr readings(new readings_t());

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next()) {

        std::string epoch = it->key().ToString();
        std::string value = it->value().ToString();

        readings->insert(
                std::pair<timestamp_t, double>(
                time_converter->convert_from_epoch(atol(epoch.c_str())),
                atof(value.c_str())
                ));
    }
    delete it;
    return readings;
}

readings_t_Ptr RocksDBStore::get_timeframe_readings(const Sensor::Ptr sensor, timestamp_t begin, timestamp_t end) {

    readings_t_Ptr readings(new readings_t());

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next()) {

        std::string epoch = it->key().ToString();
        std::string value = it->value().ToString();
        long timestamp = time_converter->convert_from_epoch(atol(epoch.c_str()));

        if (timestamp >= begin && timestamp <= end) {

            readings->insert(
                    std::pair<timestamp_t, double>(
                    timestamp,
                    atof(value.c_str())
                    ));
        }
    }
    delete it;
    return readings;
}

unsigned long int RocksDBStore::get_num_readings(const Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    return get_all_readings(sensor)->size();
}

reading_t RocksDBStore::get_last_reading(const Sensor::Ptr sensor) {

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    it->SeekToLast();

    const char* epoch = it->key().ToString().c_str();
    const char* value = it->value().ToString().c_str();

    return std::pair<timestamp_t, double>(time_converter->convert_from_epoch(atol(epoch)), atof(value));
}

reading_t RocksDBStore::get_reading(const Sensor::Ptr sensor, timestamp_t timestamp) {

    //FIXME: make this method more efficient
    klio::readings_t_Ptr readings = get_all_readings(sensor);

    std::pair<timestamp_t, double> reading = std::pair<timestamp_t, double>(0, 0);
    
    if (readings->count(timestamp)) {
        reading.first = timestamp;
        reading.second = readings->at(timestamp);
    }
    return reading;
}

void RocksDBStore::clear_buffers() {

    Store::clear_buffers();
    _db_buffer.clear();
}

rocksdb::DB* RocksDBStore::open_db(const bool create_if_missing, const bool error_if_exists, const std::string& db_path) {

    if (!_db_buffer[db_path]) {

        rocksdb::DB* db;
        rocksdb::Options options;
        options.create_if_missing = create_if_missing;
        options.error_if_exists = error_if_exists;
        options.paranoid_checks = _db_options["paranoid_checks"] == "true";
        options.disableDataSync = _db_options["disableDataSync"] == "true";

        rocksdb::Status status = rocksdb::DB::Open(options, db_path, &db);

        if (!status.ok()) {
            throw StoreException(status.ToString());
        }
        _db_buffer[db_path] = db;
    }
    return _db_buffer[db_path];
}

void RocksDBStore::close_db(const std::string& db_path) {

    rocksdb::DB* db = _db_buffer[db_path];

    if (db) {
        _db_buffer.erase(db_path);
        delete db;
    }
}

void RocksDBStore::remove_db(const std::string& db_path) {

    close_db(db_path);
    bfs::remove_all(db_path);
}

void RocksDBStore::put_value(rocksdb::DB* db, const std::string& key, const std::string& value) {

    rocksdb::WriteOptions options;
    options.sync = _write_options["sync"] == "true";
    options.disableWAL = _write_options["disableWAL"] == "true";

    rocksdb::Status status = db->Put(options, key, value);

    if (!status.ok()) {
        throw StoreException(status.ToString());
    }
}

std::string RocksDBStore::get_value(rocksdb::DB* db, const std::string& key) {

    rocksdb::ReadOptions options;

    std::string value;
    rocksdb::Status status = db->Get(options, key, &value);

    if (status.ok()) {
        return value;

    } else {
        throw StoreException(status.ToString());
    }
}

void RocksDBStore::delete_value(rocksdb::DB* db, const std::string& key) {

    rocksdb::Status status = db->Delete(rocksdb::WriteOptions(), key);

    if (!status.ok()) {
        throw StoreException(status.ToString());
    }
}

const std::string RocksDBStore::compose_sensors_path() {

    std::ostringstream str;
    str << _path.string() << "/sensors";
    return str.str();
}

const std::string RocksDBStore::compose_sensor_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensors_path() << "/" << uuid;
    return str.str();
}

const std::string RocksDBStore::compose_sensor_properties_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensor_path(uuid) << "/properties";
    return str.str();
}

const std::string RocksDBStore::compose_sensor_readings_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensor_path(uuid) << "/readings";
    return str.str();
}

void RocksDBStore::create_directory(const std::string& dir) {

    if (!bfs::create_directory(dir)) {

        std::ostringstream str;
        str << "RocksDB database directory " << dir << " can not be created.";
        throw StoreException(str.str());
    }
}

#endif /* ENABLE_ROCKSDB */