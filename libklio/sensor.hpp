#ifndef LIBKLIO_SENSOR_HPP
#define LIBKLIO_SENSOR_HPP 1

#include <libklio/common.hpp>
#include <boost/uuid/uuid.hpp>

namespace klio {
  class Sensor {
    public:
      typedef std::tr1::shared_ptr<Sensor> Ptr;
      Sensor(const boost::uuids::uuid uuid, const std::string& name, 
          const std::string& unit, const uint8_t timezone) :
        _uuid(uuid), _name(name), _unit(unit), _timezone(timezone) {};
      virtual ~Sensor() {};
      const std::string str();
      const std::string name() const { return _name; };
      const boost::uuids::uuid uuid() const { return _uuid; };
      const std::string unit() const { return _unit; };
      const uint8_t timezone() const { return _timezone; };

      int operator == (const Sensor& rhs);

    private:
      Sensor (const Sensor& original);
      Sensor& operator= (const Sensor& rhs);
      boost::uuids::uuid _uuid;
      std::string _name;
      std::string _unit;
      uint8_t _timezone;
  };
};


#endif /* LIBKLIO_SENSOR_HPP */

