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

#ifndef LIBKLIO_SENSOR_HPP
#define LIBKLIO_SENSOR_HPP 1

#include <libklio/common.hpp>
#include <map>
#include <iostream>
#include <boost/uuid/uuid.hpp>

namespace klio {
  const static std::string DEFAULT_SENSOR_DESCRIPTION="Unknown";
  class Sensor {
    public:
      typedef std::tr1::shared_ptr<Sensor> Ptr;
      typedef boost::uuids::uuid uuid_t;
      Sensor(const uuid_t uuid, const std::string& name, 
          const std::string& desc,
          const std::string& unit, const std::string& timezone) :
        _uuid(uuid), _name(name), _unit(unit), 
        _timezone(timezone), _desc(desc) {};
      virtual ~Sensor() {};
      // standard sensor methods
      const std::string str();
      const std::string name() const { return _name; };
      const uuid_t uuid() const { return _uuid; };
      const std::string uuid_string() const; 
      const std::string description() const { return _desc; };
      const std::string unit() const { return _unit; };
      const std::string timezone() const { return _timezone; };

      bool operator == (const Sensor& rhs);
      bool operator != (const Sensor& rhs);

    private:
      Sensor (const Sensor& original);
      Sensor& operator= (const Sensor& rhs);
      uuid_t _uuid;
      std::string _name;
      std::string _unit;
      std::string _timezone;
      std::string _desc;
  };
};


#endif /* LIBKLIO_SENSOR_HPP */

