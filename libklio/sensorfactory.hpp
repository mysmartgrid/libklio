#ifndef LIBKLIO_SENSORFACTORY_HPP
#define LIBKLIO_SENSORFACTORY_HPP 1

namespace klio {
  class SensorFactory {
    public:
      typedef std::tr1::shared_ptr<SensorFactory> Ptr;
      SensorFactory () {};
      klio::Sensor::Ptr createSensor(const std::string& unit, const uint8_t timezone);
      virtual ~SensorFactory() {};

    private:
      SensorFactory (const SensorFactory& original);
      SensorFactory& operator= (const SensorFactory& rhs);
      
  };
  
};


#endif /* LIBKLIO_SENSORFACTORY_HPP */

