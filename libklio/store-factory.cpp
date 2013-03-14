#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace klio;

Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    Store::Ptr store(new SQLite3Store(path));
    store->open();
    return store;
}

Store::Ptr StoreFactory::create_msg_store(const std::string& url, const std::string& id, const std::string& key) {

    Store::Ptr store(new MSGStore(url, id, key));
    store->open();
    return store;
}
