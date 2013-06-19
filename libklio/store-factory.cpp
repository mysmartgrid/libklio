#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace boost::algorithm;
using namespace klio;

/**
 * CHANGES
 * 
 * Ely: The master branch implementation of this class has two methods:
 *    createStore(const STORETYPE& type, const bfs::path& path)
 *    openStore(const STORETYPE& type, const bfs::path& path)
 * 
 * Although they have different names, they currently have the same implementation.
 * 
 * I assume that openStore is meant to both create and open a store, while
 * createStore is meant for creation only.
 * 
 * These methods have the same list of arguments and this creates a problem,
 * because different types of store tend to have different properties, possibly
 * using different data types. I think that limiting all of them to use a single
 * string argument called 'path' is too restrictive.
 * 
 * I think we should use different method names for different store types.
 * Some methods could be overloaded for cases when the user wants to create a
 * store using the default argument values.
 * 
 * I think this simplifies the API, makes the client code cleaner, and 
 * eliminates the need for 'ifs' and exception throwing.
 */

Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    return Store::Ptr(new SQLite3Store(path));
}

Store::Ptr StoreFactory::open_sqlite3_store(const bfs::path& path) {

    Store::Ptr store = create_sqlite3_store(path);
    store->open();
    return store;
}

Store::Ptr StoreFactory::create_msg_store() {

    return create_msg_store(to_string(_gen()), to_string(_gen()));
}

Store::Ptr StoreFactory::create_msg_store(const std::string& id, const std::string& key) {

    return create_msg_store("https://api.mysmartgrid.de:8443", id, key);
}

Store::Ptr StoreFactory::create_msg_store(const std::string& url, const std::string& id, const std::string& key) {

    return Store::Ptr(new MSGStore(url,
            erase_all_copy(id, "-"),
            erase_all_copy(key, "-")));
}
