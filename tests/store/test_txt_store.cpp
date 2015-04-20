/**
 * This file is part of libklio.
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

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

/**
 * see http://www.boost.org/doc/libs/1_43_0/libs/test/doc/html/tutorials/hello-the-testing-world.html
 */

BOOST_AUTO_TEST_CASE(check_open_corrupt_txt_path) {

    std::cout << "Testing storage creation for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());

    bfs::path db = boost::filesystem::unique_path();

    try {
        std::cout << "Attempting to create " << db << std::endl;
        klio::Store::Ptr store(store_factory->open_txt_store(db));

        store->dispose();

        BOOST_FAIL("An exception is expected to be risen when a corrupt store is opened.");

    } catch (klio::StoreException const& ex) {
        //This exception is expected
        bfs::remove(db); //TODO: use dispose() here
    }
}

BOOST_AUTO_TEST_CASE(check_create_txt_storage) {

    std::cout << "Testing storage creation for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Store::Ptr loaded(store_factory->create_txt_store(db));
        loaded->open();
        std::cout << "Opened database: " << loaded->str() << std::endl;

        BOOST_CHECK(bfs::exists(db));
        
        store->dispose();
        
        BOOST_CHECK(!bfs::exists(db));

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_add_txt_sensor) {

    std::cout << "Testing sensor addition for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "89c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "this is a sensor description",
                "kwh",
                "Europe/Berlin"));

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
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_update_txt_sensor) {

    std::cout << "Testing sensor update for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "92c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor);

        klio::Sensor::Ptr changed(sensor_factory->createSensor(
                "92c18074-8bcf-240b-db7c-c1281038adcb",
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
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_txt_sensor) {

    std::cout << "Testing sensor removal for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "89c18074-8bcf-890b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "description",
                "watt",
                "Europe/Berlin"));

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
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_txt_sensor) {

    std::cout << "Testing sensor query by uuid for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "98c18074-8bcf-890b-db7c-c1281038adcb",
                "GetTest",
                "GetTest",
                "GetDescription",
                "watt",
                "Europe/Berlin"));

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
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_txt_sensor_by_name) {

    std::cout << "Testing sensor query by name for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

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
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_txt_sensors_by_external_id) {

    std::cout << "Testing sensor query by external id for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

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
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());

        sensors = store->get_sensors_by_external_id("External Id 3");
        BOOST_CHECK_EQUAL(0, sensors.size());

        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_txt_sensor_uuids) {

    std::cout << "Testing sensor uuids query for TXT" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

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
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_add_retrieve_txt_reading) {

    std::cout << std::endl << "Adding & retrieving a reading to/from a sensor." << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
        store->add_sensor(sensor);
        std::cout << "added to store: " << sensor->str() << std::endl;

        // insert a reading.
        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        klio::timestamp_t timestamp = tc->get_timestamp();
        double reading = 23;
        store->add_reading(sensor, timestamp, reading);

        klio::readings_t_Ptr readings = store->get_all_readings(sensor);

        std::map<klio::timestamp_t, double>::iterator it;
        for (it = readings->begin(); it != readings->end(); it++) {

            klio::timestamp_t ts = (*it).first;
            double val = (*it).second;
            std::cout << "Got timestamp " << ts << " -> value " << val << std::endl;

            BOOST_CHECK_EQUAL(timestamp, ts);
            BOOST_CHECK_EQUAL(reading, val);
        }

        // cleanup
        store->dispose();

    } catch (std::exception const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected store exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_txt_num_readings) {

    std::cout << std::endl << "Checking number of readings." << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB_PATH);

    std::cout << "Attempting to create " << db << std::endl;
    klio::Store::Ptr store(store_factory->create_txt_store(db));
    std::cout << "Created: " << store->str() << std::endl;

    try {
        klio::Sensor::Ptr sensor(sensor_factory->createSensor("sensor", "sensor", "Watt", "Europe/Berlin"));
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

//BOOST_AUTO_TEST_SUITE_END()
