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


using namespace klio;

void MSGStore::open() {
}

void MSGStore::initialize() {

    json_object *jkey = json_object_new_string(_key.c_str());
    json_object *jdescription = json_object_new_string(_description.c_str());
    json_object *jtype = json_object_new_string(_type.c_str());

    json_object *jobject = json_object_new_object();
    json_object_object_add(jobject, "key", jkey);
    json_object_object_add(jobject, "description", jdescription);
    json_object_object_add(jobject, "type", jtype);

    std::string url = compose_device_url();
    json_object *jresponse = perform_http_post(url, _key, jobject);

    std::vector<Sensor::Ptr> sensors = get_sensors();
    for (std::vector<Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {
        init_buffers(*sensor);
    }

    json_object_put(jobject);
    json_object_put(jresponse);
}

void MSGStore::close() {
    flush(true);
}

void MSGStore::dispose() {

    std::string url = compose_device_url();
    perform_http_delete(url, _key);
    clear_buffers();
}

void MSGStore::flush() {
    heartbeat();
    flush(true);
}

const std::string MSGStore::str() {

    std::ostringstream str;
    str << "mSG store " << compose_device_url();
    return str.str();
};

void MSGStore::add_sensor(const Sensor::Ptr sensor) {

    update_sensor(sensor);
    init_buffers(sensor);
}

void MSGStore::remove_sensor(const Sensor::Ptr sensor) {

    std::string url = compose_sensor_url(sensor);
    perform_http_delete(url, _key);
    clear_buffers(sensor);
}

void MSGStore::update_sensor(const Sensor::Ptr sensor) {

    json_object *jdevice = json_object_new_string(_id.c_str());
    json_object *jexternal_id = json_object_new_string(sensor->external_id().c_str());
    json_object *jname = json_object_new_string(sensor->name().c_str());
    json_object *jdescription = json_object_new_string(sensor->description().c_str());
    json_object *junit = json_object_new_string(sensor->unit().c_str());

    json_object *jconfig = json_object_new_object();
    json_object_object_add(jconfig, "device", jdevice);
    json_object_object_add(jconfig, "externalid", jexternal_id);
    json_object_object_add(jconfig, "function", jname);
    json_object_object_add(jconfig, "description", jdescription);
    json_object_object_add(jconfig, "unit", junit);

    json_object *jobject = json_object_new_object();
    json_object_object_add(jobject, "config", jconfig);

    std::string url = compose_sensor_url(sensor);
    json_object *jresponse = perform_http_post(url, _key, jobject);
    _sensors_buffer[sensor->uuid()] = sensor;

    json_object_put(jobject);
    json_object_put(jresponse);
}

Sensor::Ptr MSGStore::get_sensor(const Sensor::uuid_t& uuid) {

    Sensor::Ptr sensor = _sensors_buffer[uuid];
    if (sensor) {
        return sensor;

    } else {
        std::ostringstream err;
        err << "Sensor " << boost::uuids::to_string(uuid) << " could not be found.";
        throw StoreException(err.str());
    }
}

Sensor::Ptr MSGStore::get_sensor_by_external_id(const std::string& external_id) {

    for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

        Sensor::Ptr sensor = (*it).second;

        if (external_id == sensor->external_id()) {
            return sensor;
        }
    }
    std::ostringstream err;
    err << "Sensor " << external_id << " could not be found.";
    throw StoreException(err.str());
}

std::vector<Sensor::Ptr> MSGStore::get_sensors_by_name(const std::string& name) {

    std::vector<Sensor::Ptr> sensors;

    for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

        Sensor::Ptr sensor = (*it).second;

        if (name == sensor->name()) {
            sensors.push_back(sensor);
        }
    }
    return sensors;
}

std::vector<Sensor::uuid_t> MSGStore::get_sensor_uuids() {

    std::vector<Sensor::uuid_t> uuids;

    for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {
        uuids.push_back((*it).first);
    }
    return uuids;
}

void MSGStore::add_reading(const Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    init_buffers(sensor);
    _readings_buffer[sensor->uuid()]->insert(reading_t(timestamp, value));
    flush(false);
}

void MSGStore::add_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    init_buffers(sensor);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        _readings_buffer[sensor->uuid()]->insert(reading_t((*it).first, (*it).second));
    }
    flush(false);
}

void MSGStore::update_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    add_readings(sensor, readings);
}

readings_t_Ptr MSGStore::get_all_readings(const Sensor::Ptr sensor) {

    flush(sensor);

    struct json_object *jreadings = get_json_readings(sensor);
    int length = json_object_array_length(jreadings);
    readings_t_Ptr readings(new readings_t());

    for (int i = 0; i < length; i++) {

        json_object *jpair = json_object_array_get_idx(jreadings, i);
        std::pair<timestamp_t, double > reading = create_reading_pair(jpair);

        if (reading.first > 0) {
            readings->insert(reading);
        }
    }
    json_object_put(jreadings);

    return readings;
}

unsigned long int MSGStore::get_num_readings(const Sensor::Ptr sensor) {

    flush(sensor);

    json_object *jreadings = get_json_readings(sensor);
    long int num = json_object_array_length(jreadings);

    json_object_put(jreadings);

    return num;
}

std::pair<timestamp_t, double> MSGStore::get_last_reading(const Sensor::Ptr sensor) {

    flush(sensor);

    json_object *jreadings = get_json_readings(sensor);
    int i = json_object_array_length(jreadings) - 1;

    if (i >= 0) {
        json_object *jpair = json_object_array_get_idx(jreadings, i);
        std::pair<timestamp_t, double > reading = create_reading_pair(jpair);

        if (reading.first > 0) {
            json_object_put(jreadings);
            return reading;
        }
    }
    json_object_put(jreadings);

    std::ostringstream err;
    err << "No reading found for sensor " << sensor->uuid_short() << ".";
    throw StoreException(err.str());
}

void MSGStore::init_buffers(const Sensor::Ptr sensor) {

    _sensors_buffer[sensor->uuid()] = sensor;

    if (!_readings_buffer[sensor->uuid()]) {
        _readings_buffer[sensor->uuid()] = readings_t_Ptr(new readings_t());
    }
}

void MSGStore::clear_buffers(const Sensor::Ptr sensor) {

    _sensors_buffer.erase(sensor->uuid());
    _readings_buffer.erase(sensor->uuid());
}

void MSGStore::clear_buffers() {

    _sensors_buffer.clear();
    _readings_buffer.clear();
}

void MSGStore::flush(bool force) {

    TimeConverter tc;
    timestamp_t now = tc.get_timestamp();

    if (force || now - _last_sync > 300) {

        for (std::map<Sensor::uuid_t, Sensor::Ptr>::const_iterator it = _sensors_buffer.begin(); it != _sensors_buffer.end(); ++it) {

            Sensor::Ptr sensor = (*it).second;
            flush(sensor);
        }
        _last_sync = now;
    }
}

void MSGStore::flush(Sensor::Ptr sensor) {

    readings_t_Ptr readings = _readings_buffer[sensor->uuid()];

    if (!readings->empty()) {

        json_object *jtuples = json_object_new_array();

        for (readings_cit_t rit = readings->begin(); rit != readings->end(); ++rit) {

            timestamp_t timestamp = (*rit).first;
            double value = (*rit).second;

            struct json_object *jtuple = json_object_new_array();
            json_object_array_add(jtuple, json_object_new_int(timestamp));
            json_object_array_add(jtuple, json_object_new_double(value));

            json_object_array_add(jtuples, jtuple);
        }

        json_object *jobject = json_object_new_object();
        json_object_object_add(jobject, "measurements", jtuples);

        std::string url = compose_sensor_url(sensor);
        json_object *jresponse = perform_http_post(url, _key, jobject);

        json_object_put(jobject);
        json_object_put(jresponse);
        readings->clear();
    }
}

bool MSGStore::heartbeat() {

    json_object *jobject = json_object_new_object();

    //TODO: post firmware version and return parsed server response
    std::string url = compose_device_url();
    json_object *jresponse = perform_http_post(url, _key, jobject);
    bool success = jresponse != NULL;

    json_object_put(jobject);
    json_object_put(jresponse);

    return success;
}

std::vector<Sensor::Ptr> MSGStore::get_sensors() {

    std::vector<Sensor::Ptr> sensors;

    std::string url = compose_device_url();
    struct json_object *jobject = perform_http_get(url, _key);
    json_object *jsensors = json_object_object_get(jobject, "sensors");

    for (int i = 0; i < json_object_array_length(jsensors); i++) {

        json_object *jsensor = json_object_array_get_idx(jsensors, i);
        json_object *jmeter = json_object_object_get(jsensor, "meter");
        const char* meter = json_object_get_string(jmeter);

        sensors.push_back(parse_sensor(
                format_uuid_string(meter),
                jsensor));

    }
    return sensors;
}

struct json_object *MSGStore::get_json_readings(const Sensor::Ptr sensor) {

    std::ostringstream query;
    query << "interval=hour&unit=" << sensor->unit();

    std::string url = compose_sensor_url(sensor, query.str());
    return perform_http_get(url, _key);
}

Sensor::Ptr MSGStore::parse_sensor(const std::string& uuid_str, json_object *jsensor) {

    json_object *jname = json_object_object_get(jsensor, "function");
    const char* name = json_object_get_string(jname);

    json_object *jdescription = json_object_object_get(jsensor, "description");
    const char* description = json_object_get_string(jdescription);

    json_object *jexternal_id = json_object_object_get(jsensor, "externalid");
    const char* external_id = json_object_get_string(jexternal_id);

    json_object *junit = json_object_object_get(jsensor, "unit");
    const char* unit = json_object_get_string(junit);

    //TODO: there should be no default timezone
    const char* timezone = "Europe/Berlin";

    SensorFactory::Ptr sensor_factory(new SensorFactory());

    return sensor_factory->createSensor(
            uuid_str,
            std::string(external_id),
            std::string(name),
            std::string(description),
            std::string(unit),
            std::string(timezone));
}

std::pair<timestamp_t, double > MSGStore::create_reading_pair(json_object *jpair) {

    json_object *jvalue = json_object_array_get_idx(jpair, 1);
    double value = json_object_get_double(jvalue);

    if (isnan(value)) {
        return std::pair<timestamp_t, double>(0, 0);

    } else {
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        long epoch = json_object_get_int(jtimestamp);

        TimeConverter::Ptr tc(new TimeConverter());
        timestamp_t timestamp = tc->convert_from_epoch(epoch);

        return std::pair<timestamp_t, double>(timestamp, value);
    }
}

const std::string MSGStore::format_uuid_string(const std::string& meter) {

    std::stringstream oss;
    for (size_t i = 0; i < meter.length(); i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            oss << '-';
        }
        oss << meter[i];
    }
    return oss.str();
}

const std::string MSGStore::compose_device_url() {

    return compose_url(std::string("device"), _id);
}

const std::string MSGStore::compose_sensor_url(const Sensor::Ptr sensor) {

    return compose_url(std::string("sensor"), sensor->uuid_short());
}

const std::string MSGStore::compose_sensor_url(const Sensor::Ptr sensor, const std::string& query) {

    std::ostringstream oss;
    oss << compose_sensor_url(sensor) << "?" << query;
    return oss.str();
}

const std::string MSGStore::compose_url(const std::string& object, const std::string& id) {

    std::ostringstream oss;
    oss << _url << "/" << object << "/" << id;
    return oss.str();
}

struct json_object *MSGStore::perform_http_get(const std::string& url, const std::string& key) {

    return perform_http_request("GET", url, key, NULL);
}

struct json_object *MSGStore::perform_http_post(const std::string& url, const std::string& key, json_object *jbody) {

    return perform_http_request("POST", url, key, jbody);
}

void MSGStore::perform_http_delete(const std::string& url, const std::string& key) {

    json_object *jobject = perform_http_request("DELETE", url, key, NULL);
    json_object_put(jobject);
}

std::string MSGStore::digest_message(const std::string& data, const std::string& key) {

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

    HMAC_cleanup(&hmacContext);

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

struct json_object *MSGStore::perform_http_request(const std::string& method, const std::string& url, const std::string& key, json_object *jbody) {

    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        throw StoreException("CURL could not be initiated.");
    }

    CURLresponse response;
    response.data = NULL;
    response.size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_custom_callback);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

    //Signal-handling is NOT thread-safe.
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    //Required if next router has an ip-change.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: libklio");
    headers = curl_slist_append(headers, "X-Version: 1.0");
    headers = curl_slist_append(headers, "Accept: application/json,text/html");

    const char* body = jbody == NULL ? "" : json_object_to_json_string(jbody);
    std::ostringstream oss;
    oss << "X-Digest: " << digest_message(body, key);
    headers = curl_slist_append(headers, oss.str().c_str());

    if (method == "POST") {
        headers = curl_slist_append(headers, "Content-type: application/json");

        oss.str(std::string());
        oss << "Content-Length: " << strlen(body);
        headers = curl_slist_append(headers, oss.str().c_str());

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode curl_code = curl_easy_perform(curl);

    long int http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    json_object *jobject = NULL;

    if (curl_code == CURLE_OK && http_code == 200) {

        jobject = json_tokener_parse(response.data);

    } else {
        oss.str(std::string());
        oss << "HTTPS request failed." <<
                " cURL Error: " << curl_easy_strerror(curl_code) << ". " <<
                " HTTPS code: " << http_code;
    }

    curl_free(response.data);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (jobject == NULL) {
        throw StoreException(oss.str());
    }

    return jobject;
}
