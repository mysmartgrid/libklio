#include "sensor.hpp"
#include <sstream>

using namespace klio;


const std::string Sensor::str() {
  std::ostringstream oss;
  oss << "Sensor " << _uuid << ", unit " << _unit << ", tz=" << _timezone;
  return oss.str();
}
