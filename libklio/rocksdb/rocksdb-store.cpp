#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/sensor-factory.hpp>
#include "rocksdb-store.hpp"


using namespace klio;

void RocksDBStore::open() {
}

void RocksDBStore::close() {
}

void RocksDBStore::initialize() {
}

void RocksDBStore::dispose() {
    close();
    bfs::remove(_path);
}

const std::string RocksDBStore::str() {

    std::ostringstream oss;
    oss << "RocksDB database, stored in file " << _path;
    return oss.str();
};

void RocksDBStore::add_sensor(klio::Sensor::Ptr sensor) {
    
    LOG("Adding sensor: " << sensor->str());
}

void RocksDBStore::remove_sensor(const klio::Sensor::Ptr sensor) {
    
    LOG("Removing sensor: " << sensor->str());
}

void RocksDBStore::update_sensor(klio::Sensor::Ptr sensor) {
    
    LOG("Updating sensor: " << sensor->str());
}

klio::Sensor::Ptr RocksDBStore::get_sensor(const klio::Sensor::uuid_t& uuid) {

    LOG("Attempting to load sensor " << uuid);

    throw StoreException("");
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_external_id(const std::string& external_id) {

    LOG("Attempting to load sensors " << external_id);

    std::vector<klio::Sensor::Ptr> sensors;
    
    return sensors;
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_name(const std::string& name) {

    LOG("Attempting to load sensors " << name);
    
    throw StoreException("");
}

std::vector<klio::Sensor::uuid_t> RocksDBStore::get_sensor_uuids() {

    LOG("Retrieving UUIDs from store.");

    std::vector<klio::Sensor::uuid_t> uuids;
    
    return uuids;
}

void RocksDBStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {
    
    LOG("Attempting to add reading of sensor " << sensor->str());
}

void RocksDBStore::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
    
    LOG("Attempting to add readings of sensor " << sensor->str());;
}

void RocksDBStore::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
    
    LOG("Attempting to update readings of sensor " << sensor->str());
}

readings_t_Ptr RocksDBStore::get_all_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving all readings of sensor " << sensor->str());

    readings_t_Ptr readings(new readings_t());
    
    return readings;
}

unsigned long int RocksDBStore::get_num_readings(klio::Sensor::Ptr sensor) {

    LOG("Retrieving number of readings for sensor " << sensor->str());

    long int num = 0;

    return num;
}

std::pair<timestamp_t, double> RocksDBStore::get_last_reading(klio::Sensor::Ptr sensor) {

    LOG("Retrieving last readings of sensor " << sensor->str());

    std::pair<timestamp_t, double> reading;

    return reading;
}
