#include <iostream>
#include <sstream>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <curl/curl.h>
#include <json/json.h>
#include <libklio/sensor-factory.hpp>
#include "msg-store.hpp"


using namespace boost::uuids;
using namespace klio;

void MSGStore::open() {
}

void MSGStore::initialize() {

    json_object *jobject = json_object_new_object();
    json_object *jkey = json_object_new_string(_key.c_str());
    json_object *jdescription = json_object_new_string("libklio MSG Store");
    json_object_object_add(jobject, "key", jkey);
    json_object_object_add(jobject, "description", jdescription);

    std::string url = compose_url("device", _id);
    perform_http_post(url, _key, jobject);
}

void MSGStore::close() {
}

void MSGStore::dispose() {

    std::string url = compose_url("device", _id);
    perform_http_delete(url, _key);
}

const std::string MSGStore::str() {

    std::ostringstream str;
    str << "MSG store " << compose_url("device", _id);
    return str.str();
};

void MSGStore::add_sensor(klio::Sensor::Ptr sensor) {

    update_sensor(sensor);
}

void MSGStore::remove_sensor(const klio::Sensor::Ptr sensor) {

    std::string url = compose_url("sensor", sensor->uuid_short());
    perform_http_delete(url, _key);
}

void MSGStore::update_sensor(const klio::Sensor::Ptr sensor) {

    json_object *jdevice = json_object_new_string(_id.c_str());
    json_object *jname = json_object_new_string(sensor->name().c_str());
    json_object *jdescription = json_object_new_string(sensor->description().c_str());
    json_object *junit = json_object_new_string(sensor->unit().c_str());

    json_object *jconfig = json_object_new_object();
    json_object_object_add(jconfig, "device", jdevice);
    json_object_object_add(jconfig, "function", jname);
    json_object_object_add(jconfig, "description", jdescription);
    json_object_object_add(jconfig, "unit", junit);

    json_object *jobject = json_object_new_object();
    json_object_object_add(jobject, "config", jconfig);

    std::string url = compose_url("sensor", sensor->uuid_short());
    perform_http_post(url, _key, jobject);
}

klio::Sensor::Ptr MSGStore::get_sensor(const klio::Sensor::uuid_t& uuid) {

    std::string uuid_str = to_string(uuid);
    json_object *jsensors = get_json_sensors();

    for (int i = 0; i < json_object_array_length(jsensors); i++) {

        json_object *jsensor = json_object_array_get_idx(jsensors, i);
        json_object *jmeter = json_object_object_get(jsensor, "meter");
        const char* meter = json_object_get_string(jmeter);

        if (uuid_str == format_uuid_string(meter)) {

            return create_sensor(uuid_str, jsensor);
        }
    }
    std::ostringstream err;
    err << "Sensor " << to_string(uuid) << " could not be found.";
    throw StoreException(err.str());
}

std::vector<klio::Sensor::Ptr> MSGStore::get_sensors_by_name(const std::string& name) {

    std::vector<klio::Sensor::Ptr> sensors;
    json_object *jsensors = get_json_sensors();

    for (int i = 0; i < json_object_array_length(jsensors); i++) {

        json_object *jsensor = json_object_array_get_idx(jsensors, i);
        json_object *jfunction = json_object_object_get(jsensor, "function");
        const char* function = json_object_get_string(jfunction);

        if (name == function) {

            json_object *jmeter = json_object_object_get(jsensor, "meter");
            std::string meter = std::string(json_object_get_string(jmeter));

            sensors.push_back(create_sensor(
                    format_uuid_string(meter),
                    jsensor));
        }
    }
    return sensors;
}

std::vector<klio::Sensor::uuid_t> MSGStore::get_sensor_uuids() {

    std::vector<klio::Sensor::uuid_t> uuids;

    json_object *jsensors = get_json_sensors();

    for (int i = 0; i < json_object_array_length(jsensors); i++) {

        json_object *jsensor = json_object_array_get_idx(jsensors, i);
        json_object *jmeter = json_object_object_get(jsensor, "meter");
        std::string meter = std::string(json_object_get_string(jmeter));

        boost::uuids::uuid u;
        std::stringstream ss;
        ss << format_uuid_string(meter);
        ss >> u;

        uuids.push_back(u);
    }
    return uuids;
}

void MSGStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    klio::reading_t reading(timestamp, value);
    readings_t readings;
    readings.insert(reading);

    add_readings(sensor, readings);
}

void MSGStore::add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    json_object *jtuples = json_object_new_array();

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        klio::timestamp_t timestamp = (*it).first;
        long value = (*it).second;

        struct json_object *jtuple = json_object_new_array();
        json_object_array_add(jtuple, json_object_new_int(timestamp));
        json_object_array_add(jtuple, json_object_new_int(value));

        json_object_array_add(jtuples, jtuple);
    }

    json_object *jobject = json_object_new_object();
    json_object_object_add(jobject, "measurements", jtuples);

    std::string url = compose_url("sensor", sensor->uuid_short());
    perform_http_post(url, _key, jobject);
}

void MSGStore::update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) {

    add_readings(sensor, readings);
}

readings_t_Ptr MSGStore::get_all_readings(klio::Sensor::Ptr sensor) {

    struct json_object *jobject = get_json_readings(sensor);
    int length = json_object_array_length(jobject);

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    readings_t_Ptr readings(new readings_t());

    for (int i = 0; i < length; i++) {

        json_object *jpair = json_object_array_get_idx(jobject, i);
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        json_object *jvalue = json_object_array_get_idx(jpair, 1);

        long epoch = json_object_get_int(jtimestamp);
        timestamp_t timestamp = tc->convert_from_epoch(epoch);
        double value = json_object_get_double(jvalue);

        if (!isnan(value)) {
            readings->insert(std::pair<timestamp_t, double>(
                    timestamp,
                    value
                    ));
        }
    }
    return readings;
}

unsigned long int MSGStore::get_num_readings(klio::Sensor::Ptr sensor) {

    json_object *jreadings = get_json_readings(sensor);
    return json_object_array_length(jreadings);
}

std::pair<timestamp_t, double> MSGStore::get_last_reading(klio::Sensor::Ptr sensor) {

    json_object *jreadings = get_json_readings(sensor);
    int i = json_object_array_length(jreadings) - 1;

    if (i >= 0) {
        json_object *jpair = json_object_array_get_idx(jreadings, i);
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        json_object *jvalue = json_object_array_get_idx(jpair, 1);

        long epoch = json_object_get_int(jtimestamp);
        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        timestamp_t timestamp = tc->convert_from_epoch(epoch);
        double value = json_object_get_double(jvalue);

        if (!isnan(value)) {
            return std::pair<timestamp_t, double>(timestamp, value);
        }
    }
    std::ostringstream err;
    err << "No reading found for sensor " << sensor->uuid_short() << ".";
    throw StoreException(err.str());
}

struct json_object *MSGStore::get_json_sensors() {

    std::string url = compose_url("device", _id);
    struct json_object *jobject = perform_http_get(url, _key);

    return json_object_object_get(jobject, "sensors");
}

struct json_object *MSGStore::get_json_readings(klio::Sensor::Ptr sensor) {

    std::ostringstream id;
    id << sensor->uuid_short() << "?interval=hour&unit=" << sensor->unit();

    std::string url = compose_url("sensor", id.str());
    return perform_http_get(url, _key);
}

klio::Sensor::Ptr MSGStore::create_sensor(std::string uuid_str, json_object *jsensor) {

    json_object *jdescription = json_object_object_get(jsensor, "description");
    const char* description = json_object_get_string(jdescription);

    json_object *jname = json_object_object_get(jsensor, "function");
    const char* name = json_object_get_string(jname);

    json_object *junit = json_object_object_get(jsensor, "unit");
    const char* unit = json_object_get_string(junit);

    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    return sensor_factory->createSensor(
            uuid_str,
            std::string(name),
            std::string(description),
            std::string(unit),
            std::string("Europe/Berlin"));
}

std::string MSGStore::format_uuid_string(std::string meter) {

    std::stringstream oss;
    for (size_t i = 0; i < meter.length(); i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            oss << '-';
        }
        oss << meter[i];
    }
    return oss.str();
}

std::string MSGStore::compose_url(std::string object, std::string id) {

    std::ostringstream oss;
    oss << _url << "/" << object << "/" << id;
    return oss.str();
}

struct json_object *MSGStore::perform_http_get(std::string url, std::string key) {

    curl_slist *headers = create_curl_headers();

    std::ostringstream header;
    header << "X-Digest: " << digest_message("", key);
    headers = curl_slist_append(headers, header.str().c_str());

    CURL *curl = create_curl_handler(url, headers);

    return perform_http_request(curl);
}

struct json_object *MSGStore::perform_http_post(std::string url, std::string key, json_object *jobject) {

    const char* body = json_object_to_json_string(jobject);

    curl_slist *headers = create_curl_headers();
    headers = curl_slist_append(headers, "Content-type: application/json");

    std::ostringstream header1;
    header1 << "Content-Length: " << strlen(body);
    headers = curl_slist_append(headers, header1.str().c_str());

    std::ostringstream header2;
    header2 << "X-Digest: " << digest_message(body, key);
    headers = curl_slist_append(headers, header2.str().c_str());

    CURL *curl = create_curl_handler(url, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    return perform_http_request(curl);
}

void *MSGStore::perform_http_delete(std::string url, std::string key) {

    curl_slist *headers = create_curl_headers();

    std::ostringstream header;
    header << "X-Digest: " << digest_message("", key);
    headers = curl_slist_append(headers, header.str().c_str());

    CURL *curl = create_curl_handler(url, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    return perform_http_request(curl);
}

curl_slist *MSGStore::create_curl_headers() {

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: libklio");
    headers = curl_slist_append(headers, "X-Version: 1.0");
    headers = curl_slist_append(headers, "Accept: application/json,text/html");

    return headers;
}

CURL *MSGStore::create_curl_handler(std::string url, curl_slist *headers) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // signal-handling in libklio is NOT thread-safe.
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        //Required if next router has an ip-change.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

        return curl;

    } else {
        std::ostringstream oss;
        oss << "CURL could not be initiated.";
        throw StoreException(oss.str());
    }
}

std::string MSGStore::digest_message(std::string data, std::string key) {

    HMAC_CTX hmacContext;
    HMAC_Init(&hmacContext, key.c_str(), key.length(), EVP_sha1());
    HMAC_Update(&hmacContext, (const unsigned char*) data.c_str(), data.length());

    unsigned char out[EVP_MAX_MD_SIZE];
    unsigned int len = EVP_MAX_MD_SIZE;

    HMAC_Final(&hmacContext, out, &len);
    char ret[2 * EVP_MAX_MD_SIZE];
    memset(ret, 0, sizeof (ret));

    for (size_t i = 0; i < len; i++) {
        char s[4];
        snprintf(s, 3, "%02x:", out[i]);
        strncat(ret, s, 2 * len);
    }

    char digest[255];
    snprintf(digest, 255, "%s", ret);
    return std::string(digest);
}

static size_t curl_write_custom_callback(void *ptr, size_t size, size_t nmemb, void *data) {

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

struct json_object *MSGStore::perform_http_request(CURL *curl) {

    CURLresponse curl_response;
    curl_response.data = NULL;
    curl_response.size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &curl_response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_custom_callback);

    CURLcode curl_code = curl_easy_perform(curl);

    long int http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (curl_code != CURLE_OK) {
        std::ostringstream oss;
        oss << "HTTPS request failed. " << curl_easy_strerror(curl_code);
        throw StoreException(oss.str());

    } else if (http_code != 200) {
        std::ostringstream oss;
        oss << "HTTPS request failed. HTTPS code: " << http_code;
        throw StoreException(oss.str());
    }

    std::string response = std::string(curl_response.data);
    free(curl_response.data);

    return json_tokener_parse(response.c_str());
}
