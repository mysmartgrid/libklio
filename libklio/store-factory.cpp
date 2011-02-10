#include "store-factory.hpp"
#include <libklio/sqlite3/sqlite3store.hpp>


using namespace klio;


Store::Ptr StoreFactory::createStore(const STORETYPE& type) {
  if (type == klio::SQLITE3) {
    return Store::Ptr(new SQLite3Store("foostore.db"));
  } else {
    throw DataFormatException("Unknown storage type requested.");
  }
}
