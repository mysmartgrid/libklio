/**
 * This class represents a local store, implemented as a RocksDB database.
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

#ifndef LIBKLIO_ROCKSDB_ROCKSDBSTORE_HPP
#define LIBKLIO_ROCKSDB_ROCKSDBSTORE_HPP 1

#include <libklio/config.h>

#ifdef ENABLE_ROCKSDB

#include <boost/filesystem.hpp>
#include <rocksdb/db.h>
#include <libklio/store.hpp>


namespace bfs = boost::filesystem;

namespace klio {

    class RocksDBStore : public Store {
    public:
        typedef boost::shared_ptr<RocksDBStore> Ptr;

        RocksDBStore(const bfs::path& path,
                const bool auto_flush,
                const timestamp_t flush_timeout,
                const bool synchronous,
                const std::map<const std::string, const std::string>& db_options,
                const std::map<const std::string, const std::string>& read_options) :
        Store(true, auto_flush, flush_timeout, 10, 10000),
        _path(path),
        _synchronous(synchronous),
        _db_options(db_options),
        _read_options(read_options) {

            if (_synchronous) {
                _write_options.sync = "true";
                _write_options.disableWAL = "false";
            } else {
                _write_options.sync = "false";
                _write_options.disableWAL = "true";
            }
        };

        virtual ~RocksDBStore() {
            close();
        };

        void open();
        void close();
        void check_integrity();
        void initialize();
        void dispose();
        const std::string str();

    protected:
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
        RocksDBStore(const RocksDBStore& original);
        RocksDBStore& operator =(const RocksDBStore& rhs);

        bfs::path _path;
        bool _synchronous;
        std::map<const std::string, const std::string> _db_options;
        std::map<const std::string, const std::string> _read_options;
        rocksdb::WriteOptions _write_options;
        std::map<const std::string, rocksdb::DB*> _db_buffer;

        rocksdb::DB* open_db(const bool create_if_missing, const bool error_if_exists, const std::string& db_path);
        void close_db(const std::string& db_path);
        void remove_db(const std::string& db_path);

        void put_sensor(const bool create, const Sensor::Ptr sensor);
        void put_value(rocksdb::DB* db, const std::string& key, const std::string& value);
        std::string get_value(rocksdb::DB* db, const std::string& key);
        void delete_value(rocksdb::DB* db, const std::string& key);
        void write_batch(rocksdb::DB* db, rocksdb::WriteBatch& batch);
        Sensor::Ptr load_sensor(const Sensor::uuid_t& uuid);

        const std::string compose_db_path();
        const std::string compose_sensors_path();
        const std::string compose_sensor_path(const std::string& uuid);
        const std::string compose_sensor_properties_path(const std::string& uuid);
        const std::string compose_sensor_readings_path(const std::string& uuid);
        void create_directory(const std::string& dir);
    };
};

#endif /* ENABLE_ROCKSDB */

#endif /* LIBKLIO_ROCKSDB_ROCKSDBSTORE_HPP */
