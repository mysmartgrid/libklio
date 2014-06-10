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
#include <boost/optional/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/sensor.hpp>
#include <libklio/time.hpp>
#include <libklio/sensor-factory.hpp>


namespace klio {

    class Store {
    public:
        typedef boost::shared_ptr<Store> Ptr;

        Store(const timestamp_t sync_timeout) :
        _last_sync(0),
        _sync_timeout(sync_timeout) {
        };

        virtual ~Store() {
        };

        virtual void open() = 0;
        virtual void close() = 0;
        virtual void check_integrity() = 0;
        virtual void initialize() = 0;
        virtual void dispose() = 0;
        virtual void prepare();
        virtual void flush();
        virtual const std::string str() = 0;

        virtual void add_sensor(const Sensor::Ptr sensor) = 0;
        virtual void remove_sensor(const klio::Sensor::Ptr sensor) = 0;
        virtual void update_sensor(const klio::Sensor::Ptr sensor) = 0;

        virtual Sensor::Ptr get_sensor(const klio::Sensor::uuid_t& uuid);
        virtual std::vector<klio::Sensor::Ptr> get_sensors_by_external_id(const std::string& external_id);
        virtual std::vector<klio::Sensor::Ptr> get_sensors_by_name(const std::string& name);
        virtual std::vector<klio::Sensor::uuid_t> get_sensor_uuids();
        virtual std::vector<klio::Sensor::Ptr> get_sensors() = 0;

        virtual void add_reading(const Sensor::Ptr sensor, timestamp_t timestamp, double value);
        virtual void add_readings(const Sensor::Ptr sensor, const readings_t& readings);
        virtual void update_readings(const Sensor::Ptr sensor, const readings_t& readings);
        virtual readings_t_Ptr get_all_readings(const Sensor::Ptr sensor) = 0;
        virtual readings_t_Ptr get_timeframe_readings(const Sensor::Ptr sensor, timestamp_t begin, timestamp_t end) = 0;
        virtual reading_t get_last_reading(const Sensor::Ptr sensor) = 0;
        virtual reading_t get_reading(const Sensor::Ptr sensor, timestamp_t timestamp);
        virtual unsigned long int get_num_readings(const Sensor::Ptr sensor) = 0;

        void sync(const Store::Ptr store);
        void sync_readings(const Sensor::Ptr sensor, const Store::Ptr store);

    protected:
        static const SensorFactory::Ptr sensor_factory;
        static const TimeConverter::Ptr time_converter;

        std::map<Sensor::uuid_t, Sensor::Ptr> _sensors_buffer;
        std::map<Sensor::uuid_t, readings_t_Ptr> _readings_buffer;
        std::map<std::string, Sensor::uuid_t> _external_ids_buffer;

        void set_buffers(const Sensor::Ptr sensor);
        void clear_buffers(const Sensor::Ptr sensor);
        virtual void clear_buffers();
        virtual void flush(const Sensor::Ptr sensor) = 0;

    private:
        Store(const Store& original);
        Store& operator=(const Store& rhs);

        timestamp_t _last_sync;
        timestamp_t _sync_timeout;

        void flush(bool force);
    };
};

#endif /* LIBKLIO_STORE_HPP */
