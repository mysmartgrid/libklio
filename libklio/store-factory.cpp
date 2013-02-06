#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>
#include "store-factory.hpp"


using namespace klio;

Store::Ptr StoreFactory::open_sqlite3_store(const bfs::path& path) {

    Store::Ptr retval(new SQLite3Store(path));
    retval->open();
    return retval;
}

Store::Ptr StoreFactory::open_msg_store(const std::string& url) {

    Store::Ptr retval(new MSGStore(url));
    retval->open();
    return retval;
}
