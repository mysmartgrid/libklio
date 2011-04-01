#include "sensorfactory.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

using namespace klio;

klio::Sensor::Ptr SensorFactory::createSensor(
    const std::string& name, 
    const std::string& unit, 
    const uint8_t timezone) 
{
  boost::uuids::uuid u = _gen();
  return createSensor(u, name, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
    const std::string& uuid_string, 
    const std::string& name, 
    const std::string& unit, 
    const uint8_t timezone
    ) 
{
  // type conversion: uuid_string to real uuid type
  boost::uuids::uuid u;
  std::stringstream ss;
  ss << uuid_string;
  ss >> u;
  return createSensor(u, name, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
    const Sensor::uuid_t& uuid,
    const std::string& name, 
    const std::string& unit, 
    const uint8_t timezone) 
{
  return Sensor::Ptr(new Sensor(uuid, name, unit, timezone));
}
