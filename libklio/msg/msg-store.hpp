/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>,    2010
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
 *
 * libklio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libklio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libklio. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBKLIO_MSG_MSGSTORE_HPP
#define LIBKLIO_MSG_MSGSTORE_HPP 1

#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/store.hpp>


namespace klio {

    typedef struct {
        char *data;
        size_t size;
    } CURLresponse;

    class MSGStore : public Store {
    public:
        typedef std::tr1::shared_ptr<MSGStore> Ptr;

        MSGStore(const std::string& url, const std::string& id, const std::string& key) :
        _url(url),
        _id(id),
        _key(key) {
        };

        virtual ~MSGStore() {
            close();
        };

        void open();
        void close();
        void initialize();
        void dispose();
        const std::string str();

        virtual void add_sensor(klio::Sensor::Ptr sensor);
        virtual void remove_sensor(const klio::Sensor::Ptr sensor);
        virtual void update_sensor(const klio::Sensor::Ptr sensor);
        virtual klio::Sensor::Ptr get_sensor(const klio::Sensor::uuid_t& uuid);
        virtual std::vector<klio::Sensor::Ptr> get_sensors_by_name(const std::string& name);
        virtual std::vector<klio::Sensor::uuid_t> get_sensor_uuids();

        virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value);
        virtual void add_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
        virtual void update_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
        virtual readings_t_Ptr get_all_readings(klio::Sensor::Ptr sensor);
        virtual unsigned long int get_num_readings(klio::Sensor::Ptr sensor);
        virtual std::pair<timestamp_t, double> get_last_reading(klio::Sensor::Ptr sensor);

    private:
        MSGStore(const MSGStore& original);
        MSGStore& operator =(const MSGStore& rhs);
        std::string _url;
        std::string _id;
        std::string _key;

        struct json_object *get_json_sensors();
        struct json_object *get_json_readings(klio::Sensor::Ptr sensor);

        //TODO: move these functions to another API to be shared with VZLogger
        struct json_object *perform_http_get(std::string url, std::string key);
        struct json_object *perform_http_post(std::string url, std::string key, json_object *jobject);
        void *perform_http_delete(std::string url, std::string key);
        curl_slist *create_curl_headers();
        CURL *create_curl_handler(std::string url, curl_slist *headers);
        std::string digest_message(std::string data, std::string key);
        struct json_object *perform_http_request(CURL *curl);
    };
};

#endif /* LIBKLIO_MSG_MSGSTORE_HPP */
