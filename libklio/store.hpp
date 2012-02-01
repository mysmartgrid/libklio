
/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>, 2010
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
 *
 */

#ifndef LIBKLIO_STORE_HPP
#define LIBKLIO_STORE_HPP 1

#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <libklio/time.hpp>
#include <vector>

namespace klio {
  enum STORETYPE {UNDEFINED, SQLITE3};
  typedef std::map<klio::timestamp_t, double> readings_t;
  typedef std::tr1::shared_ptr< readings_t > readings_t_Ptr;
  typedef std::map<klio::timestamp_t, double>::iterator readings_it_t;
  typedef std::map<klio::timestamp_t, double>::const_iterator readings_cit_t;
  typedef std::pair<klio::timestamp_t, double> reading_t;

  class Store {
    public:
      typedef std::tr1::shared_ptr<Store> Ptr;
      Store () {};
      virtual ~Store() {};
      virtual void open() = 0;
      virtual void initialize() = 0;
      virtual void close() = 0;
      // getSensor: get singleton for sensor, data insertion + retrieval works over this one
      virtual void addSensor(klio::Sensor::Ptr sensor) = 0;
      virtual klio::Sensor::Ptr getSensor(const klio::Sensor::uuid_t& uuid) = 0;
      virtual std::vector<klio::Sensor::uuid_t> getSensorUUIDs() = 0;
      virtual std::vector<klio::Sensor::Ptr> getSensorById(const std::string& sensor1_id) = 0;
      virtual void add_description(klio::Sensor::Ptr sensor, const std::string& desc) = 0;
      virtual void removeSensor(const klio::Sensor::Ptr sensor) = 0;
      virtual const std::string str() = 0;
      // methods for managing readings
      virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) = 0;
      virtual void add_readings(klio::Sensor::Ptr sensor, const readings_t& readings) = 0;
      virtual void update_readings(klio::Sensor::Ptr sensor, const readings_t& readings) = 0;
      virtual readings_t_Ptr get_all_readings(klio::Sensor::Ptr sensor) = 0;
      virtual reading_t get_last_reading(klio::Sensor::Ptr sensor) = 0;
      virtual long int get_num_readings(klio::Sensor::Ptr sensor) = 0;


    private:
      Store (const Store& original);
      Store& operator= (const Store& rhs);
  };
};

#endif 

