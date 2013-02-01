#include <iostream>
#include <sstream>
#include <cstdio>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json/json.h>

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

    std::string url = "";
    url.append(_url);
    url.append("/sensor/");
    url.append(sensor->uuid_short().c_str());
    url.append("?interval=hour&unit=watt");

    std::string response = perform_http_get_sensor(url, sensor->token());
    struct json_object *parsed = json_tokener_parse(response.c_str());

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    readings_t_Ptr readings(new readings_t());
    int length = json_object_array_length(parsed);

    for (int i = 0; i < length; i++) {

        json_object *jpair = json_object_array_get_idx(parsed, i);
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        long time = json_object_get_int(jtimestamp);
        json_object *jvalue = json_object_array_get_idx(jpair, 1);

        readings->insert(
                std::pair<timestamp_t, double>(
                tc->convert_from_epoch(time),
                json_object_get_double(jvalue)
                ));
    }
    return readings;
}

std::string MSGStore::perform_http_get_sensor(std::string url, std::string token) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        std::string token_header = "";
        token_header.append("X-Token: ");
        token_header.append(token);

        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "User-Agent: libklio");
        headers = curl_slist_append(headers, "X-Version: 1.0");
        headers = curl_slist_append(headers, token_header.c_str());
        headers = curl_slist_append(headers, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLresponse response;
        response.data = NULL;
        response.size = 0;

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_custom_callback);

        // signal-handling in libcurl is NOT thread-safe.
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        //Required if next router has an ip-change.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

        CURLcode curl_code = curl_easy_perform(curl);
        
        long int http_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (curl_code != CURLE_OK || http_code != 200) {
            std::ostringstream oss;
            oss << "HTTPS request failed. " << curl_easy_strerror(curl_code) << " HTTP code: " << http_code;
            throw StoreException(oss.str());
        }

        std::string data = std::string(response.data);
        free(response.data);

        return data;
        
    } else {
        std::ostringstream oss;
        oss << "CURL could not be initiated.";
        throw StoreException(oss.str());
    }
}

size_t klio::curl_write_custom_callback(void *ptr, size_t size, size_t nmemb, void *data) {

    size_t realsize = size * nmemb;
    CURLresponse *response = static_cast<CURLresponse *> (data);

    response->data = (char *) realloc(response->data, response->size + realsize + 1);
    if (response->data == NULL) { // out of memory!
        LOG("Cannot allocate memory");
        exit(EXIT_FAILURE);
    }

    memcpy(&(response->data[response->size]), ptr, realsize);
    response->size += realsize;
    response->data[response->size] = 0;

    return realsize;
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
