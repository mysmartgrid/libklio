#include <sstream>
#include "transaction.hpp"


using namespace klio;

void Transaction::start() {

    if (!_pending && sqlite3_exec(_db, "BEGIN", 0, 0, 0) == SQLITE_OK) {
        _pending = true;

    } else {
        log_error("begin");
    }
}

void Transaction::commit() {

    if (_pending && sqlite3_exec(_db, "COMMIT", 0, 0, 0) == SQLITE_OK) {
        _pending = false;
        
    } else {
        log_error("commit");
        rollback();
    }
}

void Transaction::rollback() {

    if (_pending && sqlite3_exec(_db, "ROLLBACK", 0, 0, 0) == SQLITE_OK) {
        _pending = false;
        
    } else {
        log_error("rollback");
    }
}

void Transaction::log_error(const std::string& operation) {

    std::ostringstream oss;
    oss << "Can't " << operation << " transaction: " << sqlite3_errmsg(_db);
    LOG(oss.str());
}