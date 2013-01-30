#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <curl/curl.h>

#include "msgstore.hpp"


using namespace klio;


void MSGStore::open () {
}

void MSGStore::initialize() {
}

void MSGStore::close () {
}

const std::string MSGStore::str() {

  std::ostringstream str;
  str << "MSG store: " << _url;
  return str.str();
};

void MSGStore::addSensor(klio::Sensor::Ptr sensor) {
}

void MSGStore::removeSensor(const klio::Sensor::Ptr sensor) {
}

std::vector<klio::Sensor::uuid_t> MSGStore::getSensorUUIDs() {
  
  std::vector<klio::Sensor::uuid_t> uuids;
  return uuids;  
}

std::vector<klio::Sensor::Ptr> MSGStore::getSensorById(const std::string& sensor_id) {

  std::vector<klio::Sensor::Ptr> retval;
  return retval;
}

klio::Sensor::Ptr MSGStore::getSensor(const klio::Sensor::uuid_t& uuid) {

  klio::Sensor::Ptr retval;
  return retval;
}

void MSGStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {
}

void MSGStore::add_description(klio::Sensor::Ptr sensor, const std::string& desc) {
}

void MSGStore::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
}

void MSGStore::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {
}

readings_t_Ptr MSGStore::get_all_readings(klio::Sensor::Ptr sensor) {

  std::string sensor_url = "";
  sensor_url.append(_url);
  sensor_url.append("/sensor/");
  sensor_url.append(sensor->uuid_string());
  sensor_url.append("?interval=hour&unit=watt");

  readings_t_Ptr retval(new readings_t());
  return retval;
}

std::pair<timestamp_t, double> MSGStore::get_last_reading(klio::Sensor::Ptr sensor) {

  std::pair<timestamp_t, double> retval;
  return retval;
}

unsigned long int MSGStore::get_num_readings(klio::Sensor::Ptr sensor) {

  return 0;
}

void MSGStore::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {
  
  sensor = getSensor(sensor->uuid());
    
  klio::readings_t_Ptr readings = store->get_all_readings(sensor);

  update_readings(sensor, *readings);
}
