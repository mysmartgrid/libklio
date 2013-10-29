#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace boost::algorithm;
using namespace klio;


SQLite3Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    return SQLite3Store::Ptr(new SQLite3Store(path));
}

SQLite3Store::Ptr StoreFactory::open_sqlite3_store(const bfs::path& path) {

    SQLite3Store::Ptr store = create_sqlite3_store(path);
    store->open();
    return store;
}

MSGStore::Ptr StoreFactory::create_msg_store() {

    return create_msg_store(to_string(_gen()), to_string(_gen()));
}

MSGStore::Ptr StoreFactory::create_msg_store(const std::string& id, const std::string& key) {

    return create_msg_store("https://api.mysmartgrid.de:8443", id, key, "libklio mSG Store", "libklio");
}

MSGStore::Ptr StoreFactory::create_msg_store(
        const std::string& url,
        const std::string& id,
        const std::string& key,
        const std::string& description,
        const std::string& type) {

    return MSGStore::Ptr(new MSGStore(url,
            erase_all_copy(id, "-"),
            erase_all_copy(key, "-"),
            description,
            type));
}
