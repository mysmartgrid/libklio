#ifndef LIBKLIO_SQLITE3_SQLITE3STORE_HPP
#define LIBKLIO_SQLITE3_SQLITE3STORE_HPP 1

#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <vector>
#include <sqlite3.h>

namespace klio {
  class SQLite3Store : public Store {
    public:
      typedef std::tr1::shared_ptr<SQLite3Store> Ptr;
      SQLite3Store (const std::string& filename) :
        _filename(filename) {};
      virtual void addSensor(klio::Sensor::Ptr sensor);
      virtual void removeSensor(const klio::Sensor::Ptr sensor);
      virtual klio::Sensor::Ptr getSensor(const klio::Sensor::uuid_t& uuid);
      virtual std::vector<klio::Sensor::uuid_t> getSensorUUIDs();
      virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value);
      virtual std::map<timestamp_t, double> get_all_readings(klio::Sensor::Ptr sensor);
      void open();
      void initialize();
      void close();
      const std::string str(); 
      virtual ~SQLite3Store() {
        close();
      };

    private:
      SQLite3Store (const SQLite3Store& original);
      SQLite3Store& operator= (const SQLite3Store& rhs);
      bool has_table(std::string name);
      void checkSensorTable();
      sqlite3 *db;
      std::string _filename;
  };
  
};


#endif /* LIBKLIO_SQLITE3_SQLITE3STORE_HPP */

