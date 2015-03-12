#include <sstream>
#include <libklio/sqlite3/sqlite3-transaction.hpp>


using namespace klio;

void SQLite3Transaction::start() {

    if (_db == NULL) {
        std::ostringstream oss;
        oss << "Database is not open.";
        throw StoreException(oss.str());

    } else if (_pending) {
        LOG("Transaction is already started.");

    } else if (sqlite3_exec(_db, "BEGIN", NULL, NULL, NULL) == SQLITE_OK) {
        _pending = true;
        LOG("Transaction has been started.");

    } else {
        LOG("Can't start transaction: " << sqlite3_errmsg(_db));
    }
}

void SQLite3Transaction::commit() {

    if (_db == NULL) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        if (sqlite3_exec(_db, "COMMIT", NULL, NULL, NULL) == SQLITE_OK) {
            _pending = false;
            LOG("Transaction has been committed.");

        } else {
            LOG("Can't commit transaction: " << sqlite3_errmsg(_db));
        }
    } else {
        LOG("Transaction is not started.");
    }
}

void SQLite3Transaction::rollback() {

    if (_db == NULL) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        if (sqlite3_exec(_db, "ROLLBACK", NULL, NULL, NULL) == SQLITE_OK) {
            _pending = false;
            LOG("Transaction has been rolled back.");

        } else {
            LOG("Can't rollback transaction: " << sqlite3_errmsg(_db));
        }
    } else {
        LOG("Transaction is not rolled back.");
    }
}
