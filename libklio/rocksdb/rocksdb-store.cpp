#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include "rocksdb-store.hpp"


using namespace klio;

void RocksDBStore::open() {

    if (bfs::exists(_path) && bfs::is_directory(_path) && _buffer.empty()) {

        std::vector<klio::Sensor::uuid_t> uuids = get_sensor_uuids();

        for (std::vector<klio::Sensor::uuid_t>::iterator uuid = uuids.begin(); uuid != uuids.end(); uuid++) {

            const std::string uuid_str = boost::uuids::to_string(*uuid);
            open_db(false, false, compose_sensor_properties_path(uuid_str));
            open_db(true, false, compose_sensor_readings_path(uuid_str));
        }
    }
}

void RocksDBStore::close() {

    for (std::map<std::string, rocksdb::DB*>::const_iterator it = _buffer.begin(); it != _buffer.end(); ++it) {
        delete (*it).second;
    }
    _buffer.clear();
}

void RocksDBStore::initialize() {

    bfs::remove_all(_path);
    create_directory(_path.string());
    create_directory(compose_sensors_path());
}

void RocksDBStore::dispose() {

    close();
    bfs::remove_all(_path);
}

const std::string RocksDBStore::str() {

    std::ostringstream oss;
    oss << "RocksDB database, stored in path " << _path.string();
    return oss.str();
};

void RocksDBStore::add_sensor(klio::Sensor::Ptr sensor) {

    const std::string uuid = sensor->uuid_string();
    create_directory(compose_sensor_path(uuid));
    create_directory(compose_sensor_properties_path(uuid));
    create_directory(compose_sensor_readings_path(uuid));

    put_sensor(true, sensor);
}

void RocksDBStore::remove_sensor(const klio::Sensor::Ptr sensor) {

    remove_db(compose_sensor_properties_path(sensor->uuid_string()));
    remove_db(compose_sensor_readings_path(sensor->uuid_string()));
    bfs::remove_all(compose_sensor_path(sensor->uuid_string()));
}

void RocksDBStore::update_sensor(klio::Sensor::Ptr sensor) {

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

klio::Sensor::Ptr RocksDBStore::get_sensor(const klio::Sensor::uuid_t& uuid) {

    const std::string uuid_str = boost::uuids::to_string(uuid);
    const std::string db_path = compose_sensor_properties_path(uuid_str);
    rocksdb::DB* db = _buffer[db_path];

    if (db) {
        klio::SensorFactory::Ptr factory(new klio::SensorFactory());
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

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_external_id(const std::string& external_id) {

    std::vector<klio::Sensor::Ptr> found;
    std::vector<klio::Sensor::Ptr> sensors = get_sensors();

    for (std::vector<klio::Sensor::Ptr>::iterator sensor = sensors.begin(); sensor != sensors.end(); sensor++) {

        if ((*sensor)->external_id() == external_id) {
            found.push_back(*sensor);
        }
    }
    return found;
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_name(const std::string& name) {

    std::vector<klio::Sensor::Ptr> found;
    std::vector<klio::Sensor::Ptr> sensors = get_sensors();

    for (std::vector<klio::Sensor::Ptr>::iterator sensor = sensors.begin(); sensor != sensors.end(); sensor++) {

        if ((*sensor)->name() == name) {
            found.push_back(*sensor);
        }
    }
    return found;
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors() {

    std::vector<klio::Sensor::Ptr> sensors;
    std::vector<klio::Sensor::uuid_t> uuids = get_sensor_uuids();

    for (std::vector<klio::Sensor::uuid_t>::iterator uuid = uuids.begin(); uuid != uuids.end(); uuid++) {

        sensors.push_back(get_sensor(*uuid));
    }
    return sensors;
}

std::vector<klio::Sensor::uuid_t> RocksDBStore::get_sensor_uuids() {

    std::vector<klio::Sensor::uuid_t> uuids;
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

void RocksDBStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    put_value(db, std::to_string(timestamp), std::to_string(value));
}

void RocksDBStore::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        add_reading(sensor, (*it).first, (*it).second);
    }
}

void RocksDBStore::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    add_readings(sensor, readings);
}

readings_t_Ptr RocksDBStore::get_all_readings(klio::Sensor::Ptr sensor) {

    readings_t_Ptr readings(new readings_t());
    klio::TimeConverter::Ptr tc(new klio::TimeConverter());

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next()) {

        std::string epoch = it->key().ToString();
        std::string value = it->value().ToString();

        readings->insert(
                std::pair<timestamp_t, double>(
                tc->convert_from_epoch(atol(epoch.c_str())),
                atof(value.c_str())
                ));
    }
    delete it;
    return readings;
}

unsigned long int RocksDBStore::get_num_readings(klio::Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    return get_all_readings(sensor)->size();
}

reading_t RocksDBStore::get_last_reading(klio::Sensor::Ptr sensor) {

    rocksdb::DB* db = open_db(true, false,
            compose_sensor_readings_path(sensor->uuid_string()));

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    it->SeekToLast();

    const char* epoch = it->key().ToString().c_str();
    const char* value = it->value().ToString().c_str();

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    return std::pair<timestamp_t, double>(tc->convert_from_epoch(atol(epoch)), atof(value));
}

rocksdb::DB* RocksDBStore::open_db(const bool create_if_missing, const bool error_if_exists, const std::string& db_path) {

    if (!_buffer[db_path]) {

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
        _buffer[db_path] = db;
    }
    return _buffer[db_path];
}

void RocksDBStore::close_db(const std::string& db_path) {

    rocksdb::DB* db = _buffer[db_path];

    if (db) {
        _buffer.erase(db_path);
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