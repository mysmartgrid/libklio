#ifndef LIBKLIO_SENSOR_HPP
#define LIBKLIO_SENSOR_HPP 1

#include <libklio/common.hpp>

namespace klio {
  class Sensor {
    public:
      typedef std::tr1::shared_ptr<Sensor> Ptr;
      Sensor(std::string uuid, std::string unit, uint8_t timezone) :
        _uuid(uuid), _unit(unit), _timezone(timezone) {};
      const std::string str();
      virtual ~Sensor() {};

    private:
      Sensor (const Sensor& original);
      Sensor& operator= (const Sensor& rhs);
      std::string _uuid;
      std::string _unit;
      uint8_t _timezone;
  };
};


#endif /* LIBKLIO_SENSOR_HPP */

