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

void MSGStore::add_sensor(klio::Sensor::Ptr sensor) {
}

void MSGStore::remove_sensor(const klio::Sensor::Ptr sensor) {
}

std::vector<klio::Sensor::uuid_t> MSGStore::get_sensor_uuids() {

    std::vector<klio::Sensor::uuid_t> uuids;
    return uuids;
}

std::vector<klio::Sensor::Ptr> MSGStore::get_sensor_by_id(const std::string& sensor_id) {

    std::vector<klio::Sensor::Ptr> retval;
    return retval;
}

klio::Sensor::Ptr MSGStore::get_sensor(const klio::Sensor::uuid_t& uuid) {

    klio::Sensor::Ptr retval;
    return retval;
}

void MSGStore::add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) {

    std::string url = "";
    url.append(_url);
    url.append("/sensor/");
    url.append(sensor->uuid_short().c_str());

    struct json_object *json_tuple = json_object_new_array();

    json_object_array_add(json_tuple, json_object_new_int(timestamp));
    json_object_array_add(json_tuple, json_object_new_int(value));

    json_object *json_tuples = json_object_new_array();
    json_object_array_add(json_tuples, json_tuple);

    json_object *json_obj = json_object_new_object();
    json_object_object_add(json_obj, "measurements", json_tuples);

    const char* json_str = json_object_to_json_string(json_obj);

    perform_http_post(url, sensor->key().c_str(), json_str);
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

    std::string response = perform_http_get(url, sensor->key().c_str());
    struct json_object *parsed = json_tokener_parse(response.c_str());

    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
    readings_t_Ptr readings(new readings_t());
    int length = json_object_array_length(parsed);

    for (int i = 0; i < length; i++) {

        json_object *jpair = json_object_array_get_idx(parsed, i);
        json_object *jtimestamp = json_object_array_get_idx(jpair, 0);
        json_object *jvalue = json_object_array_get_idx(jpair, 1);

        long epoch = json_object_get_int(jtimestamp);
        timestamp_t timestamp = tc->convert_from_epoch(epoch);
        double value = json_object_get_double(jvalue);

        readings->insert(std::pair<timestamp_t, double>(
                timestamp,
                value
                ));
    }
    return readings;
}

std::string MSGStore::perform_http_get(std::string url, std::string key) {

    std::string token_header = "X-Token: ";
    token_header.append(key);

    curl_slist *headers = create_curl_headers();
    headers = curl_slist_append(headers, token_header.c_str());

    CURL *curl = create_curl_handler(url, headers);

    return perform_http_request(curl);
}

std::string MSGStore::perform_http_post(std::string url, std::string key, std::string body) {

    std::string digest = digest_message(body, key);
    std::string digest_header = "X-Digest: ";
    digest_header.append(digest);

    curl_slist *headers = create_curl_headers();
    headers = curl_slist_append(headers, digest_header.c_str());

    CURL *curl = create_curl_handler(url, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    return perform_http_request(curl);
}

curl_slist *MSGStore::create_curl_headers() {

    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: libklio");
    headers = curl_slist_append(headers, "X-Version: 1.0");
    headers = curl_slist_append(headers, "Accept: application/json");

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

        // signal-handling in libcurl is NOT thread-safe.
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

std::string MSGStore::perform_http_request(CURL *curl) {

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
    }

    if (http_code != 200) {
        std::ostringstream oss;
        oss << "HTTPS request failed. HTTPS code: " << http_code;
        throw StoreException(oss.str());
    }

    std::string response = std::string(curl_response.data);
    free(curl_response.data);
    return response;
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

    sensor = get_sensor(sensor->uuid());

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}
