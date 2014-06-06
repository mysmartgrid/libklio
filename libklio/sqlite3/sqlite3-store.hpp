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

        SQLite3Store(const bfs::path& path) :
        Store(0),
        _path(path),
        _db(NULL),
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

        void add_sensor(const Sensor::Ptr sensor);
        void remove_sensor(const Sensor::Ptr sensor);
        void update_sensor(const Sensor::Ptr sensor);
        std::vector<Sensor::Ptr> get_sensors();

        readings_t_Ptr get_all_readings(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_readings(const Sensor::Ptr sensor, timestamp_t begin, timestamp_t end);
        unsigned long int get_num_readings(const Sensor::Ptr sensor);
        reading_t get_last_reading(const Sensor::Ptr sensor);

    protected:
        void flush(const Sensor::Ptr sensor);

    private:
        SQLite3Store(const SQLite3Store& original);
        SQLite3Store& operator =(const SQLite3Store& rhs);

        bool has_table(const std::string& name);
        bool has_column(const std::string& table, const std::string& column);
        void insert_reading_record(sqlite3_stmt* stmt, timestamp_t timestamp, double value);
        readings_t_Ptr get_readings(sqlite3_stmt* stmt);
        sqlite3_stmt *prepare(const std::string& stmt_str);
        sqlite3_stmt *get_statement(const std::string& sql);
        int execute(sqlite3_stmt *stmt, int expected_code);
        void reset(sqlite3_stmt *stmt);
        void finalize(sqlite3_stmt **stmt);
        Sensor::Ptr parse_sensor(sqlite3_stmt* stmt);

        bfs::path _path;
        sqlite3 *_db;
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
