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

#ifndef LIBKLIO_STORE_HPP
#define LIBKLIO_STORE_HPP 1

#include <vector>
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/sensor.hpp>
#include <libklio/time.hpp>


namespace klio {

    class Store {
    public:
        typedef std::tr1::shared_ptr<Store> Ptr;

        Store() {
        };

        virtual ~Store() {
        };

        virtual void open() = 0;
        virtual void close() = 0;
        virtual void initialize() = 0;
        virtual const std::string str() = 0;

        virtual void add_sensor(klio::Sensor::Ptr sensor) = 0;
        virtual void remove_sensor(const klio::Sensor::Ptr sensor) = 0;
        virtual klio::Sensor::Ptr get_sensor(const klio::Sensor::uuid_t& uuid) = 0;
        virtual std::vector<klio::Sensor::Ptr> get_sensors_by_name(const std::string& name) = 0;
        virtual std::vector<klio::Sensor::uuid_t> get_sensor_uuids() = 0;
        virtual void add_description(klio::Sensor::Ptr sensor, const std::string& description) = 0;

        virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) = 0;
        virtual void add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) = 0;
        virtual void update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) = 0;
        virtual readings_t_Ptr get_all_readings(klio::Sensor::Ptr sensor) = 0;
        virtual reading_t get_last_reading(klio::Sensor::Ptr sensor) = 0;
        virtual unsigned long int get_num_readings(klio::Sensor::Ptr sensor) = 0;
        virtual void sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store);

    private:
        Store(const Store& original);
        Store& operator=(const Store& rhs);
    };
};

#endif /* LIBKLIO_STORE_HPP */
