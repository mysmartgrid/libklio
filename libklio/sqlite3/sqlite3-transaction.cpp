#include <sstream>
#include "sqlite3-transaction.hpp"


using namespace klio;

void SQLite3Transaction::start() {

    if (pending()) {
        LOG("Transaction is already started.");

    } else if (sqlite3_exec(_db, "BEGIN", 0, 0, 0) == SQLITE_OK) {
        pending(true);

    } else {
        LOG("Can't start transaction: " << sqlite3_errmsg(_db));
    }
}

void SQLite3Transaction::commit() {

    if (!pending()) {
        LOG("Transaction is not started.");

    } else if (sqlite3_exec(_db, "COMMIT", 0, 0, 0) == SQLITE_OK) {
        pending(false);

    } else {
        LOG("Can't commit transaction: " << sqlite3_errmsg(_db));
    }
}

void SQLite3Transaction::rollback() {

    if (!pending()) {
        LOG("Transaction is not started.");

    } else if (sqlite3_exec(_db, "ROLLBACK", 0, 0, 0) == SQLITE_OK) {
        pending(false);

    } else {
        LOG("Can't rollback transaction: " << sqlite3_errmsg(_db));
    }
}
