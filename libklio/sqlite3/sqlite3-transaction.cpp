#include <sstream>
#include "sqlite3-transaction.hpp"


using namespace klio;

void SQLite3Transaction::start() {

    if (pending()) {
        LOG("Transaction is already started.");

    } else if (sqlite3_exec(_db, "BEGIN", NULL, NULL, NULL) == SQLITE_OK) {
        pending(true);
        LOG("Transaction has been started.");

    } else {
        LOG("Can't start transaction: " << sqlite3_errmsg(_db));
    }
}

void SQLite3Transaction::commit() {

    if (!pending()) {
        LOG("Transaction is not started.");

    } else if (sqlite3_exec(_db, "COMMIT", NULL, NULL, NULL) == SQLITE_OK) {
        pending(false);
        LOG("Transaction has been commited.");

    } else {
        LOG("Can't commit transaction: " << sqlite3_errmsg(_db));
    }
}

void SQLite3Transaction::rollback() {

    if (!pending()) {
        LOG("Transaction is not started.");

    } else if (sqlite3_exec(_db, "ROLLBACK", NULL, NULL, NULL) == SQLITE_OK) {
        pending(false);
        LOG("Transaction has been rolled back.");

    } else {
        LOG("Can't rollback transaction: " << sqlite3_errmsg(_db));
    }
}
