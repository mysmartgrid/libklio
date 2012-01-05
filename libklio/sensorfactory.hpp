#ifndef LIBKLIO_SENSORFACTORY_HPP
#define LIBKLIO_SENSORFACTORY_HPP 1

#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace klio {
  class SensorFactory {
    public:
      typedef std::tr1::shared_ptr<SensorFactory> Ptr;
      SensorFactory () : _gen() {};
      klio::Sensor::Ptr createSensor(
          const std::string& name, 
          const std::string& unit, 
          const std::string& timezone
          );
      klio::Sensor::Ptr createSensor(
          const std::string& uuid_string, 
          const std::string& name, 
          const std::string& unit, 
          const std::string& timezone
          );
      klio::Sensor::Ptr createSensor(
          const std::string& uuid_string, 
          const std::string& name, 
          const std::string& description, 
          const std::string& unit, 
          const std::string& timezone
          );
 
      virtual ~SensorFactory() {};

  

    private:
      SensorFactory (const SensorFactory& original);
      SensorFactory& operator= (const SensorFactory& rhs);
      boost::uuids::random_generator _gen;
      klio::Sensor::Ptr createSensor(
        const Sensor::uuid_t& uuid,
        const std::string& name, 
        const std::string& description, 
        const std::string& unit, 
        const std::string& timezone
      );
      
  };
  
};


#endif /* LIBKLIO_SENSORFACTORY_HPP */

