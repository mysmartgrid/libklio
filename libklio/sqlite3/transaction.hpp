#ifndef LIBKLIO_SQLITE3_TRANSACTION_HPP
#define LIBKLIO_SQLITE3_TRANSACTION_HPP 1

#include <libklio/common.hpp>
#include <sqlite3.h>

namespace klio {
  class Transaction {
    public:
      typedef std::tr1::shared_ptr<Transaction> Ptr;
      Transaction (sqlite3* db);
      void commit();
      virtual ~Transaction();

    private:
      Transaction (const Transaction& original);
      Transaction& operator= (const Transaction& rhs);
      bool _is_commited;
      sqlite3* _db;
  };
};

#endif /* LIBKLIO_SQLITE3_TRANSACTION_HPP */
