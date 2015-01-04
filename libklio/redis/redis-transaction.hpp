/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2014
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

#ifndef LIBKLIO_REDIS_TRANSACTION_HPP
#define LIBKLIO_REDIS_TRANSACTION_HPP 1

#include <redis3m/redis3m.hpp>
#include <libklio/transaction.hpp>


using namespace redis3m;

namespace klio {

    class RedisTransaction : public Transaction {
    public:
        typedef boost::shared_ptr<RedisTransaction> Ptr;

        RedisTransaction(connection::ptr_t connection) :
        Transaction(),
        _connection(connection) {
        }

        virtual ~RedisTransaction() {
            rollback();
        }

        void start();
        void commit();
        void rollback();

    private:
        RedisTransaction(const RedisTransaction& original);
        RedisTransaction& operator=(const RedisTransaction& rhs);

        const bool run(const std::string& command);

        connection::ptr_t _connection;

        static const std::string MULTI;
        static const std::string EXEC;
        static const std::string DISCARD;
    };
};

#endif /* LIBKLIO_REDIS_TRANSACTION_HPP */
