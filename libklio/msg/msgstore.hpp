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
 */

#ifndef LIBKLIO_MSG_MSGSTORE_HPP
#define LIBKLIO_MSG_MSGSTORE_HPP 1

#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/store.hpp>
#include <vector>

namespace klio {
  class MSGStore : public Store {
    public:
      typedef std::tr1::shared_ptr<MSGStore> Ptr;

      MSGStore (const std::string& url) :
        _url(url) {};
      virtual ~MSGStore () {
        close();
      };

      void open();
      void initialize();
      void close();
      const std::string str(); 

      virtual void addSensor(klio::Sensor::Ptr sensor);
      virtual void removeSensor(const klio::Sensor::Ptr sensor);
      virtual klio::Sensor::Ptr getSensor(const klio::Sensor::uuid_t& uuid);
      virtual std::vector<klio::Sensor::uuid_t> getSensorUUIDs();
      virtual std::vector<klio::Sensor::Ptr> getSensorById(const std::string& sensor1_id);
      virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value);
      virtual void add_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
      virtual void update_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
      virtual void add_description(klio::Sensor::Ptr sensor, const std::string& desc);
      virtual readings_t_Ptr get_all_readings(klio::Sensor::Ptr sensor);
      virtual unsigned long int get_num_readings(klio::Sensor::Ptr sensor);
      virtual std::pair<timestamp_t, double> get_last_reading(klio::Sensor::Ptr sensor);
      virtual void sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store);
      
    private:
      MSGStore (const MSGStore& original);
      MSGStore& operator = (const MSGStore& rhs);
      std::string _url;
  };
};

#endif /* LIBKLIO_MSG_MSGSTORE_HPP */
