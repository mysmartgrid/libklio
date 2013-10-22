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
#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

BOOST_AUTO_TEST_CASE(check_add_retrieve_reading) {

    try {
        std::cout << std::endl << "*** Adding & retrieving a reading to/from a sensor." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_FILE);
        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
            store->add_sensor(sensor1);
            std::cout << "added to store: " << sensor1->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            double reading = 23;
            store->add_reading(sensor1, timestamp, reading);

            // now, retrieve it and check.
            klio::readings_t_Ptr readings = store->get_all_readings(sensor1);

            // cleanup
            store->remove_sensor(sensor1);

            std::map<klio::timestamp_t, double>::iterator it;
            for (it = readings->begin(); it != readings->end(); it++) {
                klio::timestamp_t ts1 = (*it).first;
                double val1 = (*it).second;
                std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
                BOOST_CHECK_EQUAL(timestamp, ts1);
                BOOST_CHECK_EQUAL(reading, val1);
            }

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor1);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_retrieve_last_reading) {
    try {
        std::cout << std::endl << "*** retrieving the last reading from a sensor." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_FILE);
        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
            store->add_sensor(sensor1);
            std::cout << "added to store: " << sensor1->str() << std::endl;

            // insert a reading.
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp();
            klio::timestamp_t timestamp2 = (tc->get_timestamp() - 100);
            double reading = 23;
            store->add_reading(sensor1, timestamp, reading);
            store->add_reading(sensor1, timestamp2, reading);
            std::cout << "Inserted two readings of " << reading << "value, "
                    "using timestamps " << timestamp << " and " << timestamp2 << std::endl;

            // now, retrieve it and check.
            klio::reading_t last_reading = store->get_last_reading(sensor1);
            klio::timestamp_t ts1 = last_reading.first;
            double val1 = last_reading.second;
            std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;

            // cleanup
            store->remove_sensor(sensor1);

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
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_bulk_insert) {

    try {
        std::cout << std::endl << "*** bulk-inserting some readings." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_FILE);
        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
            store->add_sensor(sensor1);
            std::cout << "added to store: " << sensor1->str() << std::endl;

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
            store->add_readings(sensor1, readings);

            // now, retrieve it and check.
            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor1);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            // cleanup
            store->remove_sensor(sensor1);

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
            //store->remove_sensor(sensor1);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_bulk_insert_duplicates) {
    try {
        std::cout << std::endl << "*** bulk-inserting readings with duplicates." << std::endl;

        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_FILE);

        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
            store->add_sensor(sensor1);
            std::cout << "added to store: " << sensor1->str() << std::endl;

            // Insert some readings.
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
            store->add_readings(sensor1, readings);

            // Now, generate some readings with overlapping timestamps.
            readings.clear();
            int num_overlapping = num_readings / 2;
            for (size_t i = 0; i < num_readings; i++) {
                klio::timestamp_t timestamp = tc->get_timestamp() - i + num_overlapping;
                double reading = 42;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }

            std::cout << "Inserting " << readings.size() << " readings with "
                    << num_overlapping << "overlapping timestamps." << std::endl;

            store->update_readings(sensor1, readings);

            // Now, retrieve them and check.
            klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor1);
            std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;

            // cleanup
            store->remove_sensor(sensor1);

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
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
            //store->remove_sensor(sensor1);
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_num_readings) {
    try {
        std::cout << std::endl << "*** checking number of readings." << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB_FILE);
        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->initialize();
            store->add_sensor(sensor1);
            std::cout << "added to store: " << sensor1->str() << std::endl;

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
            store->add_readings(sensor1, readings);
            // now, retrieve it and check.
            size_t saved_readings = store->get_num_readings(sensor1);
            std::cout << "Store contains " << saved_readings << " readings." << std::endl;

            // cleanup
            store->remove_sensor(sensor1);

            BOOST_CHECK_EQUAL(num_readings, saved_readings);

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_sync_readings) {

    try {
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());

        bfs::path db1(TEST_DB_FILE);
        klio::Store::Ptr storeA(store_factory->open_sqlite3_store(db1));
        std::cout << "Created: " << storeA->str() << std::endl;

        bfs::path db2(TEST_DB2_FILE);
        klio::Store::Ptr storeB(store_factory->open_sqlite3_store(db2));
        std::cout << "Created: " << storeB->str() << std::endl;

        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor1->str() << std::endl;

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor2->str() << std::endl;

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor("sensor3", "sensor3", "Watt", "Europe/Berlin"));
        std::cout << "Created " << sensor3->str() << std::endl;

        try {
            storeA->initialize();
            std::cout << "Initialized: " << storeA->str() << std::endl;

            storeB->initialize();
            std::cout << "Initialized: " << storeB->str() << std::endl;

            storeA->add_sensor(sensor1);
            storeA->add_sensor(sensor2);
            storeA->add_sensor(sensor3);

            storeB->add_sensor(sensor1);
            storeB->add_sensor(sensor2);

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
            sync_readings = *storeB->get_all_readings(sensor2);
            BOOST_CHECK_EQUAL(0, sync_readings.size());

            //Sensor 3 does not exist in store B
            try {
                storeB->sync_readings(sensor3, storeA);

                BOOST_FAIL("Synchronization involving non existing sensors is not allowed.");
            } catch (klio::StoreException const& ex) {
            }

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
    ;
    ;
}

BOOST_AUTO_TEST_CASE(check_add_watt_reading_msg) {

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
        store->open();
        store->initialize();

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_add_kwh_reading_msg) {

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
        store->open();
        store->initialize();

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

        BOOST_CHECK_EQUAL(24, readings.size());

        int i = 23;
        for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

            BOOST_CHECK_EQUAL(timestamp - (i-- * 60), (*it).first);
            BOOST_CHECK_EQUAL(0.0000115, (*it).second);
        }

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_add_celsius_reading_msg) {

    std::cout << "Testing add_reading for MSG (°C)" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    std::cout << "Attempting to create MSG store " << url << std::endl;
    klio::Store::Ptr store(factory->create_msg_store(url,
            "22c180742bcf890bdb7cc1281038adcb",
            "22c180742bcf890bdb7cc1281038adcb",
            "libklio test",
            "libklio"));

    std::cout << "Created: " << store->str() << std::endl;

    try {
        store->open();
        store->initialize();

        double value = 26.7938;

        do {
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "22c18074-2bcf-890b-db7c-c1281038adcb",
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

            BOOST_CHECK_EQUAL(24, readings.size());

            int i = 23;
            for (klio::readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

                BOOST_CHECK_EQUAL(timestamp - (i-- * 60), (*it).first);
                BOOST_CHECK_EQUAL(value, (*it).second);
            }

            store->remove_sensor(sensor);
            value -= 26.7938;

        } while (value > -27);
        
        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

//BOOST_AUTO_TEST_SUITE_END()
