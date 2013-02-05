#include <libklio/sqlite3/sqlite3store.hpp>
#include <libklio/msg/msgstore.hpp>
#include "store-factory.hpp"


using namespace klio;

Store::Ptr StoreFactory::create_store(const STORETYPE& type, const bfs::path& path) {

    return open_store(type, path);
}

Store::Ptr StoreFactory::open_store(const STORETYPE& type, const bfs::path& path) {

    if (type == klio::SQLITE3) {
        return open_sqlite3_store(path);

    } else {
        throw DataFormatException("Unknown storage type requested.");
    }
}

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
