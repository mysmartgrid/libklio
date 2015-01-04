#include <libklio/config.h>

#ifdef ENABLE_POSTGRESQL

#include <sstream>
#include <libklio/postgresql/postgresql-transaction.hpp>


using namespace klio;

void PostgreSQLTransaction::start() {

    if (_connection == NULL) {
        std::ostringstream oss;
        oss << "Database is not open.";
        throw StoreException(oss.str());

    } else if (_pending) {
        LOG("Transaction is already started.");

    } else {

        PGresult *result = PQexec(_connection, "BEGIN");

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            _pending = true;
            LOG("Transaction has been started.");

        } else {
            LOG("Can't start transaction." << PQerrorMessage(_connection));
        }

        PQclear(result);
    }
}

void PostgreSQLTransaction::commit() {

    if (_connection == NULL) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        PGresult *result = PQexec(_connection, "COMMIT");

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            _pending = false;
            LOG("Transaction has been committed.");

        } else {
            LOG("Can't commit transaction." << PQerrorMessage(_connection));
        }

        PQclear(result);

    } else {
        LOG("Transaction is not started.");
    }
}

void PostgreSQLTransaction::rollback() {

    if (_connection == NULL) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        PGresult *result = PQexec(_connection, "ROLLBACK");

        if (PQresultStatus(result) == PGRES_COMMAND_OK) {
            _pending = false;
            LOG("Transaction has been rolled back.");

        } else {
            LOG("Can't rollback transaction: " << PQerrorMessage(_connection));
        }

        PQclear(result);

    } else {
        LOG("Transaction is not rolled back.");
    }
}

#endif /* ENABLE_POSTGRESQL */