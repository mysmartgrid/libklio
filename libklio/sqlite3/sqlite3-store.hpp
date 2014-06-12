/**
 * This class represents a local store, implemented as a SQlite 3 database.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>,    2010
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
 *
 * libklio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libklio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libklio. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBKLIO_SQLITE3_SQLITE3STORE_HPP
#define LIBKLIO_SQLITE3_SQLITE3STORE_HPP 1

#include <vector>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <libklio/sensor-factory.hpp>
#include "transaction.hpp"


namespace bfs = boost::filesystem;

namespace klio {

    class SQLite3Store : public Store {
    public:
        typedef boost::shared_ptr<SQLite3Store> Ptr;

        SQLite3Store(const bfs::path& path, bool auto_commit) :
        Store(0),
        _path(path),
        _db(NULL),
        _auto_commit(auto_commit),
        _insert_sensor_stmt(NULL),
        _remove_sensor_stmt(NULL),
        _update_sensor_stmt(NULL),
        _select_sensor_stmt(NULL),
        _select_sensor_by_external_id_stmt(NULL),
        _select_sensor_by_name_stmt(NULL),
        _select_sensors_stmt(NULL),
        _select_all_sensor_uuids_stmt(NULL) {
        };

        virtual ~SQLite3Store() {
            close();
        };

        void open();
        void close();
        void check_integrity();
        void initialize();
        void upgrade();
        void prepare();
        void dispose();
        const std::string str();
        
        void start_transaction();
        void commit_transaction();
        void rollback_transaction();

    protected:
        void add_sensor_record(const Sensor::Ptr sensor);
        void remove_sensor_record(const klio::Sensor::Ptr sensor);
        void update_sensor_record(const klio::Sensor::Ptr sensor);

        std::vector<klio::Sensor::Ptr> get_sensors_records();

        void add_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value);
        void update_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value);

        readings_t_Ptr get_all_readings_records(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_readings_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end);
        unsigned long int get_num_readings_value(const Sensor::Ptr sensor);
        reading_t get_last_reading_record(const Sensor::Ptr sensor);
        reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp);

    private:
        SQLite3Store(const SQLite3Store& original);
        SQLite3Store& operator =(const SQLite3Store& rhs);

        bool has_table(const std::string& name);
        bool has_column(const std::string& table, const std::string& column);

        Transaction::Ptr create_inner_transaction();
        void commit_inner_transaction(const Transaction::Ptr transaction);
        
        void add_reading_record(const Sensor::Ptr sensor, timestamp_t timestamp, const double value, const bool update);
        readings_t_Ptr get_readings_records(sqlite3_stmt* stmt);

        void flush(const Sensor::Ptr sensor);

        sqlite3_stmt *prepare(const std::string& stmt_str);
        sqlite3_stmt *get_statement(const std::string& sql);
        int execute(sqlite3_stmt *stmt, const int expected_code);
        void reset(sqlite3_stmt *stmt);
        void finalize(sqlite3_stmt **stmt);
        Sensor::Ptr parse_sensor(sqlite3_stmt* stmt);

        void check_auto_commit();
        void check_open_transaction();

        bfs::path _path;
        sqlite3 *_db;
        bool _auto_commit;
        Transaction::Ptr _transaction;
        sqlite3_stmt* _insert_sensor_stmt;
        sqlite3_stmt* _remove_sensor_stmt;
        sqlite3_stmt* _update_sensor_stmt;
        sqlite3_stmt* _select_sensor_stmt;
        sqlite3_stmt* _select_sensor_by_external_id_stmt;
        sqlite3_stmt* _select_sensor_by_name_stmt;
        sqlite3_stmt* _select_sensors_stmt;
        sqlite3_stmt* _select_all_sensor_uuids_stmt;
        std::map<const std::string, sqlite3_stmt*> _statements;
    };
};

#endif /* LIBKLIO_SQLITE3_SQLITE3STORE_HPP */
