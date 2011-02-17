#include "sensor.hpp"
#include <sstream>
#include <boost/uuid/uuid_io.hpp>

using namespace klio;


const std::string Sensor::str() {
  std::ostringstream oss;
  oss << "Sensor " << _name << "(" << to_string(_uuid) << "), unit " 
    << _unit << ", tz=" << (int)_timezone;
  return oss.str();
}


int Sensor::operator == (const Sensor& rhs) {
  if (_name==rhs.name() && _uuid==rhs.uuid() &&
      _unit==rhs.unit() && _timezone==rhs.timezone())
    return 1;
  else
    return 0;
}
