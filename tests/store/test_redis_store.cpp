/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
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

#include <libklio/config.h>

#ifdef ENABLE_REDIS3M

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

klio::StoreFactory::Ptr store_factory = klio::StoreFactory::Ptr(new klio::StoreFactory());
klio::SensorFactory::Ptr sensor_factory = klio::SensorFactory::Ptr(new klio::SensorFactory());

klio::RedisStore::Ptr create_redis_test_store() {

    std::cout << "Attempting to create Redis store " << std::endl;
    klio::RedisStore::Ptr store = store_factory->create_redis_store();
    std::cout << "Created: " << store->str() << std::endl;
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

BOOST_AUTO_TEST_CASE(check_create_redis_storage) {

    std::cout << "Testing storage creation for Redis" << std::endl;
    try {
        klio::RedisStore::Ptr store = create_redis_test_store();

        BOOST_CHECK_EQUAL(store->host(), klio::RedisStore::DEFAULT_REDIS_HOST);
        BOOST_CHECK_EQUAL(store->port(), klio::RedisStore::DEFAULT_REDIS_PORT);
        BOOST_CHECK_EQUAL(store->db(), klio::RedisStore::DEFAULT_REDIS_DB);

        store->dispose();

    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during RedisStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_redis_sensor) {

    std::cout << "Testing sensor addition for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("aaaa", "Test libklio", "kwh");
        store->add_sensor(sensor);

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

        BOOST_CHECK_EQUAL(sensor->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor->timezone(), retrieved->timezone());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_update_redis_sensor) {

    std::cout << "Testing sensor update for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("Test", "Test update", "kwh");
        store->add_sensor(sensor);

        klio::Sensor::Ptr changed(sensor_factory->createSensor(
                sensor->uuid_string(),
                "Test",
                "Test libklio",
                "changed description",
                "watt",
                "Europe/Berlin"));

        store->update_sensor(changed);

        klio::Sensor::Ptr retrieved = store->get_sensor(changed->uuid());

        BOOST_CHECK_EQUAL(changed->name(), retrieved->name());
        BOOST_CHECK_EQUAL(changed->description(), retrieved->description());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_redis_sensor) {

    std::cout << "Testing sensor removal for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("cccc", "Test libklio", "kwh");
        store->add_sensor(sensor);

        store->remove_sensor(sensor);

        try {
            klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

            BOOST_FAIL("An exception must be raised if the sensor is not found.");

        } catch (klio::StoreException const& ok) {
            //This exception is expected
        }
        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_get_redis_sensor) {

    std::cout << "Testing sensor query by uuid for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("dddd", "Test libklio", "kwh");
        store->add_sensor(sensor);

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

        BOOST_CHECK_EQUAL(sensor->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor->description(), retrieved->description());

        store->remove_sensor(sensor);

        try {
            //Non existent
            store->get_sensor(sensor->uuid());

            BOOST_FAIL("An exception must be raised if the sensor is not found.");

        } catch (klio::StoreException const& ok) {
            //This exception is expected
        }

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_get_redis_sensor_by_name) {

    std::cout << "Testing sensor query by name for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                "98c18074-8bcf-890b-db7c-c1281038adcb",
                "Unique External Id",
                "Unique Name",
                "Unique Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                "88c18074-890b-8bcf-db7c-c1281038adcb",
                "External Id 1",
                "Duplicated Name",
                "Duplicated Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor2);

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
                "99c18074-890b-8bcf-db7c-c1281038adcb",
                "External Id 2",
                "Duplicated Name",
                "Duplicated Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor3);

        std::vector<klio::Sensor::Ptr> duplicated = store->get_sensors_by_name("Duplicated Name");
        std::vector<klio::Sensor::Ptr> unique = store->get_sensors_by_name("Unique Name");

        BOOST_CHECK_EQUAL(2, duplicated.size());
        BOOST_CHECK_EQUAL(1, unique.size());

        std::vector<klio::Sensor::Ptr>::iterator it = unique.begin();
        klio::Sensor::Ptr retrieved = (*it);

        BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_get_redis_sensors_by_external_id) {

    std::cout << "Testing sensor query by external id for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                "82c18074-8bcf-890b-db7c-c1281038adcb",
                "External Id 1",
                "Sensor 1",
                "Description 1",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                "74c18074-890b-8bcf-db7c-c1281038adcb",
                "External Id 2",
                "Sensor 2",
                "Description 2",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor2);

        std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id("External Id 1");

        BOOST_CHECK_EQUAL(1, sensors.size());

        std::vector<klio::Sensor::Ptr>::iterator it = sensors.begin();
        klio::Sensor::Ptr retrieved = (*it);

        BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());

        sensors = store->get_sensors_by_external_id("External Id 3");
        BOOST_CHECK_EQUAL(0, sensors.size());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_get_redis_sensor_uuids) {

    std::cout << "Testing sensor uuids query for Redis" << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                "98c17480-8bcf-890b-db7c-c1081038adcb",
                "TestA",
                "TestA",
                "DescriptionA",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                "88c17480-890b-8bcf-db7c-c1181038adcb",
                "TestB",
                "TestB",
                "DescriptionB",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor2);

        std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();

        BOOST_CHECK_EQUAL(2, uuids.size());

        std::vector<klio::Sensor::uuid_t>::iterator it = uuids.begin();
        klio::Sensor::uuid_t uuid2 = (*it++);
        klio::Sensor::uuid_t uuid1 = (*it);

        BOOST_CHECK(uuid1 != uuid2);
        BOOST_CHECK(uuid1 == sensor1->uuid() || uuid1 == sensor2->uuid());
        BOOST_CHECK(uuid2 == sensor2->uuid() || uuid2 == sensor1->uuid());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred");
    }
}

BOOST_AUTO_TEST_CASE(check_add_retrieve_redis_reading) {

    std::cout << std::endl << "Adding & retrieving a reading to/from a sensor." << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("eeee", "Test libklio", "watt");
        store->add_sensor(sensor);
        std::cout << "added to store: " << sensor->str() << std::endl;

        // insert a reading.
        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        klio::timestamp_t timestamp = tc->get_timestamp();
        double reading = 23;
        store->add_reading(sensor, timestamp, reading);

        klio::readings_t_Ptr readings = store->get_all_readings(sensor);

        BOOST_CHECK_EQUAL(1, readings->size());

        std::map<klio::timestamp_t, double>::iterator it;
        for (it = readings->begin(); it != readings->end(); it++) {

            klio::timestamp_t ts = (*it).first;
            double val = (*it).second;
            std::cout << "Got timestamp " << ts << " -> value " << val << std::endl;

            BOOST_CHECK_EQUAL(timestamp, ts);
            BOOST_CHECK_EQUAL(reading, val);
        }

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected store exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_redis_num_readings) {

    std::cout << std::endl << "Checking number of readings." << std::endl;
    klio::RedisStore::Ptr store = create_redis_test_store();

    try {
        klio::Sensor::Ptr sensor = create_test_sensor("ffff", "Test libklio", "watt");
        store->add_sensor(sensor);
        std::cout << "added to store: " << sensor->str() << std::endl;

        // insert a reading.
        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        klio::readings_t readings;
        size_t num_readings = 10;

        klio::timestamp_t start = tc->get_timestamp();
        for (size_t i = 0; i < num_readings; i++) {
            klio::timestamp_t timestamp = start - i;
            double reading = 23;
            klio::reading_t foo(timestamp, reading);
            readings.insert(foo);
        }
        std::cout << "Inserting " << readings.size() << " readings." << std::endl;
        store->add_readings(sensor, readings);

        size_t saved_readings = store->get_num_readings(sensor);
        std::cout << "Store contains " << saved_readings << " readings." << std::endl;

        store->dispose();

        BOOST_CHECK_EQUAL(num_readings, saved_readings);

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected store exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_redis_store_creation_performance) {

    try {
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        klio::Store::Ptr store;

        boost::posix_time::time_duration elapsed_time;
        double seconds = 0;
        boost::posix_time::ptime time_before;
        boost::posix_time::ptime time_after;

        std::cout.precision(7);
        std::cout << std::fixed;

        try {
            std::cout << std::endl << "Performance Test" << std::endl;
            std::cout << "Performance Test - RedisStore" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();

            store = store_factory->create_redis_store(
                    klio::RedisStore::DEFAULT_REDIS_HOST,
                    klio::RedisStore::DEFAULT_REDIS_PORT,
                    klio::RedisStore::DEFAULT_REDIS_DB,
                    false,
                    false,
                    0);

            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Create store :                              "
                    << seconds << " s" << std::endl;

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

void run_redis_store_performance_tests(const bool auto_commit, const bool auto_flush, const long flush_timeout, const size_t num_readings) {

    try {
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        klio::Store::Ptr store;

        boost::posix_time::time_duration elapsed_time;
        double seconds = 0;
        boost::posix_time::ptime time_before;
        boost::posix_time::ptime time_after;

        std::cout.precision(7);
        std::cout << std::fixed;

        try {
            std::cout << std::endl << "Performance Test" << std::endl;
            std::cout << std::endl << "Performance Test - RedisStore - " <<
                    " auto commit: " << (auto_commit ? "true" : "false") <<
                    ", auto flushing: " << (auto_flush ? "true" : "false") << std::endl;

            store = store_factory->create_redis_store(
                    klio::RedisStore::DEFAULT_REDIS_HOST,
                    klio::RedisStore::DEFAULT_REDIS_PORT,
                    klio::RedisStore::DEFAULT_REDIS_DB,
                    auto_commit,
                    auto_flush,
                    flush_timeout);

            if (!auto_commit) {
                store->start_transaction();
            }

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_sensor(sensor1);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Add 1st sensor:                             "
                    << seconds << " s" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_sensor(sensor2);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Add 2nd sensor:                             "
                    << seconds << " s" << std::endl;

            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp = tc->get_timestamp() - 1000;
            double reading = 23;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_reading(sensor1, timestamp, reading);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Add 1st reading:                            "
                    << seconds << " s" << std::endl;

            timestamp -= 3000;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_reading(sensor1, timestamp, reading);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Add 2nd reading:                            "
                    << seconds << " s" << std::endl;

            klio::readings_t readings;
            for (size_t i = 0; i < num_readings; i++) {
                timestamp = tc->get_timestamp() - i;
                reading = 23;
                klio::reading_t foo(timestamp, reading);
                readings.insert(foo);
            }

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_readings(sensor1, readings);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Add " << num_readings << " readings:                          "
                    << seconds << " s" << std::endl;

            if (!auto_flush) {
                time_before = boost::posix_time::microsec_clock::local_time();
                store->flush();
                time_after = boost::posix_time::microsec_clock::local_time();

                elapsed_time = time_after - time_before;
                seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
                std::cout << "Performance Test - RedisStore - " <<
                        "Flushing " << num_readings << " readings:                     "
                        << seconds << " s" << std::endl;
            }

            if (!auto_commit) {
                time_before = boost::posix_time::microsec_clock::local_time();
                store->commit_transaction();
                time_after = boost::posix_time::microsec_clock::local_time();

                elapsed_time = time_after - time_before;
                seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
                std::cout << "Performance Test - RedisStore - " <<
                        "Committing " << num_readings << " readings:                   "
                        << seconds << " s" << std::endl;
            }

            time_before = boost::posix_time::microsec_clock::local_time();
            store->get_sensors_by_external_id(sensor1->external_id());
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Get sensors by external id:                 "
                    << seconds << " s" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->get_all_readings(sensor1);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - RedisStore - " <<
                    "Get " << num_readings << " readings:                          "
                    << seconds << " s" << std::endl;

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

BOOST_AUTO_TEST_CASE(check_redis_store_performance) {

    run_redis_store_performance_tests(true, true, 0, 1000);
    run_redis_store_performance_tests(true, false, 0, 1000);
    run_redis_store_performance_tests(false, true, 0, 1000);
    run_redis_store_performance_tests(false, false, 0, 1000);
}

#endif /* ENABLE_REDIS3M */

//BOOST_AUTO_TEST_SUITE_END()
