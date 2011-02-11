#include "store-factory.hpp"
#include <libklio/sqlite3/sqlite3store.hpp>


using namespace klio;


Store::Ptr StoreFactory::createStore(const STORETYPE& type) {
  if (type == klio::SQLITE3) {
    Store::Ptr retval(new SQLite3Store("foostore.db"));
    retval->open();
    return retval;
  } else {
    throw DataFormatException("Unknown storage type requested.");
  }
}
