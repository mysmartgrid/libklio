/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>,    2010
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
 *
 * libklio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libklio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libklio. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBKLIO_STORE_FACTORY_HPP
#define LIBKLIO_STORE_FACTORY_HPP 1

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/shared_ptr.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/txt/txt-store.hpp>
#ifdef ENABLE_MSG
#include <libklio/msg/msg-store.hpp>
#endif /* ENABLE_MSG */
#ifdef ENABLE_ROCKSDB
#include <libklio/rocksdb/rocksdb-store.hpp>
#endif /* ENABLE_ROCKSDB */
#ifdef ENABLE_REDIS3M
#include <libklio/redis/redis-store.hpp>
#endif /* ENABLE_REDIS3M */
#ifdef ENABLE_POSTGRESQL
#include <libklio/postgresql/postgresql-store.hpp>
#endif /* ENABLE_POSTGRESQL */

namespace bfs = boost::filesystem;

namespace klio {

    class StoreFactory {
    public:
        typedef boost::shared_ptr<StoreFactory> Ptr;
        typedef boost::uuids::uuid uuid_t;

        StoreFactory() {
        };

        virtual ~StoreFactory() {
        };

        SQLite3Store::Ptr create_sqlite3_store(const bfs::path& path);

        SQLite3Store::Ptr create_sqlite3_store(const bfs::path& path, const bool prepare);

        SQLite3Store::Ptr create_sqlite3_store(
                const bfs::path& path,
                const bool prepare,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                //Visit: http://www.sqlite.org/pragma.html#pragma_synchronous
                const std::string& synchronous
                );

        SQLite3Store::Ptr open_sqlite3_store(const bfs::path& path);

        SQLite3Store::Ptr open_sqlite3_store(
                const bfs::path& path,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                //Visit: http://www.sqlite.org/pragma.html#pragma_synchronous
                const std::string& synchronous
                );

        TXTStore::Ptr create_txt_store(const bfs::path& path);

        TXTStore::Ptr create_txt_store(const bfs::path& path, const std::string& field_separator);

        TXTStore::Ptr open_txt_store(const bfs::path& path);

        TXTStore::Ptr open_txt_store(const bfs::path& path, const std::string& field_separator);

#ifdef ENABLE_MSG

        MSGStore::Ptr create_msg_store();

        MSGStore::Ptr create_msg_store(const std::string& id, const std::string& key);

        MSGStore::Ptr create_msg_store(const std::string& url,
                const std::string& id,
                const std::string& key,
                const std::string& description,
                const std::string& type
                );

        MSGStore::Ptr open_msg_store(const std::string& id, const std::string& key);

        MSGStore::Ptr open_msg_store(
                const std::string& url,
                const std::string& id,
                const std::string& key
                );

#endif /* ENABLE_MSG */

#ifdef ENABLE_ROCKSDB

        RocksDBStore::Ptr create_rocksdb_store(const bfs::path& path);

        RocksDBStore::Ptr create_rocksdb_store(const bfs::path& path,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous
                );

        RocksDBStore::Ptr create_rocksdb_store(const bfs::path& path,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous,
                const std::map<const std::string, const std::string>& db_options,
                const std::map<const std::string, const std::string>& read_options
                );

        RocksDBStore::Ptr open_rocksdb_store(const bfs::path& path);

        RocksDBStore::Ptr open_rocksdb_store(const bfs::path& path,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous
                );

        RocksDBStore::Ptr open_rocksdb_store(const bfs::path& path,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous,
                const std::map<const std::string, const std::string>& db_options,
                const std::map<const std::string, const std::string>& read_options
                );

#endif /* ENABLE_ROCKSDB */

#ifdef ENABLE_REDIS3M

        RedisStore::Ptr create_redis_store();

        RedisStore::Ptr create_redis_store(
                const std::string& host,
                const unsigned int port,
                const unsigned int db
                );

        RedisStore::Ptr create_redis_store(
                const std::string& host,
                const unsigned int port,
                const unsigned int db,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout
                );

#endif /* ENABLE_REDIS3M */

#ifdef ENABLE_POSTGRESQL

        PostgreSQLStore::Ptr create_postgresql_store();

        PostgreSQLStore::Ptr create_postgresql_store(const std::string& info);

        PostgreSQLStore::Ptr create_postgresql_store(const std::string& info, const bool prepare);

        PostgreSQLStore::Ptr create_postgresql_store(
                const std::string& info,
                const bool prepare,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous
                );

#endif /* ENABLE_POSTGRESQL */

    private:
        StoreFactory(const StoreFactory& original);
        StoreFactory& operator =(const StoreFactory& rhs);

        static boost::uuids::random_generator _gen_uuid;
    };
};

#endif /* LIBKLIO_STORE_FACTORY_HPP */