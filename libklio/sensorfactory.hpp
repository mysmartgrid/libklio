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
      klio::Sensor::Ptr createSensor(const std::string& name, 
          const std::string& unit, 
          const uint8_t timezone);
      virtual ~SensorFactory() {};

    private:
      SensorFactory (const SensorFactory& original);
      SensorFactory& operator= (const SensorFactory& rhs);
      boost::uuids::random_generator _gen;
      
  };
  
};


#endif /* LIBKLIO_SENSORFACTORY_HPP */

