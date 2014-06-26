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

#ifndef LIBKLIO_TRANSACTION_HPP
#define LIBKLIO_TRANSACTION_HPP 1

#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>


namespace klio {

    class Transaction {
    public:
        typedef boost::shared_ptr<Transaction> Ptr;

        Transaction() :
        _pending(false) {
        }

        virtual ~Transaction() {
        }

        const bool pending() const {
            return _pending;
        };

        virtual void start();
        virtual void commit();
        virtual void rollback();

    protected:
        bool _pending;

    private:
        Transaction(const Transaction& original);
        Transaction& operator=(const Transaction& rhs);
    };
};

#endif /* LIBKLIO_TRANSACTION_HPP */
