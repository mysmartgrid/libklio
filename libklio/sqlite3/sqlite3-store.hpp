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

#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <libklio/store.hpp>
#include <libklio/sqlite3/sqlite3-transaction.hpp>


namespace bfs = boost::filesystem;

namespace klio {

    class SQLite3Store : public Store {
    public:
        typedef boost::shared_ptr<SQLite3Store> Ptr;

        SQLite3Store(
                const bfs::path& path,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const std::string& synchronous
                ) :
        Store(auto_commit, auto_flush, flush_timeout, 10, 10000),
        _path(path),
        _db(NULL),
        _synchronous(synchronous),
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
        void rotate(bfs::path to_path);
        const std::string str();

        static const std::string OS_SYNC_OFF;
        static const std::string OS_SYNC_NORMAL;
        static const std::string OS_SYNC_FULL;

    protected:
        Transaction::Ptr get_transaction_handler();

        void add_sensor_record(const Sensor::Ptr sensor);
        void remove_sensor_record(const Sensor::Ptr sensor);
        void update_sensor_record(const Sensor::Ptr sensor);
        void add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors);
        void add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);
        void update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);

        std::vector<Sensor::Ptr> get_sensor_records();
        readings_t_Ptr get_all_reading_records(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end);
        unsigned long int get_num_readings_value(const Sensor::Ptr sensor);
        reading_t get_last_reading_record(const Sensor::Ptr sensor);
        reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp);

        void clear_buffers();

    private:
        SQLite3Store(const SQLite3Store& original);
        SQLite3Store& operator =(const SQLite3Store& rhs);

        void open_db();
        void close_db();
        void prepare_statements();
        void finalize_statements();
        SQLite3Transaction::Ptr create_transaction_handler();

        const bool has_table(const std::string& name);
        const bool has_column(const std::string& table, const std::string& column);

        void add_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const std::string& operation, const bool ignore_errors);
        readings_t_Ptr get_readings_records(sqlite3_stmt* stmt);

        sqlite3_stmt *prepare(const std::string& stmt_str);
        sqlite3_stmt *get_statement(const std::string& sql);
        int execute(sqlite3_stmt *stmt, const int expected_code);
        void reset(sqlite3_stmt *stmt);
        void finalize(sqlite3_stmt **stmt);
        Sensor::Ptr parse_sensor(sqlite3_stmt* stmt);

        bfs::path _path;
        sqlite3 *_db;
        SQLite3Transaction::Ptr _transaction;
        std::string _synchronous;
        sqlite3_stmt* _insert_sensor_stmt;
        sqlite3_stmt* _remove_sensor_stmt;
        sqlite3_stmt* _update_sensor_stmt;
        sqlite3_stmt* _select_sensor_stmt;
        sqlite3_stmt* _select_sensor_by_external_id_stmt;
        sqlite3_stmt* _select_sensor_by_name_stmt;
        sqlite3_stmt* _select_sensors_stmt;
        sqlite3_stmt* _select_all_sensor_uuids_stmt;
        boost::unordered_map<const std::string, sqlite3_stmt*> _statements;
    };
};

#endif /* LIBKLIO_SQLITE3_SQLITE3STORE_HPP */
