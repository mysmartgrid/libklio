#ifndef LIBKLIO_STORE_HPP
#define LIBKLIO_STORE_HPP 1

#include <libklio/common.hpp>

namespace klio {
  enum STORETYPE {UNDEFINED, SQLITE3};

  class Store {
    public:
      typedef std::tr1::shared_ptr<Store> Ptr;
      Store () {};
      virtual void open() = 0;
      virtual void close() = 0;
      // getSensor: get singleton for sensor, data insertion + retrieval works over this one
      virtual void createSensor() = 0;
      // deleteSensor
      virtual const std::string str() = 0;
      virtual ~Store() {};
    private:
      Store (const Store& original);
      Store& operator= (const Store& rhs);
  };
};

#endif 

