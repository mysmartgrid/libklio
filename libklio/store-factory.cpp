#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/store-factory.hpp>


using namespace boost::algorithm;
using namespace klio;

boost::uuids::random_generator StoreFactory::_gen_uuid;

SQLite3Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path) {

    return create_sqlite3_store(path, true);
}

SQLite3Store::Ptr StoreFactory::create_sqlite3_store(const bfs::path& path, const bool prepare) {

    return create_sqlite3_store(path, prepare, true, true, 600, SQLite3Store::OS_SYNC_FULL);
}

SQLite3Store::Ptr StoreFactory::create_sqlite3_store(
        const bfs::path& path,
        const bool prepare,
        const bool auto_commit,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const std::string& synchronous) {

    SQLite3Store::Ptr store = SQLite3Store::Ptr(new SQLite3Store(path, auto_commit, auto_flush, flush_timeout, synchronous));
    store->open();
    store->initialize();
    if (prepare) {
        store->prepare();
    }
    return store;
}

SQLite3Store::Ptr StoreFactory::open_sqlite3_store(const bfs::path& path) {

    return open_sqlite3_store(path, true, true, 600, SQLite3Store::OS_SYNC_FULL);
}

SQLite3Store::Ptr StoreFactory::open_sqlite3_store(
        const bfs::path& path,
        const bool auto_commit,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const std::string& synchronous) {

    SQLite3Store::Ptr store = SQLite3Store::Ptr(new SQLite3Store(path, auto_commit, auto_flush, flush_timeout, synchronous));
    store->open();
    store->check_integrity();
    store->prepare();
    return store;
}

TXTStore::Ptr StoreFactory::create_txt_store(const bfs::path& path) {

    return create_txt_store(path, TXTStore::DEFAULT_FIELD_SEPARATOR);
}

TXTStore::Ptr StoreFactory::create_txt_store(const bfs::path& path, const std::string& field_separator) {

    TXTStore::Ptr store = TXTStore::Ptr(new TXTStore(path, field_separator));
    store->open();
    store->initialize();
    store->check_integrity();
    store->prepare();
    return store;
}

TXTStore::Ptr StoreFactory::open_txt_store(const bfs::path& path) {

    return open_txt_store(path, TXTStore::DEFAULT_FIELD_SEPARATOR);
}

TXTStore::Ptr StoreFactory::open_txt_store(const bfs::path& path, const std::string& field_separator) {

    TXTStore::Ptr store = TXTStore::Ptr(new TXTStore(path, field_separator));
    store->open();
    store->check_integrity();
    store->prepare();
    return store;
}

#ifdef ENABLE_MSG

MSGStore::Ptr StoreFactory::create_msg_store() {

    return create_msg_store(
            boost::uuids::to_string(_gen_uuid()),
            boost::uuids::to_string(_gen_uuid()));
}

MSGStore::Ptr StoreFactory::create_msg_store(const std::string& id, const std::string& key) {

    return create_msg_store(
            MSGStore::DEFAULT_MSG_URL,
            id,
            key,
            MSGStore::DEFAULT_MSG_DESCRIPTION,
            MSGStore::DEFAULT_MSG_TYPE);
}

MSGStore::Ptr StoreFactory::create_msg_store(
        const std::string& url,
        const std::string& id,
        const std::string& key,
        const std::string& description,
        const std::string& type) {

    MSGStore::Ptr store = MSGStore::Ptr(new MSGStore(url,
            erase_all_copy(id, "-"),
            erase_all_copy(key, "-"),
            description,
            type));
    store->open();
    store->initialize();
    store->prepare();
    return store;
}

MSGStore::Ptr StoreFactory::open_msg_store(const std::string& id, const std::string& key) {

    return open_msg_store(
            MSGStore::DEFAULT_MSG_URL,
            id,
            key);
}

MSGStore::Ptr StoreFactory::open_msg_store(
        const std::string& url,
        const std::string& id,
        const std::string& key) {

    MSGStore::Ptr store = MSGStore::Ptr(new MSGStore(url,
            erase_all_copy(id, "-"),
            erase_all_copy(key, "-"),
            MSGStore::DEFAULT_MSG_DESCRIPTION,
            MSGStore::DEFAULT_MSG_TYPE));
    store->open();
    store->check_integrity();
    store->prepare();
    return store;
}

#endif /* ENABLE_MSG */

#ifdef ENABLE_ROCKSDB

RocksDBStore::Ptr StoreFactory::create_rocksdb_store(const bfs::path& path) {

    return create_rocksdb_store(path, true, 600, false);
}

RocksDBStore::Ptr StoreFactory::create_rocksdb_store(const bfs::path& path,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const bool synchronous) {

    std::map<const std::string, const std::string> db_options;
    std::map<const std::string, const std::string> read_options;

    return create_rocksdb_store(path, auto_flush, flush_timeout, synchronous, db_options, read_options);
}

RocksDBStore::Ptr StoreFactory::create_rocksdb_store(const bfs::path& path,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const bool synchronous,
        const std::map<const std::string, const std::string>& db_options,
        const std::map<const std::string, const std::string>& read_options) {

    RocksDBStore::Ptr store = RocksDBStore::Ptr(new RocksDBStore(path, auto_flush, flush_timeout, synchronous, db_options, read_options));
    store->open();
    store->initialize();
    store->prepare();
    return store;
}

RocksDBStore::Ptr StoreFactory::open_rocksdb_store(const bfs::path& path) {

    return open_rocksdb_store(path, true, 600, false);
}

RocksDBStore::Ptr StoreFactory::open_rocksdb_store(const bfs::path& path,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const bool synchronous) {

    std::map<const std::string, const std::string> db_options;
    std::map<const std::string, const std::string> read_options;

    return open_rocksdb_store(path, auto_flush, flush_timeout, synchronous, db_options, read_options);
}

RocksDBStore::Ptr StoreFactory::open_rocksdb_store(const bfs::path& path,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const bool synchronous,
        const std::map<const std::string, const std::string>& db_options,
        const std::map<const std::string, const std::string>& read_options) {

    RocksDBStore::Ptr store = RocksDBStore::Ptr(new RocksDBStore(path, auto_flush, flush_timeout, synchronous, db_options, read_options));
    store->open();
    store->check_integrity();
    store->prepare();
    return store;
}

#endif /* ENABLE_ROCKSDB */


#ifdef ENABLE_REDIS3M

RedisStore::Ptr StoreFactory::create_redis_store() {

    return create_redis_store(
            RedisStore::DEFAULT_REDIS_HOST,
            RedisStore::DEFAULT_REDIS_PORT,
            RedisStore::DEFAULT_REDIS_DB);
}

RedisStore::Ptr StoreFactory::create_redis_store(const std::string& host, const unsigned int port, const unsigned int db) {

    return create_redis_store(host, port, db, true, true, 600);
}

RedisStore::Ptr StoreFactory::create_redis_store(
        const std::string& host,
        const unsigned int port,
        const unsigned int db,
        const bool auto_commit,
        const bool auto_flush,
        const timestamp_t flush_timeout) {

    RedisStore::Ptr store = RedisStore::Ptr(new RedisStore(host, port, db, auto_commit, auto_flush, flush_timeout));
    store->open();
    store->initialize();
    store->prepare();
    return store;
}

#endif /* ENABLE_REDIS3M */

#ifdef ENABLE_POSTGRESQL

PostgreSQLStore::Ptr StoreFactory::create_postgresql_store() {

    return create_postgresql_store(PostgreSQLStore::DEFAULT_CONNECTION_INFO);
}

PostgreSQLStore::Ptr StoreFactory::create_postgresql_store(const std::string& info) {

    return create_postgresql_store(info, true);
}

PostgreSQLStore::Ptr StoreFactory::create_postgresql_store(const std::string& info, const bool prepare) {

    return create_postgresql_store(info, prepare, true, true, 600, true);
}

PostgreSQLStore::Ptr StoreFactory::create_postgresql_store(
        const std::string& info,
        const bool prepare,
        const bool auto_commit,
        const bool auto_flush,
        const timestamp_t flush_timeout,
        const bool synchronous) {

    PostgreSQLStore::Ptr store = PostgreSQLStore::Ptr(new PostgreSQLStore(info, auto_commit, auto_flush, flush_timeout, synchronous));
    store->open();
    store->initialize();
    if (prepare) {
        store->prepare();
    }
    return store;
}

#endif /* ENABLE_POSTGRESQL */
