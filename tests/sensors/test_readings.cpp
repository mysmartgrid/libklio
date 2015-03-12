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

#include <iostream>
#include <map>
#include <boost/uuid/uuid_io.hpp>
#include <boost/test/unit_test.hpp>
#include <libklio/config.h>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

klio::StoreFactory::Ptr store_factory = klio::StoreFactory::Ptr(new klio::StoreFactory());
klio::SensorFactory::Ptr sensor_factory = klio::SensorFactory::Ptr(new klio::SensorFactory());

klio::SQLite3Store::Ptr create_sqlite3_test_store(const bfs::path& path) {

    std::cout << "Attempt to create SQLite3Store " << path << std::endl;
    klio::SQLite3Store::Ptr store = store_factory->create_sqlite3_store(path);
    std::cout << "Created " << store->str() << std::endl;
    return store;
}

klio::SQLite3Store::Ptr create_sqlite3_test_store(
        const bfs::path& path,
        const bool prepare,
        const bool auto_commit,
        const bool auto_flush,
        const long flush_timeout,
        const std::string& synchronous) {

    std::cout << "Attempt to create SQLite3Store " << path << std::endl;
    klio::SQLite3Store::Ptr store = store_factory->create_sqlite3_store(
            path,
            prepare,
            auto_commit,
            auto_flush,
            flush_timeout,
            synchronous);

    std::cout << "Created " << store->str() << std::endl;
    return store;
}

klio::Sensor::Ptr create_test_sensor(
        const std::string& external_id,
        const std::string& name,
        const std::string& unit) {

    klio::Sensor::Ptr sensor(sensor_factory->createSensor(external_id, name, unit, "Europe/Berlin"));
    std::cout << "Created " << sensor->str() << std::endl;
    return sensor;
}

BOOST_AUTO_TEST_CASE(check_add_retrieve_reading) {

    try {
        std::cout << std::endl << "Testing - Adding & retrieving a reading to/from a sensor." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(
                TEST_DB1_FILE,
                true,
                true,
                true,
                0,
                klio::SQLite3Store::OS_SYNC_OFF);

        try {
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            double value = 23;

            try {
                //Non existent sensor
                store->add_reading(sensor, timestamp, value);

                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            store->add_reading(sensor, timestamp, value);

            // now, retrieve it and check.
            klio::readings_t_Ptr readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(1, readings->size());

            std::map<klio::timestamp_t, double>::iterator it;
            for (it = readings->begin(); it != readings->end(); it++) {
                BOOST_CHECK_EQUAL(timestamp, (*it).first);
                BOOST_CHECK_EQUAL(value, (*it).second);
            }

            klio::reading_t retrieved = store->get_reading(sensor, timestamp);

            BOOST_CHECK_EQUAL(timestamp, retrieved.first);
            BOOST_CHECK_EQUAL(value, retrieved.second);

            //Duplicated reading with auto flush does not raise exception
            store->add_reading(sensor, timestamp, value);

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_reading_timeframe) {

    try {
        std::cout << std::endl << "Testing - Adding & retrieving a timeframe of readings to/from a sensor." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(TEST_DB1_FILE);

        try {
            klio::timestamp_t marker_begin = 1393418290;
            klio::timestamp_t marker_end = 1393418296;

            try {
                //Non existent sensor
                store->get_timeframe_readings(sensor, marker_begin, marker_end);

                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            // Create a pattern in the database: lets have 23....42....23....
            for (klio::timestamp_t ts = 1393418246; ts < marker_begin; ts += 1) {
                store->add_reading(sensor, ts, 23);
            }
            // five times 42 in the middle!
            for (klio::timestamp_t ts = marker_begin; ts < marker_end; ts += 1) {
                store->add_reading(sensor, ts, 42);
            }
            for (klio::timestamp_t ts = marker_end; ts < 1393418396; ts += 1) {
                store->add_reading(sensor, ts, 23);
            }

            // Now check if we can retrieve the 42s.
            klio::readings_t_Ptr readings = store->get_timeframe_readings(sensor, marker_begin, marker_end);

            uint32_t result_counter = 0;
            std::map<klio::timestamp_t, double>::iterator it;
            for (it = readings->begin(); it != readings->end(); it++) {
                double value = (*it).second;
                BOOST_CHECK(42 == value || 23 == value);
                result_counter++;
            }
            BOOST_CHECK_EQUAL(7, result_counter);
            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_retrieve_last_reading) {

    try {
        std::cout << std::endl << "Testing - Retrieving the last reading from a sensor in SQLite3." << std::endl;
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(TEST_DB1_FILE);

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");

            try {
                //Non existent sensor
                store->get_last_reading(sensor);

                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp1 = tc->get_timestamp();
            klio::timestamp_t timestamp2 = timestamp1 - 100;
            double value = 23;
            store->add_reading(sensor, timestamp1, value);
            store->add_reading(sensor, timestamp2, value);
            std::cout << "Inserted two readings of value " << value << ", "
                    "using timestamps " << timestamp1 << " and " << timestamp2 << std::endl;

            // now, retrieve it and check.
            klio::reading_t last_reading = store->get_last_reading(sensor);
            BOOST_CHECK_EQUAL(timestamp1, last_reading.first);
            BOOST_REQUIRE(timestamp2 != last_reading.first);
            BOOST_CHECK_EQUAL(value, last_reading.second);

            store->dispose();

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            store->dispose();
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_bulk_insert) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings in SQLite3." << std::endl;
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(TEST_DB1_FILE);

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");

            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            klio::readings_t readings;
            size_t num_readings = 100;
            for (size_t i = 0; i < num_readings; i++) {
                klio::reading_t foo(timestamp--, 23);
                readings.insert(foo);
            }

            try {
                //Non existent sensor
                store->add_readings(sensor, readings);

                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

klio::TXTStore::Ptr create_txt_test_store(const bfs::path& path) {

    std::cout << "Attempt to create TXTStore " << path << std::endl;
    klio::TXTStore::Ptr store = store_factory->create_txt_store(path);
    std::cout << "Created " << store->str() << std::endl;
    return store;
}

BOOST_AUTO_TEST_CASE(check_txt_bulk_insert) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings in TXT." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::TXTStore::Ptr store = create_txt_test_store(TEST_DB1_FILE);

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 100;
            for (size_t i = 0; i < num_readings; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#ifdef ENABLE_ROCKSDB

klio::RocksDBStore::Ptr create_rocksdb_test_store(const bfs::path& path) {

    std::cout << "Attempt to create RocksDBStore " << path << std::endl;
    klio::RocksDBStore::Ptr store = store_factory->create_rocksdb_store(path);
    std::cout << "Created " << store->str() << std::endl;
    return store;
}

BOOST_AUTO_TEST_CASE(check_roksdb_bulk_insert) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings in RocksDB." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::RocksDBStore::Ptr store = create_rocksdb_test_store(TEST_DB1_FILE);

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 100;
            for (size_t i = 0; i < num_readings; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#endif /* ENABLE_ROCKSDB */

#ifdef ENABLE_REDIS3M

klio::RedisStore::Ptr create_redis_test_store(const std::string& host, const unsigned int port, const unsigned int db) {

    std::cout << "Attempting to create Redis store " << std::endl;
    klio::RedisStore::Ptr store = store_factory->create_redis_store(host, port, db);
    std::cout << "Created: " << store->str() << std::endl;
    return store;
}

klio::RedisStore::Ptr create_redis_test_store() {

    return create_redis_test_store(
            klio::RedisStore::DEFAULT_REDIS_HOST,
            klio::RedisStore::DEFAULT_REDIS_PORT,
            klio::RedisStore::DEFAULT_REDIS_DB);
}

BOOST_AUTO_TEST_CASE(check_redis_bulk_insert) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings in RocksDB." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::RedisStore::Ptr store = create_redis_test_store();

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 100;
            for (size_t i = 0; i < num_readings; i++) {

                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#endif /* ENABLE_REDIS3M */

#ifdef ENABLE_POSTGRESQL

klio::PostgreSQLStore::Ptr create_postgresql_test_store(const std::string& info) {

    std::cout << "Attempting to create PostgreSQL store " << std::endl;
    klio::PostgreSQLStore::Ptr store = store_factory->create_postgresql_store(info);
    std::cout << "Created: " << store->str() << std::endl;
    return store;
}

klio::PostgreSQLStore::Ptr create_postgresql_test_store() {

    return create_postgresql_test_store(klio::PostgreSQLStore::DEFAULT_CONNECTION_INFO);
}

BOOST_AUTO_TEST_CASE(check_postgresql_bulk_insert) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings in PostgreSQL." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::PostgreSQLStore::Ptr store = create_postgresql_test_store();

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 100;
            for (size_t i = 0; i < num_readings; i++) {

                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#endif /* ENABLE_POSTGRESQL */

BOOST_AUTO_TEST_CASE(check_sqlite3_bulk_insert_duplicates) {

    try {
        std::cout << std::endl << "Testing - The bulk-insertion of readings with duplicates in SQLite3." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(TEST_DB1_FILE);

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // Insert some readings.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t start_time = tc->get_timestamp();

            klio::readings_t readings;
            size_t num_readings = 10;

            for (size_t i = 0; i < num_readings; i++) {

                klio::timestamp_t timestamp = start_time - i;
                klio::reading_t foo(timestamp, 23);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            // Now, generate some readings with overlapping timestamps.
            readings.clear();
            int num_overlapping = num_readings / 2;
            for (size_t i = 0; i < num_readings; i++) {

                klio::timestamp_t timestamp = start_time - i + num_overlapping;
                klio::reading_t foo(timestamp, 42);
                readings.insert(foo);
            }

            std::cout << "Inserting " << readings.size() << " readings with "
                    << num_overlapping << "overlapping timestamps." << std::endl;

            store->update_readings(sensor, readings);

            // Now, retrieve them and check.
            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings + num_overlapping, loaded_readings->size());

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_num_readings) {

    try {
        std::cout << std::endl << "Testing - Getting the number of readings in SQLite3." << std::endl;
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");
        klio::SQLite3Store::Ptr store = create_sqlite3_test_store(TEST_DB1_FILE);

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 10;

            for (size_t i = 0; i < num_readings; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings." << std::endl;
            store->add_readings(sensor, readings);

            // now, retrieve it and check.
            size_t saved_readings = store->get_num_readings(sensor);
            std::cout << "Store contains " << saved_readings << " readings." << std::endl;

            BOOST_CHECK_EQUAL(num_readings, saved_readings);

            store->dispose();

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sync_readings) {

    try {
        std::cout << std::endl << "Testing - Readings synchronization." << std::endl;
        klio::SQLite3Store::Ptr storeA = create_sqlite3_test_store(TEST_DB1_FILE);
        klio::SQLite3Store::Ptr storeB = create_sqlite3_test_store(TEST_DB2_FILE);

        klio::Sensor::Ptr sensor1 = create_test_sensor("sensor1", "sensor1", "Watt");
        klio::Sensor::Ptr sensor2 = create_test_sensor("sensor2", "sensor2", "Watt");
        klio::Sensor::Ptr sensor3 = create_test_sensor("sensor3", "sensor3", "Watt");
        klio::Sensor::Ptr sensor4 = create_test_sensor("sensor4", "sensor4", "Watt");

        try {
            storeA->add_sensor(sensor1);
            storeB->sync(storeA);
            std::vector<klio::Sensor::uuid_t> uuids = storeB->get_sensor_uuids();
            BOOST_CHECK_EQUAL(1, uuids.size());

            storeA->add_sensor(sensor2);
            storeA->add_sensor(sensor3);
            storeB->add_sensor(sensor4);

            //Add readings to sensor 1 and 3 at store A
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings;
            size_t num_readings = 10;

            klio::timestamp_t timestamp = tc->get_timestamp();
            for (size_t i = 0; i < num_readings; i++) {
                klio::reading_t foo(timestamp--, 23);
                readings.insert(foo);
            }
            std::cout << "Inserting " << readings.size() << " readings to sensor 1 and 3." << std::endl;
            storeA->add_readings(sensor1, readings);
            storeA->add_readings(sensor3, readings);

            std::cout << "Synchronize sensor1 readings." << std::endl;
            storeB->sync_readings(sensor1, storeA);
            std::cout << "Synchronized." << std::endl;

            klio::readings_t sync_readings = *storeB->get_all_readings(sensor1);
            BOOST_CHECK_EQUAL(readings.size(), sync_readings.size());

            std::map<klio::timestamp_t, double>::iterator it1;
            std::map<klio::timestamp_t, double>::iterator it2 = readings.begin();

            for (it1 = sync_readings.begin(); it1 != sync_readings.end(); ++it1) {

                klio::timestamp_t ts1 = (*it1).first;
                klio::timestamp_t ts2 = (*it2).first;
                double val1 = (*it1).second;
                double val2 = (*it2).second;
                ++it2;

                BOOST_CHECK_EQUAL(ts1, ts2);
                BOOST_CHECK_EQUAL(val1, val2);
            }

            //Repeat synchronization
            storeB->sync_readings(sensor1, storeA);
            sync_readings = *storeB->get_all_readings(sensor1);
            BOOST_CHECK_EQUAL(readings.size(), sync_readings.size());

            //Sensor 2 has no readings
            storeB->sync_readings(sensor2, storeA);
            sync_readings = *storeB->get_all_readings(sensor2);
            BOOST_CHECK_EQUAL(0, sync_readings.size());

            //Sensor 3 does not exist in store B
            storeB->sync_readings(sensor3, storeA);

            std::vector<klio::Sensor::Ptr> sensors = storeB->get_sensors_by_external_id(sensor3->external_id());

            for (std::vector<klio::Sensor::Ptr>::const_iterator it = sensors.begin(); it != sensors.end(); ++it) {

                sensor3 = *it;
                sync_readings = *storeB->get_all_readings(sensor3);
                BOOST_CHECK_EQUAL(readings.size(), sync_readings.size());
            }

            klio::Sensor::Ptr changed3 = sensor_factory->createSensor(sensor3->uuid(), sensor3->external_id(), "Changed Name", "Changed Description", "kWh", "Europe/Paris", klio::DeviceType::DRIER);
            storeA->update_sensor(changed3);

            storeB->sync_readings(changed3, storeA);

            sync_readings = *storeB->get_all_readings(changed3);
            BOOST_CHECK_EQUAL(readings.size(), sync_readings.size());

            sensors = storeB->get_sensors_by_external_id(sensor3->external_id());
            BOOST_CHECK_EQUAL(1, sensors.size());

            for (std::vector<klio::Sensor::Ptr>::const_iterator it = sensors.begin(); it != sensors.end(); ++it) {

                klio::Sensor::Ptr found = (*it);
                BOOST_CHECK_EQUAL(found->uuid(), sensor3->uuid());
                BOOST_CHECK_EQUAL(found->external_id(), sensor3->external_id());
                BOOST_CHECK_EQUAL(found->name(), "Changed Name");
                BOOST_CHECK_EQUAL(found->description(), "Changed Description");
                BOOST_CHECK_EQUAL(found->unit(), "kWh");
                BOOST_CHECK_EQUAL(found->timezone(), "Europe/Paris");
                BOOST_CHECK_EQUAL(found->device_type(), klio::DeviceType::DRIER);
            }

            //Same external id, different uuids
            klio::Sensor::Ptr copy4(sensor_factory->createSensor("sensor4", "copy of sensor 4", "kWh", "Europe/Paris"));
            std::cout << "Created " << copy4->str() << std::endl;
            storeA->add_sensor(copy4);
            storeA->add_readings(copy4, readings);

            storeB->sync_readings(copy4, storeA);
            sync_readings = *storeB->get_all_readings(sensor4);
            BOOST_CHECK_EQUAL(readings.size(), sync_readings.size());

            klio::Sensor::Ptr found = storeB->get_sensor(sensor4->uuid());
            BOOST_CHECK_EQUAL(found->uuid(), sensor4->uuid());
            BOOST_CHECK_EQUAL(found->external_id(), sensor4->external_id());
            BOOST_CHECK_EQUAL(found->name(), copy4->name());
            BOOST_CHECK_EQUAL(found->description(), found->description());
            BOOST_CHECK_EQUAL(found->unit(), found->unit());
            BOOST_CHECK_EQUAL(found->timezone(), found->timezone());

            // cleanup
            storeA->dispose();
            storeB->dispose();

        } catch (klio::StoreException const& ex) {
            // cleanup
            storeA->dispose();
            storeB->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sync_store) {

    try {
        std::cout << std::endl << "Testing - Store synchronization." << std::endl;
        klio::SQLite3Store::Ptr source1 = create_sqlite3_test_store(TEST_DB1_FILE);
        klio::SQLite3Store::Ptr source2 = create_sqlite3_test_store(TEST_DB2_FILE);
        klio::SQLite3Store::Ptr target = create_sqlite3_test_store(TEST_DB3_FILE);
        klio::Sensor::Ptr sensor1 = create_test_sensor("same_external_id", "sensor1", "Watt");
        klio::Sensor::Ptr sensor2 = create_test_sensor("same_external_id", "sensor2", "Watt");

        try {
            source1->add_sensor(sensor1);
            source2->add_sensor(sensor2);

            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings1;
            klio::readings_t readings2;

            klio::timestamp_t timestamp = 1400000000;
            for (size_t i = 0; i < 10; i++) {
                klio::reading_t foo1(timestamp, 888);
                klio::reading_t foo2(timestamp + 5, 999);
                readings1.insert(foo1);
                readings2.insert(foo2);
                timestamp++;
            }
            source1->add_readings(sensor1, readings1);
            source2->add_readings(sensor2, readings2);

            BOOST_CHECK_EQUAL(10, source1->get_num_readings(sensor1));
            BOOST_CHECK_EQUAL(10, source2->get_num_readings(sensor2));

            target->sync(source1);
            target->sync(source2);

            std::vector<klio::Sensor::uuid_t> uuids = target->get_sensor_uuids();
            BOOST_CHECK_EQUAL(1, uuids.size());

            for (std::vector<klio::Sensor::uuid_t>::const_iterator uuid = uuids.begin(); uuid != uuids.end(); ++uuid) {

                klio::Sensor::Ptr found = target->get_sensor(*uuid);
                BOOST_CHECK_EQUAL(found->external_id(), sensor2->external_id());
                BOOST_CHECK_EQUAL(found->name(), sensor2->name());
                BOOST_CHECK_EQUAL(found->description(), sensor2->description());
                BOOST_CHECK_EQUAL(found->unit(), sensor2->unit());
                BOOST_CHECK_EQUAL(found->timezone(), sensor2->timezone());

                klio::readings_t readings = *target->get_all_readings(found);
                BOOST_CHECK_EQUAL(15, readings.size());
            }

            // cleanup
            source1->dispose();
            source2->dispose();
            target->dispose();

        } catch (klio::StoreException const& ex) {
            // cleanup
            source1->dispose();
            source2->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#ifdef ENABLE_MSG

klio::MSGStore::Ptr create_msg_test_store(std::string id) {

    return store_factory->create_msg_store(
            "https://dev3-api.mysmartgrid.de:8443",
            id, id, "libklio test store", "libklio");
}

BOOST_AUTO_TEST_CASE(check_add_retrieve_single_readings_msg) {

    try {
        std::cout << "Testing - Add/retrieve a reading to/from MSGStore (Watt)" << std::endl;
        klio::Store::Ptr store = create_msg_test_store("72c160748bcf890bdb7cc1281032adcb");
        klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "Watt");

        try {
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());

            klio::timestamp_t timestamp = time(0) - 3000;
            timestamp -= timestamp % 60;
            double counter = 1000;

            try {
                //Non existent sensor
                store->add_reading(sensor, timestamp, counter);

                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert first reading
            store->add_reading(sensor, timestamp, counter);

            klio::readings_t_Ptr readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(0, readings->size());

            // insert second reading
            timestamp += 60;
            counter += 1000;
            store->add_reading(sensor, timestamp, counter);

            readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(1, readings->size());

            // insert third reading
            timestamp += 60;
            counter += 1000;
            store->add_reading(sensor, timestamp, counter);

            readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(2, readings->size());

            klio::reading_t retrieved = store->get_reading(sensor, timestamp);

            BOOST_CHECK_EQUAL(timestamp, retrieved.first);
            BOOST_CHECK_EQUAL(17, round(retrieved.second));

            store->dispose();

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::StoreException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_watt_readings_msg) {

    try {
        std::cout << "Testing - Add readings to MSGStore (Watt)" << std::endl;
        klio::Store::Ptr store = create_msg_test_store("72c160748bcf890bdb7cc1281032adcb");

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "watt");
            store->add_sensor(sensor);

            klio::timestamp_t timestamp = time(0) - 3000;
            timestamp -= timestamp % 60;

            for (int i = 0; i < 12; i++) {
                store->add_reading(sensor, timestamp + (i * 60), i * 1000);
            }

            klio::readings_t readings = *store->get_all_readings(sensor);

            store->dispose();

            BOOST_CHECK_EQUAL(11, readings.size());

            int i = 1;
            for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                BOOST_CHECK_EQUAL(timestamp + (i++ * 60), (*it).first);
                BOOST_CHECK_EQUAL(17, round((*it).second));
            }

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_kwh_readings_msg) {

    try {
        std::cout << "Testing - Add readings to MSGStore (kWh)" << std::endl;
        klio::Store::Ptr store = create_msg_test_store("28c180728bcf890bdb7cc1281038ad01");

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "kwh");
            store->add_sensor(sensor);

            klio::timestamp_t timestamp = time(0) - 3000;
            timestamp -= timestamp % 60;

            klio::readings_t readings;
            for (int i = 0; i < 12; i++) {
                klio::reading_t reading(timestamp + (i * 60), i * 1000);
                readings.insert(reading);
            }
            store->add_readings(sensor, readings);

            readings = *store->get_all_readings(sensor);
            store->dispose();

            BOOST_CHECK_EQUAL(11, readings.size());

            int i = 1;
            for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                BOOST_CHECK_EQUAL(timestamp + (i++ * 60), (*it).first);
                BOOST_CHECK_EQUAL(17, round((*it).second));
            }

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_celsius_readings_msg) {

    try {
        std::cout << "Testing - Add readings to MSGStore (°C)" << std::endl;
        klio::Store::Ptr store = create_msg_test_store("21c180742bcf888bdb7cc1221038ad01");

        try {
            double value = 26.7938;

            do {
                klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "degC");
                store->add_sensor(sensor);

                klio::timestamp_t timestamp = time(0) - 3000;
                timestamp -= timestamp % 60;

                klio::readings_t readings;
                for (int i = 0; i < 12; i++) {
                    klio::reading_t reading(timestamp + (i * 60), value);
                    readings.insert(reading);
                }
                store->add_readings(sensor, readings);

                readings = *store->get_all_readings(sensor);

                BOOST_CHECK_EQUAL(11, readings.size());

                int i = 1;
                for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                    BOOST_CHECK_EQUAL(timestamp + (i++ * 60), (*it).first);
                    BOOST_CHECK_EQUAL(value, (*it).second);
                }

                store->remove_sensor(sensor);
                value -= 26.7938;

            } while (value > -27);

            store->dispose();

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_hsbs_readings_msg) {

    try {
        std::cout << "Testing - Add readings to MSGStore (°C)" << std::endl;
        klio::Store::Ptr store = create_msg_test_store("21c180742bcf888bdb7cc1221038adcb");

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "_hsbs");
            store->add_sensor(sensor);

            klio::Sensor::Ptr found = store->get_sensor(sensor->uuid());
            BOOST_CHECK_EQUAL(found->external_id(), sensor->external_id());
            BOOST_CHECK_EQUAL(found->name(), sensor->name());
            BOOST_CHECK_EQUAL(found->description(), sensor->description());
            BOOST_CHECK_EQUAL(found->unit(), sensor->unit());
            BOOST_CHECK_EQUAL(found->timezone(), sensor->timezone());

            klio::timestamp_t timestamp = time(0);
            timestamp -= timestamp % 60;
            klio::readings_t readings;
            double value = 30;

            for (int i = 0; i < 72; i++) {
                klio::reading_t reading(timestamp - (i * 20), value);
                readings.insert(reading);
            }
            store->add_readings(sensor, readings);

            store->dispose();

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_msg_get_all_readings) {

    try {
        std::cout << std::endl << "Testing - Retrieving all readings from a MSGStore." << std::endl;
        klio::Store::Ptr store = create_msg_test_store("252525de-3ecd-f3d3-24db-3e96755d2424");

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "watt");
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            //No measurement yet
            klio::readings_t_Ptr readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(0, readings->size());

            klio::timestamp_t last_timestamp = time(0) - 3000;
            last_timestamp -= last_timestamp % 60;
            double counter = 1000;

            for (int i = 0; i < 12; i++) {
                store->add_reading(sensor, last_timestamp, counter);
                counter += 1000;
                last_timestamp += 60;
            }

            readings = store->get_all_readings(sensor);
            BOOST_CHECK_EQUAL(11, readings->size());

            store->dispose();

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            store->dispose();
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_msg_retrieve_last_reading) {

    try {
        std::cout << std::endl << "Testing - Retrieving the last reading from a MSGStore." << std::endl;
        klio::Store::Ptr store = create_msg_test_store("2424f4de-3ecd-f3d3-24db-3e96755d2424");

        try {
            klio::Sensor::Ptr sensor = create_test_sensor("sensor", "sensor", "watt");
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            //No measurement yet
            klio::reading_t last_reading = store->get_last_reading(sensor);

            BOOST_CHECK_EQUAL(0, last_reading.first);
            BOOST_CHECK_EQUAL(0, round(last_reading.second));

            klio::timestamp_t last_timestamp = time(0) - 3000;
            last_timestamp -= last_timestamp % 60;
            double counter = 1000;

            for (int i = 0; i < 12; i++) {
                store->add_reading(sensor, last_timestamp, counter);
                counter += 1000;
                last_timestamp += 60;
            }
            //Add last reading
            store->add_reading(sensor, last_timestamp, counter * 2);

            last_reading = store->get_last_reading(sensor);

            BOOST_CHECK_EQUAL(last_timestamp, last_reading.first);
            BOOST_CHECK_EQUAL(233, round(last_reading.second));

            store->dispose();

        } catch (klio::CommunicationException const& ce) {
            //Ignore this kind of exception

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            store->dispose();
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#endif /* ENABLE_MSG */

//BOOST_AUTO_TEST_SUITE_END()
