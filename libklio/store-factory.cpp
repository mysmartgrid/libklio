#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace klio;

Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    Store::Ptr store(new SQLite3Store(path));
    store->open();
    return store;
}

Store::Ptr StoreFactory::create_msg_store() {

    const std::string id = to_string(_gen());
    const std::string key = to_string(_gen());

    return create_msg_store("https://www.mysmartgrid.de:8443", id, key);
}

Store::Ptr StoreFactory::create_msg_store(const std::string& url, const std::string& id, const std::string& key) {

    Store::Ptr store(new MSGStore(url,
            boost::algorithm::erase_all_copy(id, "-"),
            boost::algorithm::erase_all_copy(key, "-")));
    store->open();
    return store;
}
