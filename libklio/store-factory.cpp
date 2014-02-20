#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include "store-factory.hpp"


using namespace boost::algorithm;
using namespace klio;

SQLite3Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    return SQLite3Store::Ptr(new SQLite3Store(path));
}

SQLite3Store::Ptr StoreFactory::open_sqlite3_store(const bfs::path& path) {

    SQLite3Store::Ptr store = create_sqlite3_store(path);
    store->open();
    store->check_integrity();
    return store;
}

MSGStore::Ptr StoreFactory::create_msg_store() {

    return create_msg_store(
            boost::uuids::to_string(_gen()),
            boost::uuids::to_string(_gen()));
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

#ifdef ENABLE_ROCKSDB

RocksDBStore::Ptr StoreFactory::create_rocksdb_store(const bfs::path& path) {

    std::map<std::string, std::string> db_options;
    std::map<std::string, std::string> read_options;
    std::map<std::string, std::string> write_options;

    return create_rocksdb_store(path, db_options, read_options, write_options);
}

RocksDBStore::Ptr StoreFactory::create_rocksdb_store(const bfs::path& path,
        const std::map<std::string, std::string>& db_options,
        const std::map<std::string, std::string>& read_options,
        const std::map<std::string, std::string>& write_options) {

   return RocksDBStore::Ptr(new RocksDBStore(path, db_options, read_options, write_options));
}

#endif /* ENABLE_ROCKSDB */