#ifndef LIBKLIO_SQLITE3_SQLITE3STORE_HPP
#define LIBKLIO_SQLITE3_SQLITE3STORE_HPP 1

#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <sqlite3.h>

namespace klio {
  class SQLite3Store : public Store {
    public:
      typedef std::tr1::shared_ptr<SQLite3Store> Ptr;
      SQLite3Store (const std::string& filename) :
        _filename(filename) {};
      virtual void addSensor();
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
      sqlite3 *db;
      std::string _filename;
  };
  
};


#endif /* LIBKLIO_SQLITE3_SQLITE3STORE_HPP */

