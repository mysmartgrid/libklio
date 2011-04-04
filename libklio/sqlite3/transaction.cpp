#include "transaction.hpp"
#include <sstream>


using namespace klio;

Transaction::Transaction(sqlite3* db) : 
        _is_commited(false), _db(db)
{
  if ( sqlite3_exec(_db, "BEGIN", 0, 0, 0) != SQLITE_OK ){
    std::ostringstream oss;
    oss << "Can't begin transaction: " << sqlite3_errmsg(_db);
    //throw StoreException(oss.str());
    LOG(oss.str());
  }
}

void Transaction::commit() {
  if ( sqlite3_exec(_db, "COMMIT", 0, 0, 0) != SQLITE_OK ){
    sqlite3_exec(_db, "ROLLBACK", 0, 0, 0);
    std::ostringstream oss;
    oss << "Can't commit changes to database: " << sqlite3_errmsg(_db);
    //throw StoreException(oss.str());
    LOG(oss.str());
  } else {
    _is_commited=true;
  }
}

Transaction::~Transaction() {
  if (not _is_commited) {
    if ( sqlite3_exec(_db, "ROLLBACK", 0, 0, 0) != SQLITE_OK ){
      std::ostringstream oss;
      oss << "Can't roll back database transaction: " << sqlite3_errmsg(_db);
      //  throw StoreException(oss.str());
      LOG(oss.str());
    }
  }
}

