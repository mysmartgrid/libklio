#include "store-factory.hpp"
#include <libklio/sqlite3/sqlite3store.hpp>
#include <libklio/msg/msgstore.hpp>


using namespace klio;

/**
 * @deprecated
 */
Store::Ptr StoreFactory::createStore(const STORETYPE& type, const bfs::path& path) {

  return openStore(type, path);
}

/**
 * @deprecated
 */
Store::Ptr StoreFactory::openStore(const STORETYPE& type, const bfs::path& path) {

  if (type == klio::SQLITE3) {
    return createSQLite3Store(path);

  } else {
    throw DataFormatException("Unknown storage type requested.");
  }
}

Store::Ptr StoreFactory::createSQLite3Store(const bfs::path& path) {
    
  Store::Ptr retval(new SQLite3Store(path));
  retval->open();
  return retval;
}

Store::Ptr StoreFactory::createMSGStore(const std::string& url) {
    
  Store::Ptr retval(new MSGStore(url));
  retval->open();
  return retval;
}
