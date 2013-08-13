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
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/store.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/msg/msg-store.hpp>

namespace bfs = boost::filesystem;

namespace klio {

    class StoreFactory {
    public:
        typedef std::tr1::shared_ptr<StoreFactory> Ptr;
        typedef boost::uuids::uuid uuid_t;

        StoreFactory() : _gen() {
        };

        virtual ~StoreFactory() {
        };

        SQLite3Store::Ptr create_sqlite3_store(const bfs::path& path);
        SQLite3Store::Ptr open_sqlite3_store(const bfs::path& path);
        MSGStore::Ptr create_msg_store();
        MSGStore::Ptr create_msg_store(const std::string& id, const std::string& key);
        MSGStore::Ptr create_msg_store(const std::string& url, const std::string& id, const std::string& key, const std::string& description, const std::string& type);

    private:
        StoreFactory(const StoreFactory& original);
        StoreFactory& operator =(const StoreFactory& rhs);
        boost::uuids::random_generator _gen;
    };
};

#endif /* LIBKLIO_STORE_FACTORY_HPP */
