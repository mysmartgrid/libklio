/**
 * This class represents an abstract Klio store.
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

#ifndef LIBKLIO_STORE_HPP
#define LIBKLIO_STORE_HPP 1

#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/unordered_map.hpp>
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/sensor.hpp>
#include <libklio/time.hpp>
#include <libklio/transaction.hpp>
#include <libklio/sensor-factory.hpp>


namespace klio {

    class Store {
    public:
        typedef boost::shared_ptr<Store> Ptr;

        Store(bool auto_commit, bool auto_flush, const timestamp_t flush_timeout) :
        _auto_commit(auto_commit),
        _auto_flush(auto_flush),
        _flush_timeout(flush_timeout),
        _last_flush(0) {
        };

        virtual ~Store() {
        };

        virtual void open();
        virtual void close();
        virtual void check_integrity() = 0;
        virtual void initialize() = 0;
        virtual void dispose();
        virtual void prepare();
        virtual void flush();
        virtual const std::string str() = 0;

        void start_transaction();
        void commit_transaction();
        void rollback_transaction();

        void add_sensor(const Sensor::Ptr sensor);
        void remove_sensor(const Sensor::Ptr sensor);
        void update_sensor(const Sensor::Ptr sensor);

        Sensor::Ptr get_sensor(const klio::Sensor::uuid_t& uuid);
        std::vector<klio::Sensor::Ptr> get_sensors_by_external_id(const std::string& external_id);
        std::vector<klio::Sensor::Ptr> get_sensors_by_name(const std::string& name);
        std::vector<klio::Sensor::uuid_t> get_sensor_uuids();
        std::vector<klio::Sensor::Ptr> get_sensors();

        void add_reading(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value);
        void add_readings(const Sensor::Ptr sensor, const readings_t& readings);
        void update_readings(const Sensor::Ptr sensor, const readings_t& readings);

        readings_t_Ptr get_all_readings(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_readings(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end);
        reading_t get_last_reading(const Sensor::Ptr sensor);
        reading_t get_reading(const Sensor::Ptr sensor, const timestamp_t timestamp);
        unsigned long int get_num_readings(const Sensor::Ptr sensor);

        void sync(const Store::Ptr store);
        void sync_readings(const Sensor::Ptr sensor, const Store::Ptr store);
        void sync_sensors(const Store::Ptr store);

    protected:
        static const SensorFactory::Ptr sensor_factory;
        static const TimeConverter::Ptr time_converter;

        virtual Transaction::Ptr create_transaction();
        Transaction::Ptr get_transaction();
        virtual void start_inner_transaction(const Transaction::Ptr transaction);
        virtual void commit_inner_transaction(const Transaction::Ptr transaction);

        virtual void add_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void remove_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void update_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void add_readings_records(const Sensor::Ptr sensor, const readings_t& readings) = 0;
        virtual void update_readings_records(const Sensor::Ptr sensor, const readings_t& readings) = 0;

        virtual std::vector<klio::Sensor::Ptr> get_sensors_records() = 0;
        virtual readings_t_Ptr get_all_readings_records(const Sensor::Ptr sensor) = 0;
        virtual readings_t_Ptr get_timeframe_readings_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) = 0;
        virtual reading_t get_last_reading_record(const Sensor::Ptr sensor) = 0;
        virtual reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) = 0;
        virtual unsigned long int get_num_readings_value(const Sensor::Ptr sensor) = 0;

        virtual void clear_buffers();

    private:
        typedef unsigned int cached_operation_type_t;
        typedef std::pair<const cached_operation_type_t, const klio::readings_t_Ptr> cached_readings_type_t;
        typedef std::map<const cached_operation_type_t, const klio::readings_t_Ptr> cached_operations_type_t;
        typedef std::map<const cached_operation_type_t, const klio::readings_t_Ptr>::const_iterator cached_operations_type_it_t;
        typedef boost::shared_ptr<cached_operations_type_t> cached_operations_type_t_Ptr;

        Store(const Store& original);
        Store& operator=(const Store& rhs);

        static const cached_operation_type_t INSERT_OPERATION;
        static const cached_operation_type_t UPDATE_OPERATION;
        static const cached_operation_type_t DELETE_OPERATION;

        bool _auto_commit;
        bool _auto_flush;
        timestamp_t _flush_timeout;
        timestamp_t _last_flush;
        Transaction::Ptr _transaction;

        boost::unordered_map<const Sensor::uuid_t, cached_operations_type_t_Ptr> _readings_operations_buffer;
        boost::unordered_map<const Sensor::uuid_t, Sensor::Ptr> _sensors_buffer;
        boost::unordered_map<std::string, Sensor::uuid_t> _external_ids_buffer;

        void add_readings(const Sensor::Ptr sensor, const readings_t& readings, const cached_operation_type_t operation_type);
        void flush_readings(const Sensor::Ptr sensor);
        readings_t_Ptr get_buffered_readings(const Sensor::Ptr sensor, const cached_operation_type_t operation_type);

        void flush(const bool force);
        virtual void flush(const Sensor::Ptr sensor);
        void flush(const Sensor::Ptr sensor, const bool force);
        void set_buffers(const Sensor::Ptr sensor);
        void clear_buffers(const Sensor::Ptr sensor);
    };
};

#endif /* LIBKLIO_STORE_HPP */
