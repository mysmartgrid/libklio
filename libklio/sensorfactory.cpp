#include "sensorfactory.hpp"

using namespace klio;

klio::Sensor::Ptr SensorFactory::createSensor(
    const std::string& name, 
    const std::string& unit, 
    const uint8_t timezone) 
{
  boost::uuids::uuid u = _gen();
  return Sensor::Ptr(new Sensor(u, name, unit, timezone));
}
