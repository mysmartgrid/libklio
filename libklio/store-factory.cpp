#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace boost::algorithm;
using namespace klio;


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
