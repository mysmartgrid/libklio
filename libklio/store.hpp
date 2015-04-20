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

        virtual ~Store() {
        };

        virtual void open() = 0;
        virtual void close() = 0;
        virtual void check_integrity() = 0;
        virtual void initialize() = 0;
        virtual void dispose() = 0;
        virtual void prepare();
        virtual void flush();
        void flush(bool ignore_errors);
        virtual const std::string str() = 0;

        //FIXME: move transactions handling out of this class
        void start_transaction();
        void commit_transaction();
        void rollback_transaction();

        void add_sensor(const Sensor::Ptr sensor);
        void remove_sensor(const Sensor::Ptr sensor);
        void update_sensor(const Sensor::Ptr sensor);

        Sensor::Ptr get_sensor(const Sensor::uuid_t& uuid);
        std::vector<Sensor::Ptr> get_sensors_by_external_id(const std::string& external_id);
        std::vector<Sensor::Ptr> get_sensors_by_name(const std::string& name);
        std::vector<Sensor::uuid_t> get_sensor_uuids();
        std::vector<Sensor::Ptr> get_sensors();

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

        Store(const bool auto_commit, const bool auto_flush, const timestamp_t flush_timeout, const unsigned int min_bulk_size, const unsigned int max_bulk_size) :
        _auto_commit(auto_commit),
        _auto_flush(auto_flush),
        _flush_timeout(flush_timeout),
        _last_flush(0),
        _min_bulk_size(min_bulk_size),
        _max_bulk_size(max_bulk_size) {
        };

        static const SensorFactory::Ptr sensor_factory;
        static const TimeConverter::Ptr time_converter;

        virtual Transaction::Ptr get_transaction_handler();
        Transaction::Ptr auto_start_transaction();
        virtual void auto_commit_transaction(const Transaction::Ptr transaction);

        virtual void add_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void remove_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void update_sensor_record(const Sensor::Ptr sensor) = 0;
        virtual void add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors) = 0;
        virtual void add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) = 0;
        virtual void update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) = 0;

        virtual std::vector<Sensor::Ptr> get_sensor_records() = 0;
        virtual readings_t_Ptr get_all_reading_records(const Sensor::Ptr sensor) = 0;
        virtual readings_t_Ptr get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) = 0;
        virtual reading_t get_last_reading_record(const Sensor::Ptr sensor) = 0;
        virtual reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) = 0;
        virtual unsigned long int get_num_readings_value(const Sensor::Ptr sensor) = 0;

        void set_buffers(const Sensor::Ptr sensor);
        virtual void clear_buffers();
        void handle_reading_insertion_error(const bool ignore_errors, const timestamp_t timestamp, const double value);
        void handle_reading_insertion_error(const bool ignore_errors, const Sensor::Ptr sensor);

        bool _auto_commit;
        bool _auto_flush;
        boost::unordered_map<const Sensor::uuid_t, Sensor::Ptr> _sensors_buffer;

    private:
        typedef unsigned int cached_operation_type_t;
        typedef std::pair<const cached_operation_type_t, const readings_t_Ptr> cached_readings_type_t;
        typedef boost::unordered_map<const cached_operation_type_t, const readings_t_Ptr> cached_reading_operations_type_t;
        typedef boost::unordered_map<const cached_operation_type_t, const readings_t_Ptr>::const_iterator cached_reading_operations_type_it_t;
        typedef boost::shared_ptr<cached_reading_operations_type_t> cached_reading_operations_type_t_Ptr;

        Store(const Store& original);
        Store& operator=(const Store& rhs);

        static const cached_operation_type_t INSERT_OPERATION;
        static const cached_operation_type_t UPDATE_OPERATION;
        static const cached_operation_type_t DELETE_OPERATION;

        timestamp_t _flush_timeout;
        timestamp_t _last_flush;
        unsigned int _min_bulk_size;
        unsigned int _max_bulk_size;

        boost::unordered_map<const Sensor::uuid_t, cached_reading_operations_type_t_Ptr> _reading_operations_buffer;
        boost::unordered_map<const std::string, Sensor::uuid_t> _external_ids_buffer;

        void sync_reading_records(const Sensor::Ptr sensor, const Store::Ptr store);
        Sensor::Ptr sync_sensor_record(const Sensor::Ptr sensor);
        void add_readings(const Sensor::Ptr sensor, const readings_t& readings, const cached_operation_type_t operation_type);
        readings_t_Ptr get_buffered_readings(const Sensor::Ptr sensor, const cached_operation_type_t operation_type);

        void check_out_commit_off();
        void auto_flush();
        void flush_all(const bool ignore_errors);
        void flush(const Sensor::Ptr sensor, const bool ignore_errors);
        void clear_buffers(const Sensor::Ptr sensor);
        void handle_reading_insertion_error(const bool ignore_errors, const std::string message);
    };
};

#endif /* LIBKLIO_STORE_HPP */
