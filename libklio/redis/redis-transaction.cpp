#include <sstream>
#include "redis-transaction.hpp"


using namespace klio;

const std::string RedisTransaction::MULTI = "MULTI";
const std::string RedisTransaction::EXEC = "EXEC";
const std::string RedisTransaction::DISCARD = "DISCARD";

void RedisTransaction::start() {

    if (!_connection) {
        std::ostringstream oss;
        oss << "Database is not open.";
        throw StoreException(oss.str());

    } else if (_pending) {
        LOG("Transaction is already started.");

    } else if (run(MULTI)) {
        _pending = true;
        LOG("Transaction has been started.");

    } else {
        LOG("Can't start transaction.");
    }
}

void RedisTransaction::commit() {

    if (!_connection) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        if (run(EXEC)) {
            _pending = false;
            LOG("Transaction has been committed.");

        } else {
            LOG("Can't commit transaction.");
        }
    } else {
        LOG("Transaction is not started.");
    }
}

void RedisTransaction::rollback() {

    if (!_connection) {
        //FIXME: raise an exception
        LOG("Database is not open.");

    } else if (_pending) {

        if (run(DISCARD)) {
            _pending = false;
            LOG("Transaction has been rolled back.");

        } else {
            LOG("Can't rollback transaction.");
        }
    } else {
        LOG("Transaction is not rolled back.");
    }
}

const bool RedisTransaction::run(const std::string& command) {

    //FIXME: execute command
    //_connection->run(redis3m::command(command));
    
    return true;
}
