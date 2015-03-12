/**
 * This class represents a local store, implemented as a Redis database.
 *
 * (c) Fraunhofer ITWM - Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2014
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

#ifndef LIBKLIO_REDIS_REDISSTORE_HPP
#define LIBKLIO_REDIS_REDISSTORE_HPP 1

#include <libklio/config.h>

#ifdef ENABLE_REDIS3M

#include <iostream>
#include <redis3m/redis3m.hpp>
#include <libklio/store.hpp>
#include <libklio/redis/redis-transaction.hpp>

using namespace redis3m;

namespace klio {

    class RedisStore : public Store {
    public:
        typedef boost::shared_ptr<RedisStore> Ptr;

        RedisStore(const std::string& host,
                const unsigned int port,
                const unsigned int db,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout) :
        Store(auto_commit, auto_flush, flush_timeout, 10, 10000),
        _host(host),
        _port(port),
        _db(db) {
        };

        virtual ~RedisStore() {
            close();
        };

        const std::string host() const {
            return _host;
        };

        const unsigned int port() const {
            return _port;
        };

        const unsigned int db() const {
            return _db;
        };

        void open();
        void close();
        void check_integrity();
        void initialize();
        void dispose();
        const std::string str();

        static const std::string DEFAULT_REDIS_HOST;
        static const unsigned int DEFAULT_REDIS_PORT;
        static const unsigned int DEFAULT_REDIS_DB;
        static const std::string OK;

    protected:
        Transaction::Ptr get_transaction_handler();

        void add_sensor_record(const Sensor::Ptr sensor);
        void remove_sensor_record(const Sensor::Ptr sensor);
        void update_sensor_record(const Sensor::Ptr sensor);
        void add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors);
        void add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);
        void update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);

        std::vector<Sensor::Ptr> get_sensor_records();
        readings_t_Ptr get_all_reading_records(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end);
        unsigned long int get_num_readings_value(const Sensor::Ptr sensor);
        reading_t get_last_reading_record(const Sensor::Ptr sensor);
        reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp);

    private:
        RedisStore(const RedisStore& original);
        RedisStore& operator =(const RedisStore& rhs);

        RedisTransaction::Ptr create_transaction_handler();

        const std::string check_sensor_existence(const Sensor::Ptr sensor, const bool should_exist);

        void run_del_readings(const Sensor::Ptr sensor);

        void run_hmset_sensor(const std::string& key, const Sensor::Ptr sensor);
        const Sensor::Ptr run_hmget_sensor(const std::string& key);
        void run_hmset_reading(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value);
        void run_hmset_readings(const Sensor::Ptr sensor, const readings_t& readings);
        readings_t_Ptr run_hget_readings(const Sensor::Ptr sensor);
        void run_hdel_sensor(const std::string& key);
        const std::vector<reply> run_hkeys(const std::string& key);
        const std::string run_hget(const std::string& key, const std::string& field);

        void run_sadd(const std::string& key, const std::string& value);
        const std::vector<reply> run_smembers(const std::string& key);
        void run_srem(const std::string& key, const std::string& value);

        void run_select(const unsigned int index);
        void run_flushdb();

        const std::string compose_sensor_key(const Sensor::Ptr sensor);
        const std::string compose_readings_key(const Sensor::Ptr sensor);

        std::string _host;
        unsigned int _port;
        unsigned int _db;
        redis3m::connection::ptr_t _connection;
        RedisTransaction::Ptr _transaction;

        static const std::string SENSORS_KEY;
        static const std::string SENSOR_KEY;
        static const std::string READINGS_KEY;

        static const std::string SADD;
        static const std::string SMEMBERS;
        static const std::string SREM;

        static const std::string HMSET;
        static const std::string HMGET;
        static const std::string HGETALL;
        static const std::string HGET;
        static const std::string HDEL;
        static const std::string HKEYS;

        static const std::string DEL;
        static const std::string SELECT;
        static const std::string FLUSHDB;
    };
};

#endif /* ENABLE_REDIS3M */

#endif /* LIBKLIO_REDIS_REDISSTORE_HPP */
