/**
 * This class represents a local store, implemented as a SQlite 3 database.
 *
 * (c) Fraunhofer ITWM - Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2014
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

#ifndef LIBKLIO_POSTGRESQL_POSTGRESQLSTORE_HPP
#define LIBKLIO_POSTGRESQL_POSTGRESQLSTORE_HPP 1

#include <libklio/config.h>

#ifdef ENABLE_POSTGRESQL

#include <stdio.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <libklio/store.hpp>
#include <libklio/sensor-factory.hpp>
#include <postgresql/libpq-fe.h>
#include "postgresql-transaction.hpp"


namespace bfs = boost::filesystem;

namespace klio {

    class PostgreSQLStore : public Store {
    public:
        typedef boost::shared_ptr<PostgreSQLStore> Ptr;

        PostgreSQLStore(
                const std::string& info,
                const bool auto_commit,
                const bool auto_flush,
                const timestamp_t flush_timeout
                ) :
        Store(auto_commit, auto_flush, flush_timeout),
        _info(info),
        _connection(NULL) {
        };

        virtual ~PostgreSQLStore() {
            close();
        };

        const std::string info() const {
            return _info;
        };

        void open();
        void close();
        void check_integrity();
        void initialize();
        void upgrade();
        void prepare();
        void dispose();
        const std::string str();

    protected:
        Transaction::Ptr get_transaction_handler();

        void add_sensor_record(const Sensor::Ptr sensor);
        void remove_sensor_record(const Sensor::Ptr sensor);
        void update_sensor_record(const Sensor::Ptr sensor);
        void add_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);
        void update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);

        std::vector<Sensor::Ptr> get_sensor_records();
        readings_t_Ptr get_all_reading_records(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end);
        unsigned long int get_num_readings_value(const Sensor::Ptr sensor);
        reading_t get_last_reading_record(const Sensor::Ptr sensor);
        reading_t get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp);

    private:
        PostgreSQLStore(const PostgreSQLStore& original);
        PostgreSQLStore& operator =(const PostgreSQLStore& rhs);

        void open_db();
        void close_db();
        void prepare_statements();
        void prepare_statement(const char* statement_name, const char* statement, const int num_params);
        void finalize_statements();
        void clear_connection_refferences();
        PostgreSQLTransaction::Ptr create_transaction_handler();

        void add_reading_records(const char* statement_name, const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors);
        readings_t_Ptr get_reading_records(const char* statement_name, const char* params[], const int num_params);

        void execute(const char* statement, const char* params[], const int num_params);
        void execute_prepared(const char* statement_name, const char* params[], const int num_params);
        PGresult* execute_prepared(const char* statement_name, const char* params[], const int num_params, const ExecStatusType expected_return);
        void check(PGresult* result, const ExecStatusType expected_status);
        void clear(PGresult* result);

        Sensor::Ptr parse_sensor(PGresult* result, const int row);
        reading_t parse_reading(PGresult* result, const int row);
        std::string get_string_value(PGresult* result, const int row, const int col);
        unsigned long int get_long_value(PGresult* result, const int row, const int col);
        double get_double_value(PGresult* result, const int row, const int col);

        std::string _info;
        PGconn* _connection;
        PostgreSQLTransaction::Ptr _transaction;

        static const char* CREATE_SENSORS_TABLE_SQL;
        static const char* CREATE_READINGS_TABLE_SQL;
        static const char* DROP_SENSORS_TABLE_SQL;
        static const char* DROP_READINGS_TABLE_SQL;
        static const char* DEALLOCATE_ALL_SQL;

        static const char* INSERT_SENSOR_SQL;
        static const char* DELETE_SENSOR_SQL;
        static const char* UPDATE_SENSOR_SQL;
        static const char* SELECT_SENSORS_SQL;

        static const char* INSERT_READING_SQL;
        static const char* UPDATE_READING_SQL;
        static const char* SELECT_READINGS_SQL;
        static const char* SELECT_TIMEFRAME_READINGS_SQL;
        static const char* COUNT_READINGS_SQL;
        static const char* SELECT_LAST_READING_SQL;
        static const char* SELECT_READING_SQL;

        static const char* INSERT_SENSOR_STMT;
        static const char* DELETE_SENSOR_STMT;
        static const char* UPDATE_SENSOR_STMT;
        static const char* SELECT_SENSORS_STMT;

        static const char* INSERT_READING_STMT;
        static const char* UPDATE_READING_STMT;
        static const char* SELECT_READINGS_STMT;
        static const char* SELECT_TIMEFRAME_READINGS_STMT;
        static const char* COUNT_READINGS_STMT;
        static const char* SELECT_LAST_READING_STMT;
        static const char* SELECT_READING_STMT;
    };
};

#endif /* ENABLE_POSTGRESQL */

#endif /* LIBKLIO_POSTGRESQL_POSTGRESQLSTORE_HPP */