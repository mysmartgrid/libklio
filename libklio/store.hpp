#ifndef LIBKLIO_STORE_HPP
#define LIBKLIO_STORE_HPP 1

#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <vector>

namespace klio {
  enum STORETYPE {UNDEFINED, SQLITE3};

  class Store {
    public:
      typedef std::tr1::shared_ptr<Store> Ptr;
      Store () {};
      virtual ~Store() {};
      virtual void open() = 0;
      virtual void initialize() = 0;
      virtual void close() = 0;
      // getSensor: get singleton for sensor, data insertion + retrieval works over this one
      virtual void addSensor(klio::Sensor::Ptr sensor) = 0;
      virtual klio::Sensor::Ptr getSensor(const klio::Sensor::uuid_t& uuid) = 0;
      virtual std::vector<klio::Sensor::uuid_t> getSensorUUIDs() = 0;
      virtual void removeSensor(const klio::Sensor::Ptr sensor) = 0;
      virtual const std::string str() = 0;
      // methods for managing readings
      virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value) = 0;
      virtual std::map<timestamp_t, double> get_all_readings(klio::Sensor::Ptr sensor) = 0;

    private:
      Store (const Store& original);
      Store& operator= (const Store& rhs);
  };
};

#endif 

