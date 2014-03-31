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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <libklio/config.h>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

BOOST_AUTO_TEST_CASE(check_add_retrieve_reading) {

    try {
        std::cout << std::endl << "*** Adding & retrieving a reading to/from a sensor." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            double reading = 23;
            store->add_reading(sensor, timestamp, reading);

            // now, retrieve it and check.
            klio::readings_t_Ptr readings = store->get_all_readings(sensor);

            BOOST_CHECK_EQUAL(1, readings->size());

            std::map<klio::timestamp_t, double>::iterator it;
            for (it = readings->begin(); it != readings->end(); it++) {
                klio::timestamp_t ts1 = (*it).first;
                double val1 = (*it).second;
                std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
                BOOST_CHECK_EQUAL(timestamp, ts1);
                BOOST_CHECK_EQUAL(reading, val1);
            }

            klio::reading_t retrieved = store->get_reading(sensor, timestamp);

            BOOST_CHECK_EQUAL(timestamp, retrieved.first);
            BOOST_CHECK_EQUAL(reading, retrieved.second);

            // cleanup
            store->remove_sensor(sensor);

        } catch (klio::StoreException const& ex) {
            //store->remove_sensor(sensor);
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_reading_timeframe) {

    try {
        std::cout << std::endl << "*** Adding & retrieving a timeframe of readings to/from a sensor." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            klio::timestamp_t marker_begin = 1393418290;
            klio::timestamp_t marker_end = 1393418296;
            // insert a reading.
            // Create a pattern in the database: lets have 23....42....23....
            for(klio::timestamp_t ts = 1393418246; ts < marker_begin; ts += 1) {
              store->add_reading(sensor, ts, 23);
            }
            // five times 42 in the middle!
            for(klio::timestamp_t ts = marker_begin; ts < marker_end; ts += 1) {
              store->add_reading(sensor, ts, 42);
            }
            for(klio::timestamp_t ts = marker_end; ts < 1393418396; ts += 1) {
              store->add_reading(sensor, ts, 23);
            }

            // Now check if we can retrieve the 42s.
            klio::readings_t_Ptr readings = store->get_timeframe_readings(sensor, marker_begin, marker_end);

            // cleanup
            store->remove_sensor(sensor);

            uint32_t result_counter = 0;
            std::map<klio::timestamp_t, double>::iterator it;
            for (it = readings->begin(); it != readings->end(); it++) {
              klio::timestamp_t ts1 = (*it).first;
              double val1 = (*it).second;
              std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
              BOOST_CHECK(42 == val1 || 23 == val1);
              result_counter++;
            }
            BOOST_CHECK_EQUAL(7, result_counter);

        } catch (klio::StoreException const& ex) {
          //store->remove_sensor(sensor);
          std::cout << "Caught invalid exception: " << ex.what() << std::endl;
          BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
      BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_last_reading) {

    try {
        std::cout << std::endl << "*** retrieving the last reading from a sensor." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor);
            std::cout << "added to store: " << sensor->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            klio::timestamp_t timestamp2 = (tc->get_timestamp() - 100);
            double reading = 23;
            store->add_reading(sensor, timestamp, reading);
            store->add_reading(sensor, timestamp2, reading);
            std::cout << "Inserted two readings of " << reading << "value, "
                    "using timestamps " << timestamp << " and " << timestamp2 << std::endl;

            // now, retrieve it and check.
            klio::reading_t last_reading = store->get_last_reading(sensor);
            klio::timestamp_t ts1 = last_reading.first;
            double val1 = last_reading.second;
            std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;

            // cleanup
            store->remove_sensor(sensor);

            BOOST_CHECK_EQUAL(timestamp, ts1);
            BOOST_REQUIRE(timestamp2 != ts1);
            BOOST_CHECK_EQUAL(reading, val1);

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor1);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_bulk_insert) {

    try {
        std::cout << std::endl << "*** bulk-inserting some readings." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

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
            boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();

            store->add_readings(sensor, readings);

            boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration diff = t2 - t1;
            std::cout << "Bulk insert duration for SQLite: " << diff.total_milliseconds() << " ms" << std::endl;

            std::cout << "Loading " << readings.size() << " readings." << std::endl;
            t1 = boost::posix_time::microsec_clock::local_time();

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);

            t2 = boost::posix_time::microsec_clock::local_time();
            diff = t2 - t1;
            std::cout << "Bulk load duration for SQLite: " << diff.total_milliseconds() << " ms" << std::endl;
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            // cleanup
            store->remove_sensor(sensor);

            klio::readings_cit_t it;
            size_t ret_size = 0;
            for (it = loaded_readings->begin(); it != loaded_readings->end(); ++it) {
                klio::timestamp_t ts1 = (*it).first;
                double val1 = (*it).second;
                std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
                ret_size++;
            }
            BOOST_CHECK_EQUAL(num_readings, ret_size);

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#ifdef ENABLE_ROCKSDB

BOOST_AUTO_TEST_CASE(check_roksdb_bulk_insert) {

    try {
        std::cout << std::endl << "*** bulk-inserting some readings." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_PATH);
        klio::Store::Ptr store(store_factory->create_rocksdb_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
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
            boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();

            store->add_readings(sensor, readings);

            boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration diff = t2 - t1;
            std::cout << "Bulk insert duration for RocksDB: " << diff.total_milliseconds() << " ms" << std::endl;

            std::cout << "Loading " << readings.size() << " readings." << std::endl;
            t1 = boost::posix_time::microsec_clock::local_time();

            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor);

            t2 = boost::posix_time::microsec_clock::local_time();
            diff = t2 - t1;
            std::cout << "Bulk load duration for RocksDB: " << diff.total_milliseconds() << " ms" << std::endl;
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            // cleanup
            store->remove_sensor(sensor);

            klio::readings_cit_t it;
            size_t ret_size = 0;
            for (it = loaded_readings->begin(); it != loaded_readings->end(); ++it) {
                klio::timestamp_t ts1 = (*it).first;
                double val1 = (*it).second;
                std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
                ret_size++;
            }
            BOOST_CHECK_EQUAL(num_readings, ret_size);

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#endif /* ENABLE_ROCKSDB */


BOOST_AUTO_TEST_CASE(check_bulk_insert_duplicates) {

    try {
        std::cout << std::endl << "*** bulk-inserting readings with duplicates." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);

        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

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

            // cleanup
            store->remove_sensor(sensor);

            klio::readings_cit_t it;
            size_t ret_size = 0;
            for (it = loaded_readings->begin(); it != loaded_readings->end(); ++it) {
                klio::timestamp_t ts1 = (*it).first;
                double val1 = (*it).second;
                std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
                ret_size++;
            }
            BOOST_CHECK_EQUAL(num_readings + num_overlapping, ret_size);

        } catch (klio::StoreException const& ex) {
            //store->remove_sensor(sensor);
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_num_readings) {

    try {
        std::cout << std::endl << "*** checking number of readings." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store(store_factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

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

            // cleanup
            store->remove_sensor(sensor);

            BOOST_CHECK_EQUAL(num_readings, saved_readings);

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sync_readings) {

    try {
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());

        bfs::path db1(TEST_DB1_FILE);
        klio::Store::Ptr storeA(store_factory->create_sqlite3_store(db1));
        std::cout << "Created: " << storeA->str() << std::endl;

        bfs::path db2(TEST_DB2_FILE);
        klio::Store::Ptr storeB(store_factory->create_sqlite3_store(db2));
        std::cout << "Created: " << storeB->str() << std::endl;

        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor2->str() << std::endl;

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor("sensor3", "sensor3", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor3->str() << std::endl;

        klio::Sensor::Ptr sensor4(sensor_factory->createSensor("sensor4", "sensor4", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor4->str() << std::endl;

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

            for (size_t i = 0; i < num_readings; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                double reading = 23;
                klio::reading_t foo(timestamp, reading);
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
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_sync_store) {

    try {
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());

        bfs::path db1(TEST_DB1_FILE);
        klio::Store::Ptr source1(store_factory->create_sqlite3_store(db1));
        std::cout << "Created: " << source1->str() << std::endl;

        bfs::path db2(TEST_DB2_FILE);
        klio::Store::Ptr source2(store_factory->create_sqlite3_store(db2));
        std::cout << "Created: " << source2->str() << std::endl;

        bfs::path db4(TEST_DB4_FILE);
        klio::Store::Ptr target(store_factory->create_sqlite3_store(db4));
        std::cout << "Created: " << target->str() << std::endl;

        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("same_external_id", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("same_external_id", "sensor2", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor2->str() << std::endl;

        try {
            source1->add_sensor(sensor1);
            source2->add_sensor(sensor2);

            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::readings_t readings1;
            klio::readings_t readings2;

            for (size_t i = 0; i < 10; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i;
                klio::reading_t foo1(timestamp, 123);
                klio::reading_t foo2(timestamp + 5, 321);
                readings1.insert(foo1);
                readings2.insert(foo2);
            }
            source1->add_readings(sensor1, readings1);
            source2->add_readings(sensor2, readings2);

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
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

#ifdef ENABLE_MSG

BOOST_AUTO_TEST_CASE(check_add_watt_reading_msg) {

    try {
        std::cout << "Testing add_reading for MSG (Watt)" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url,
                "72c160748bcf890bdb7cc1281032adcb",
                "72c160748bcf890bdb7cc1281038adcb",
                "libklio test",
                "libklio"));

        std::cout << "Created: " << store->str() << std::endl;

        try {
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "72c160748bcf890bdb7cc1281032adcb",
                    "Sensor1",
                    "Test",
                    "description",
                    "watt",
                    "Europe/Berlin"));

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

BOOST_AUTO_TEST_CASE(check_add_kwh_reading_msg) {

    try {
        std::cout << "Testing add_reading for MSG (kWh)" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url,
                "28c180728bcf890bdb7cc1281038adcb",
                "28c180728bcf890bdb7cc1281038adcb",
                "libklio test",
                "libklio"));

        std::cout << "Created: " << store->str() << std::endl;

        try {
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "28c18072-8bcf-890b-db7c-c1281038adcb",
                    "Sensor1",
                    "Test",
                    "description",
                    "kwh",
                    "Europe/Berlin"));

            store->add_sensor(sensor);

            klio::timestamp_t timestamp = time(0);
            timestamp -= timestamp % 60;
            klio::readings_t readings;

            for (int i = 0; i < 72; i++) {
                klio::reading_t reading(timestamp - (i * 20), 2.113237 - 0.00023 * (1 + i));
                readings.insert(reading);
            }
            store->add_readings(sensor, readings);

            readings = *store->get_all_readings(sensor);
            store->dispose();

            BOOST_CHECK(readings.size() == 23 || readings.size() == 24);

            int i = 23;
            for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                BOOST_CHECK_EQUAL(timestamp - (i-- * 60), (*it).first);
                BOOST_CHECK_EQUAL(0.0000115, (*it).second);
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

BOOST_AUTO_TEST_CASE(check_add_celsius_reading_msg) {

    try {
        std::cout << "Testing add_reading for MSG (°C)" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url,
                "21c180742bcf888bdb7cc1221038adcb",
                "21c180742bcf888bdb7cc1221038adcb",
                "libklio test",
                "libklio"));

        std::cout << "Created: " << store->str() << std::endl;

        try {
            double value = 26.7938;

            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
            do {
                klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                        "21c18074-2bcf-888b-db7c-c1221038adcb",
                        "Sensor1",
                        "Test",
                        "description",
                        "degC",
                        "Europe/Berlin"));

                store->add_sensor(sensor);

                klio::timestamp_t timestamp = time(0);
                timestamp -= timestamp % 60;
                klio::readings_t readings;

                for (int i = 0; i < 72; i++) {
                    klio::reading_t reading(timestamp - (i * 20), value);
                    readings.insert(reading);
                }
                store->add_readings(sensor, readings);

                readings = *store->get_all_readings(sensor);

                BOOST_CHECK(readings.size() == 23 || readings.size() == 24);

                int i = 23;
                for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                    BOOST_CHECK_EQUAL(timestamp - (i-- * 60), (*it).first);
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

BOOST_AUTO_TEST_CASE(check_add_hsbs_reading_msg) {

    try {
        std::cout << "Testing add_reading for MSG (°C)" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url,
                "21c180742bcf888bdb7cc1221038adcb",
                "21c180742bcf888bdb7cc1221038adcb",
                "libklio test",
                "libklio"));

        std::cout << "Created: " << store->str() << std::endl;

        try {
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "12121274-2bcf-888b-db7c-c1221038adcb",
                    "Sensor12",
                    "Test",
                    "description",
                    "_hsbs",
                    "Europe/Berlin"));

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

#endif /* ENABLE_MSG */

//BOOST_AUTO_TEST_SUITE_END()
