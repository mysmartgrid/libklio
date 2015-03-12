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

#ifndef LIBKLIO_POSTGRESQL_TRANSACTION_HPP
#define LIBKLIO_POSTGRESQL_TRANSACTION_HPP 1

#include <postgresql/libpq-fe.h>
#include <libklio/transaction.hpp>


namespace klio {

    class PostgreSQLTransaction : public Transaction {
    public:
        typedef boost::shared_ptr<PostgreSQLTransaction> Ptr;

        PostgreSQLTransaction(PGconn* connection) :
        Transaction(),
        _connection(connection) {
        }

        virtual ~PostgreSQLTransaction() {
            rollback();
        }

        void connection(PGconn* connection) {
            _connection = connection;
        }

        void start();
        void commit();
        void rollback();

    private:
        PostgreSQLTransaction(const PostgreSQLTransaction& original);
        PostgreSQLTransaction& operator=(const PostgreSQLTransaction& rhs);

        PGconn* _connection;
    };
};

#endif /* LIBKLIO_POSTGRESQL_TRANSACTION_HPP */
