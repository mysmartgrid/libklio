#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>

#include "msgstore.hpp"


using namespace klio;

void MSGStore::open() {
}

void MSGStore::initialize() {
}

void MSGStore::close() {
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

    readings_t_Ptr readings(new readings_t());

    //TODO: Complete method
    std::string response = perform_http_get_sensor(sensor_url, "2dd8605907fa2c9d4ef8bb831d21030e");

    //TODO: remove this test block
    std::ostringstream oss;
    oss << "get_all_readings response: " << response << "\n";
    LOG(oss);

    return readings;
}

std::string MSGStore::perform_http_get_sensor(std::string sensor_url, std::string sensor_token) {

    CURL *curl;
    CURLcode res = CURLE_OK;
    std::string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, sensor_url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        std::string token_header = "";
        token_header.append("X-Token: ");
        token_header.append(sensor_token);

        curl_slist *headers = NULL;
        curl_slist_append(headers, token_header.c_str());
        curl_slist_append(headers, "X-Version: 1.0");
        curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::ostringstream oss;
            oss << "HTTPS connection failed: " << curl_easy_strerror(res) << "\n";
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw StoreException(oss.str());
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return response;
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
