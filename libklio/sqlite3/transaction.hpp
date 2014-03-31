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

#ifndef LIBKLIO_SQLITE3_TRANSACTION_HPP
#define LIBKLIO_SQLITE3_TRANSACTION_HPP 1

#include <sqlite3.h>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>


namespace klio {

    class Transaction {
    public:
        typedef boost::shared_ptr<Transaction> Ptr;

        Transaction(sqlite3* _db) :
        _db(_db),
        _pending(false) {
            start();
        }

        virtual ~Transaction() {
            rollback();
        }

        void commit();

    private:
        Transaction(const Transaction& original);
        Transaction& operator=(const Transaction& rhs);

        void start();
        void rollback();
        void log_error(const std::string& operation);

        sqlite3* _db;
        bool _pending;
    };
};

#endif /* LIBKLIO_SQLITE3_TRANSACTION_HPP */
