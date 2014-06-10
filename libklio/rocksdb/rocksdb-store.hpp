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

#include <vector>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <rocksdb/db.h>
#include <libklio/store.hpp>


namespace bfs = boost::filesystem;

namespace klio {

    class RocksDBStore : public Store {
    public:
        typedef boost::shared_ptr<RocksDBStore> Ptr;

        RocksDBStore(const bfs::path& path,
                const std::map<const std::string, const std::string>& db_options,
                const std::map<const std::string, const std::string>& read_options,
                const std::map<const std::string, const std::string>& write_options) :
        Store(0),
        _path(path),
        _db_options(db_options),
        _read_options(read_options),
        _write_options(write_options) {
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

        void add_sensor(const Sensor::Ptr sensor);
        void remove_sensor(const Sensor::Ptr sensor);
        void update_sensor(const Sensor::Ptr sensor);
        Sensor::Ptr get_sensor(const Sensor::uuid_t& uuid);
        std::vector<Sensor::Ptr> get_sensors_by_external_id(const std::string& external_id);
        std::vector<Sensor::Ptr> get_sensors_by_name(const std::string& name);
        std::vector<Sensor::uuid_t> get_sensor_uuids();
        std::vector<Sensor::Ptr> get_sensors();

        void add_reading(const Sensor::Ptr sensor, timestamp_t timestamp, double value);
        void add_readings(const Sensor::Ptr sensor, const readings_t& readings);
        void update_readings(const Sensor::Ptr sensor, const readings_t& readings);

        readings_t_Ptr get_all_readings(const Sensor::Ptr sensor);
        readings_t_Ptr get_timeframe_readings(const Sensor::Ptr sensor, timestamp_t begin, timestamp_t end);
        unsigned long int get_num_readings(const Sensor::Ptr sensor);
        reading_t get_last_reading(const Sensor::Ptr sensor);
        reading_t get_reading(const Sensor::Ptr sensor, timestamp_t timestamp);

    protected:
        void flush(const Sensor::Ptr sensor);
        void clear_buffers();

    private:
        RocksDBStore(const RocksDBStore& original);
        RocksDBStore& operator =(const RocksDBStore& rhs);

        bfs::path _path;
        std::map<const std::string, const std::string> _db_options;
        std::map<const std::string, const std::string> _read_options;
        std::map<const std::string, const std::string> _write_options;
        std::map<const std::string, rocksdb::DB*> _db_buffer;
        
        rocksdb::DB* open_db(const bool create_if_missing, const bool error_if_exists, const std::string& db_path);
        void close_db(const std::string& db_path);
        void remove_db(const std::string& db_path);

        void put_sensor(const bool create, const Sensor::Ptr sensor);
        void put_value(rocksdb::DB* db, const std::string& key, const std::string& value);
        std::string get_value(rocksdb::DB* db, const std::string& key);
        void delete_value(rocksdb::DB* db, const std::string& key);

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
