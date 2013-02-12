/**
 * This file is part of libklio.
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
#include <libklio/common.hpp>
#include <libklio/store.hpp>


namespace bfs = boost::filesystem;

namespace klio {

    class SQLite3Store : public Store {
    public:
        typedef std::tr1::shared_ptr<SQLite3Store> Ptr;

        SQLite3Store(const bfs::path& path) :
        _path(path) {
        };

        virtual ~SQLite3Store() {
            close();
        };

        void open();
        void initialize();
        void close();
        const std::string str();

        virtual void add_sensor(klio::Sensor::Ptr sensor);
        virtual void remove_sensor(const klio::Sensor::Ptr sensor);
        virtual void update_sensor(const klio::Sensor::Ptr sensor);
        virtual klio::Sensor::Ptr get_sensor(const klio::Sensor::uuid_t& uuid);
        virtual std::vector<klio::Sensor::Ptr> get_sensors_by_name(const std::string& sensor_id);
        virtual std::vector<klio::Sensor::uuid_t> get_sensor_uuids();

        virtual void add_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp, double value);
        virtual void add_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
        virtual void update_readings(klio::Sensor::Ptr sensor, const readings_t& readings);
        virtual readings_t_Ptr get_all_readings(klio::Sensor::Ptr sensor);
        virtual unsigned long int get_num_readings(klio::Sensor::Ptr sensor);
        virtual std::pair<timestamp_t, double> get_last_reading(klio::Sensor::Ptr sensor);

    private:
        SQLite3Store(const SQLite3Store& original);
        SQLite3Store& operator =(const SQLite3Store& rhs);

        void checkSensorTable();
        bool has_table(std::string name);

        sqlite3_stmt *prepare(const std::string stmt_str);
        int execute(sqlite3_stmt *stmt, int expected_code);
        void reset(sqlite3_stmt *stmt);
        void finalize(sqlite3_stmt *stmt);
        int execute(std::string stmt, int (*callback)(void*, int, char**, char**), void *arg);

        sqlite3 *db;
        bfs::path _path;
    };
};

#endif /* LIBKLIO_SQLITE3_SQLITE3STORE_HPP */
