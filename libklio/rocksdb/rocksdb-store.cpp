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
}

const std::string RocksDBStore::str() {

    std::ostringstream oss;
    oss << "RocksDB database, stored in file " << _path;
    return oss.str();
};

void RocksDBStore::add_sensor(klio::Sensor::Ptr sensor) {
}

void RocksDBStore::remove_sensor(const klio::Sensor::Ptr sensor) {
}

void RocksDBStore::update_sensor(klio::Sensor::Ptr sensor) {
}

klio::Sensor::Ptr RocksDBStore::get_sensor(const klio::Sensor::uuid_t& uuid) {

    throw StoreException("");
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_external_id(const std::string& external_id) {

    throw StoreException("");
}

std::vector<klio::Sensor::Ptr> RocksDBStore::get_sensors_by_name(const std::string& name) {

    throw StoreException("");
}

std::vector<klio::Sensor::uuid_t> RocksDBStore::get_sensor_uuids() {

    throw StoreException("");
}

void RocksDBStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {
}

void RocksDBStore::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
}

void RocksDBStore::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
}

readings_t_Ptr RocksDBStore::get_all_readings(klio::Sensor::Ptr sensor) {

    throw StoreException("");
}

unsigned long int RocksDBStore::get_num_readings(klio::Sensor::Ptr sensor) {

    throw StoreException("");
}

std::pair<timestamp_t, double> RocksDBStore::get_last_reading(klio::Sensor::Ptr sensor) {

    throw StoreException("");
}
