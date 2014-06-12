#include <sstream>
#include "sqlite3-transaction.hpp"


using namespace klio;

void SQLite3Transaction::start() {

    if (!pending() && sqlite3_exec(_db, "BEGIN", 0, 0, 0) == SQLITE_OK) {
        pending(true);

    } else {
        log_error("begin");
    }
}

void SQLite3Transaction::commit() {

    if (pending() && sqlite3_exec(_db, "COMMIT", 0, 0, 0) == SQLITE_OK) {
        pending(false);
        
    } else {
        log_error("commit");
        rollback();
    }
}

void SQLite3Transaction::rollback() {

    if (pending() && sqlite3_exec(_db, "ROLLBACK", 0, 0, 0) == SQLITE_OK) {
        pending(false);
        
    } else {
        log_error("rollback");
    }
}

void SQLite3Transaction::log_error(const std::string& operation) {

    std::ostringstream oss;
    oss << "Can't " << operation << " transaction: " << sqlite3_errmsg(_db);
    LOG(oss.str());
}
