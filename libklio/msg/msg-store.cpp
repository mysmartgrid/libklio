#include <libklio/config.h>

#ifdef ENABLE_MSG

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
#include <curl/curl.h>
#include <json/json.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sensor-factory.hpp>
#include "msg-store.hpp"


using namespace klio;

void MSGStore::open() {
}

void MSGStore::close() {

    Store::flush();
    clear_buffers();
}

void MSGStore::initialize() {

    json_object *jobject = NULL;

    try {
        jobject = create_json_object();

        json_object_object_add(jobject, "key",
                create_json_string(_key.c_str()));

        json_object_object_add(jobject, "description",
                create_json_string(_description.c_str()));

        json_object_object_add(jobject, "type",
                create_json_string(_type.c_str()));

        std::string url = compose_device_url();
        json_object *jresponse = perform_http_post(url, _key, jobject);

        destroy_object(jresponse);
        destroy_object(jobject);

    } catch (GenericException const& e) {
        destroy_object(jobject);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jobject);
        throw EnvironmentException(e.what());
    }
}

void MSGStore::check_integrity() {
   //TODO: implement this method
}

void MSGStore::dispose() {

    //Tries to delete the remote store
    try {
        std::string url = compose_device_url();
        perform_http_delete(url, _key);

    } catch (GenericException const& e) {
        //Ignore failed attempt

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::flush() {
    heartbeat();
    Store::flush();
}

const std::string MSGStore::str() {

    std::ostringstream str;
    str << "mSG store " << compose_device_url();
    return str.str();
};

void MSGStore::add_sensor(const Sensor::Ptr sensor) {

    update_sensor(sensor);
}

void MSGStore::remove_sensor(const Sensor::Ptr sensor) {

    try {
        std::string url = compose_sensor_url(sensor);
        perform_http_delete(url, _key);
        clear_buffers(sensor);

    } catch (GenericException const& e) {
        throw;

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::update_sensor(const Sensor::Ptr sensor) {

    json_object *jconfig = NULL;

    try {
        jconfig = create_json_object();

        json_object *jdevice = create_json_string(_id.c_str());
        json_object_object_add(jconfig, "device", jdevice);

        json_object *jexternal_id = create_json_string(sensor->external_id().c_str());
        json_object_object_add(jconfig, "externalid", jexternal_id);

        json_object *jname = create_json_string(sensor->name().c_str());
        json_object_object_add(jconfig, "function", jname);

        json_object *jdescription = create_json_string(sensor->description().c_str());
        json_object_object_add(jconfig, "description", jdescription);

        json_object *junit = create_json_string(sensor->unit().c_str());
        json_object_object_add(jconfig, "unit", junit);

        json_object *jobject = create_json_object();
        json_object_object_add(jobject, "config", jconfig);
        jconfig = jobject;

        std::string url = compose_sensor_url(sensor);

        json_object *jresponse = perform_http_post(url, _key, jconfig);
        set_buffers(sensor);

        destroy_object(jresponse);
        destroy_object(jconfig);

    } catch (GenericException const& e) {
        destroy_object(jconfig);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jconfig);
        throw EnvironmentException(e.what());
    }
}

std::vector<Sensor::Ptr> MSGStore::get_sensors() {

    struct json_object *jobject = NULL;

    try {
        std::string url = compose_device_url();
        jobject = perform_http_get(url, _key);

        std::vector<Sensor::Ptr> sensors;
        json_object *jsensors = json_object_object_get(jobject, "sensors");

        for (int i = 0; i < json_object_array_length(jsensors); i++) {

            json_object *jsensor = json_object_array_get_idx(jsensors, i);
            json_object *jmeter = json_object_object_get(jsensor, "meter");
            const char* meter = json_object_get_string(jmeter);

            sensors.push_back(parse_sensor(
                    format_uuid_string(meter),
                    jsensor));
        }
        destroy_object(jobject);

        return sensors;

    } catch (GenericException const& e) {
        destroy_object(jobject);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jobject);
        throw EnvironmentException(e.what());
    }
}

void MSGStore::add_reading(const Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    set_buffers(sensor);
    _readings_buffer[sensor->uuid()]->insert(reading_t(timestamp, value));
    Store::flush(false);
}

void MSGStore::add_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    set_buffers(sensor);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {
        _readings_buffer[sensor->uuid()]->insert(reading_t((*it).first, (*it).second));
    }
    Store::flush(false);
}

void MSGStore::update_readings(const Sensor::Ptr sensor, const readings_t& readings) {

    add_readings(sensor, readings);
}

readings_t_Ptr MSGStore::get_all_readings(const Sensor::Ptr sensor) {

    struct json_object *jreadings = NULL;

    try {
        flush(sensor);
        jreadings = get_json_readings(sensor);

        int length = json_object_array_length(jreadings);
        readings_t_Ptr readings(new readings_t());

        for (int i = 0; i < length; i++) {

            json_object *jpair = json_object_array_get_idx(jreadings, i);
            std::pair<timestamp_t, double > reading = create_reading_pair(jpair);

            if (reading.first > 0) {
                readings->insert(reading);
            }
        }

        destroy_object(jreadings);
        return readings;

    } catch (GenericException const& e) {
        destroy_object(jreadings);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jreadings);
        throw EnvironmentException(e.what());
    }
}

readings_t_Ptr MSGStore::get_timeframe_readings(const Sensor::Ptr sensor,
        timestamp_t begin, timestamp_t end) {

    //TODO: improve this method

    readings_t_Ptr selected;
    readings_t_Ptr readings = get_all_readings(sensor);

    for (readings_cit_t reading = readings->begin(); reading != readings->end(); ++reading) {

        if ((*reading).first >= begin && (*reading).first <= end) {
            selected->insert(*reading);
        }
    }
    return selected;
}

unsigned long int MSGStore::get_num_readings(const Sensor::Ptr sensor) {

    struct json_object *jreadings = NULL;

    try {
        flush(sensor);
        jreadings = get_json_readings(sensor);
        long int num = json_object_array_length(jreadings);

        destroy_object(jreadings);
        return num;

    } catch (GenericException const& e) {
        destroy_object(jreadings);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jreadings);
        throw EnvironmentException(e.what());
    }
}

reading_t MSGStore::get_last_reading(const Sensor::Ptr sensor) {

    json_object *jreadings = NULL;

    try {
        flush(sensor);
        jreadings = get_json_readings(sensor);

        int i = json_object_array_length(jreadings) - 1;

        if (i >= 0) {
            json_object *jpair = json_object_array_get_idx(jreadings, i);
            std::pair<timestamp_t, double > reading = create_reading_pair(jpair);

            if (reading.first > 0) {
                destroy_object(jreadings);
                return reading;
            }
        }
        std::ostringstream err;
        err << "No reading found for sensor " << sensor->uuid_short() << ".";
        throw StoreException(err.str());

    } catch (GenericException const& e) {
        destroy_object(jreadings);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jreadings);
        throw EnvironmentException(e.what());
    }
}

reading_t MSGStore::get_reading(const Sensor::Ptr sensor, timestamp_t timestamp) {

    //FIXME: make this method more efficient
    klio::readings_t_Ptr readings = get_all_readings(sensor);

    std::pair<timestamp_t, double> reading = std::pair<timestamp_t, double>(0, 0);
    
    if (readings->count(timestamp)) {
        reading.first = timestamp;
        reading.second = readings->at(timestamp);
    }
    return reading;
}

void MSGStore::flush(const Sensor::Ptr sensor) {

    json_object *jmeasurements = NULL;

    try {
        readings_t_Ptr readings = _readings_buffer[sensor->uuid()];

        if (!readings->empty()) {

            jmeasurements = create_json_array();

            for (readings_cit_t rit = readings->begin(); rit != readings->end(); ++rit) {

                timestamp_t timestamp = (*rit).first;
                double value = (*rit).second;

                struct json_object *jtuple = create_json_array();
                json_object_array_add(jmeasurements, jtuple);

                json_object_array_add(jtuple, create_json_int(timestamp));
                json_object_array_add(jtuple, create_json_double(value));
            }

            json_object *jobject = create_json_object();
            json_object_object_add(jobject, "measurements", jmeasurements);
            jmeasurements = jobject;

            std::string url = compose_sensor_url(sensor);

            json_object *jresponse = perform_http_post(url, _key, jmeasurements);
            readings->clear();

            destroy_object(jresponse);
            destroy_object(jmeasurements);
        }

    } catch (GenericException const& e) {
        destroy_object(jmeasurements);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jmeasurements);
        throw EnvironmentException(e.what());
    }
}

void MSGStore::heartbeat() {

    json_object *jobject = NULL;

    try {
        timestamp_t now = time_converter->get_timestamp();

        //Send a heartbeat every 30 minutes
        if (now - _last_heartbeat > 1800) {

            json_object *jobject = create_json_object();
            std::string url = compose_device_url();

            json_object *jresponse = perform_http_post(url, _key, jobject);
            _last_heartbeat = now;

            destroy_object(jresponse);
            destroy_object(jobject);
        }

    } catch (GenericException const& e) {
        destroy_object(jobject);
        throw;

    } catch (std::exception const& e) {
        destroy_object(jobject);
        throw EnvironmentException(e.what());
    }
}

const std::string MSGStore::format_uuid_string(const std::string & meter) {

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

const std::string MSGStore::compose_sensor_url(const Sensor::Ptr sensor, const std::string & query) {

    std::ostringstream oss;
    oss << compose_sensor_url(sensor) << "?" << query;
    return oss.str();
}

const std::string MSGStore::compose_url(const std::string& object, const std::string & id) {

    std::ostringstream oss;
    oss << _url << "/" << object << "/" << id;
    return oss.str();
}

struct json_object * MSGStore::perform_http_get(const std::string& url, const std::string & key) {

    return perform_http_request("GET", url, key, NULL);
}

struct json_object * MSGStore::perform_http_post(const std::string& url, const std::string& key, json_object * jbody) {

    return perform_http_request("POST", url, key, jbody);
}

void MSGStore::perform_http_delete(const std::string& url, const std::string & key) {

    json_object *jobject = perform_http_request("DELETE", url, key, NULL);
    destroy_object(jobject);
}

std::string MSGStore::digest_message(const std::string& data, const std::string& key) {

    HMAC_CTX context;
    HMAC_CTX_init(&context);
    HMAC_Init_ex(&context, key.c_str(), key.length(), EVP_sha1(), NULL);
    HMAC_Update(&context, (const unsigned char*) data.c_str(), data.length());

    unsigned char out[EVP_MAX_MD_SIZE];
    unsigned int len = EVP_MAX_MD_SIZE;

    HMAC_Final(&context, out, &len);
    char ret[2 * EVP_MAX_MD_SIZE];
    memset(ret, 0, sizeof (ret));

    for (size_t i = 0; i < len; i++) {
        char s[4];
        snprintf(s, 3, "%02x:", out[i]);
        strncat(ret, s, 2 * len);
    }

    HMAC_CTX_cleanup(&context);

    char digest[255];
    memset(digest, 0, sizeof (digest));
    snprintf(digest, 255, "%s", ret);
    return std::string(digest);
}

static size_t curl_write_custom_callback(void *ptr, size_t size, size_t nmemb, void *data) {

    size_t realsize = size * nmemb;
    CURLresponse *response = static_cast<CURLresponse *> (data);

    response->data = (char *) realloc(response->data, response->size + realsize + 1);
    if (response->data == NULL) { // out of memory!
        std::ostringstream err;
        err << "Cannot allocate memory for CURLresponse.";
        throw MemoryException(err.str());
    }

    memcpy(&(response->data[response->size]), ptr, realsize);
    response->size += realsize;
    response->data[response->size] = 0;

    return realsize;
}

struct json_object * MSGStore::perform_http_request(const std::string& method, const std::string& url, const std::string& key, json_object * jbody) {

    long int http_code = 0;
    CURLcode curl_code;
    CURLresponse response;
    CURL *curl = NULL;
    json_object *jobject = NULL;
    curl_slist *headers = NULL;

    try {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) {
            throw EnvironmentException("CURL could not be initiated.");
        }

        response.data = NULL;
        response.size = 0;

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_custom_callback);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

        //Signal-handling is NOT thread-safe.
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        //Required if next router has an ip-change.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

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

        curl_code = curl_easy_perform(curl);

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (curl_code == CURLE_OK && http_code == 200) {

            jobject = json_tokener_parse(response.data);

            //Clean up
            curl_free(response.data);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();

            return jobject;

        } else {
            oss.str(std::string());
            oss << "HTTPS request failed." <<
                    " cURL Error: " << curl_easy_strerror(curl_code) << ". " <<
                    " HTTPS code: " << http_code;

            if (http_code >= 400 && http_code <= 499) {
                throw DataFormatException(oss.str());

            } else if (http_code >= 500 || http_code == 0) {
                throw CommunicationException(oss.str());

            } else {
                throw StoreException(oss.str());
            }
        }

    } catch (std::exception const& e) {

        //Clean up
        if (response.data != NULL) {
            curl_free(response.data);
        }
        if (headers != NULL) {
            curl_slist_free_all(headers);
        }
        if (curl != NULL) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        throw;
    }
    //Some compilers require a return here
    throw StoreException("This point should never be reached.");
    return NULL;
}

struct json_object * MSGStore::create_json_object() {

    json_object *jobject = json_object_new_object();

    if (jobject == NULL) {
        std::ostringstream err;
        err << "Cannot allocate memory for json_object.";
        throw MemoryException(err.str());
    }
    return jobject;
}

struct json_object * MSGStore::create_json_string(const std::string & string) {

    json_object *jstring = json_object_new_string(string.c_str());

    if (jstring == NULL) {
        std::ostringstream err;
        err << "Cannot allocate memory for json string.";
        throw MemoryException(err.str());
    }
    return jstring;
}

struct json_object * MSGStore::create_json_array() {

    struct json_object *jarray = json_object_new_array();

    if (jarray == NULL) {
        std::ostringstream err;
        err << "Cannot allocate memory for json array.";
        throw MemoryException(err.str());
    }
    return jarray;
}

struct json_object * MSGStore::create_json_int(const int value) {

    struct json_object *jint = json_object_new_int(value);

    if (jint == NULL) {
        std::ostringstream err;
        err << "Cannot allocate memory for json int.";
        throw MemoryException(err.str());
    }
    return jint;
}

struct json_object * MSGStore::create_json_double(const double value) {

    struct json_object *jdouble = json_object_new_double(value);

    if (jdouble == NULL) {
        std::ostringstream err;
        err << "Cannot allocate memory for json double.";
        throw MemoryException(err.str());
    }
    return jdouble;
}

struct json_object * MSGStore::get_json_readings(const Sensor::Ptr sensor) {

    std::ostringstream query;
    query << "interval=hour&unit=" << sensor->unit();

    std::string url = compose_sensor_url(sensor, query.str());
    return perform_http_get(url, _key);
}

Sensor::Ptr MSGStore::parse_sensor(const std::string& uuid_str, json_object * jsensor) {

    json_object *jname = json_object_object_get(jsensor, "function");
    const char* name = json_object_get_string(jname);

    json_object *jdescription = json_object_object_get(jsensor, "description");
    const char* description = json_object_get_string(jdescription);

    json_object *jexternal_id = json_object_object_get(jsensor, "externalid");
    const char* external_id = json_object_get_string(jexternal_id);

    json_object *junit = json_object_object_get(jsensor, "unit");
    const char* unit = json_object_get_string(junit);

    //TODO: there should be no hard coded default timezone
    const char* timezone = "Europe/Berlin";

    return sensor_factory->createSensor(
            uuid_str,
            std::string(external_id),
            std::string(name),
            std::string(description),
            std::string(unit),
            std::string(timezone));
}

std::pair<timestamp_t, double > MSGStore::create_reading_pair(json_object * jpair) {

    json_object *jvalue = json_object_array_get_idx(jpair, 1);
    double value = json_object_get_double(jvalue);

    if (isnan(value)) {
        return std::pair<timestamp_t, double>(0, 0);

    } else {
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        long epoch = json_object_get_int(jtimestamp);
        timestamp_t timestamp = time_converter->convert_from_epoch(epoch);

        return std::pair<timestamp_t, double>(timestamp, value);
    }
}

void MSGStore::destroy_object(json_object * jobject) {

    if (jobject != NULL) {
        json_object_put(jobject);
    }
}

#endif /* ENABLE_MSG */