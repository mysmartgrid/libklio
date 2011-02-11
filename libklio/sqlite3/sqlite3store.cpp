#include "sqlite3store.hpp"
#include <iostream>
#include <sstream>

using namespace klio;

// Opens a Database file.
void SQLite3Store::open () {
  int rc = sqlite3_open(_filename.c_str(), &db);
  if( rc ){
    std::ostringstream oss;
    oss << "Can't open database: " << sqlite3_errmsg(db);
    sqlite3_close(db);
    throw StoreException(oss.str());
  }
}


void SQLite3Store::initialize() {
  int rc;
  sqlite3_stmt* stmt;
  const char* pzTail ;
  const std::string createSensorTableStmt(
      "create table sensors(uuid varchar(16) primary key, unit varchar(20), timezone integer);");

  if (has_table("sensors")) {
    LOG("Sensors table already exists, skipping creation.");
    return;
  }

  rc=sqlite3_prepare(
      db,            /* Database handle */
      createSensorTableStmt.c_str(),       /* SQL statement, UTF-8 encoded */
      -1,              /* Maximum length of zSql in bytes - read complete string. */
      &stmt,  /* OUT: Statement handle */
      &pzTail     /* OUT: Pointer to unused portion of zSql */
    );
  if( rc!=SQLITE_OK ){
    std::ostringstream oss;
    oss << "Can't create sensors creation command: " << sqlite3_errmsg(db) << ", error code " << rc;
    sqlite3_finalize(stmt);
    throw StoreException(oss.str());
  }

  rc=sqlite3_step(stmt);
  if( rc!=SQLITE_DONE ) {  // sqlite3_step has finished, no further result lines available
    std::ostringstream oss;
    oss << "Can't create sensors table: " << sqlite3_errmsg(db) << ", error code " << rc;
    sqlite3_finalize(stmt);
    throw StoreException(oss.str());
  }

  sqlite3_finalize(stmt);
}

void SQLite3Store::close () {
  sqlite3_close(db);
}


bool SQLite3Store::has_table(std::string name) {
  int rc;
  sqlite3_stmt* stmt;
  const char* pzTail ;
  const std::string hasSensorTableStmt("select * from sqlite_master where name='sensors';");


  rc=sqlite3_prepare(
      db,                               /* Database handle */
      hasSensorTableStmt.c_str(),       /* SQL statement, UTF-8 encoded */
      -1,                               /* Maximum length of zSql in bytes - read complete string. */
      &stmt,                            /* OUT: Statement handle */
      &pzTail                           /* OUT: Pointer to unused portion of zSql */
      );
  if( rc!=SQLITE_OK ){
    std::ostringstream oss;
    oss << "Can't create sensors query command: " << sqlite3_errmsg(db) << ", error code " << rc;
    sqlite3_finalize(stmt);
    throw StoreException(oss.str());
  }

  rc=sqlite3_step(stmt);
  if( rc!=SQLITE_ROW ) {  // sqlite3_step has finished, there was no result row.
    sqlite3_finalize(stmt);
    return false;
  } else { // there was a result row - the table exists.
    sqlite3_finalize(stmt);
    return true;
  }
}

const std::string SQLite3Store::str() { 
  return std::string("sqlite3 store"); 
};

void SQLite3Store::addSensor() {
  std::cout << "Whee!" << std::endl;
}
