#include "sqlite3store.hpp"
#include <iostream>
#include <sstream>

using namespace klio;


void SQLite3Store::open () {
  int rc = sqlite3_open(_filename.c_str(), &db);
  if( rc ){
    std::ostringstream oss;
    oss << "Can't open database: " << sqlite3_errmsg(db);
    sqlite3_close(db);
    throw StoreException(oss.str());
  }
}

void SQLite3Store::close () {
  sqlite3_close(db);
}

const std::string SQLite3Store::str() { 
  return std::string("sqlite3 store"); 
};

void SQLite3Store::createSensor() {
  std::cout << "Whee!" << std::endl;
}
