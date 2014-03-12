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
#include <boost/test/unit_test.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <libklio/store.hpp>
#include <libklio/sqlite3/sqlite3-store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>

/**
 * see http://www.boost.org/doc/libs/1_43_0/libs/test/doc/html/tutorials/hello-the-testing-world.html
 */

BOOST_AUTO_TEST_CASE(check_sanity) {
    try {
        std::cout << "Demo test case: Checking world sanity." << std::endl;
        BOOST_CHECK_EQUAL(42, 42);
        BOOST_CHECK(23 != 42); // #1 continues on error
        BOOST_REQUIRE(23 != 42); // #2 throws on error

    } catch (std::exception const & ex) {
        BOOST_ERROR(ex.what());
    }
    if (23 == 42) {
        BOOST_FAIL("23 == 42, oh noes"); // #4 throws on error
    }
}

BOOST_AUTO_TEST_CASE(check_create_sqlite3_storage) {

    std::cout << "Testing storage creation for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

        klio::Store::Ptr loaded(store_factory->open_sqlite3_store(db));
        std::cout << "Opened database: " << loaded->str() << std::endl;

        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_open_close_sqlite3_storage) {

    std::cout << "Testing opening and closing SQLite3 store" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

        store->open();
        store->open();
        
        store->close();
        store->close();

        store->open();

        store->close();
        store->open();

        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_open_corrupt_sqlite3_file) {

    std::cout << "Testing storage creation for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());

    bfs::path db = boost::filesystem::unique_path();

    boost::iostreams::stream_buffer<boost::iostreams::file_sink> buf(db.string());
    std::ostream out(&buf);
    out << "";

    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        klio::Store::Ptr store(store_factory->open_sqlite3_store(db));

        store->dispose();

        BOOST_FAIL("An exception is expected to be risen when a corrupt store is opened.");

    } catch (klio::StoreException const& ex) {
        //This exception is expected
        bfs::remove(db);//TODO: use dispose() here
    }
}

BOOST_AUTO_TEST_CASE(check_add_sqlite3_sensor) {

    std::cout << "Testing sensor addition for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_update_sqlite3_sensor) {

    std::cout << "Testing sensor update for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_sqlite3_sensor) {

    std::cout << "Testing sensor removal for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_sqlite3_sensor) {

    std::cout << "Testing sensor query by uuid for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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
            store->dispose();

            BOOST_FAIL("An exception must be raised if the sensor is not found.");

        } catch (klio::StoreException const& ok) {
            //This exception is expected
        }

        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_sqlite3_sensor_by_name) {

    std::cout << "Testing sensor query by name for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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
                "Duplicated External Id",
                "Duplicated Name",
                "Duplicated Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor2);

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
                "99c18074-890b-8bcf-db7c-c1281038adcb",
                "Duplicated External Id",
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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_sqlite3_sensors_by_external_id) {

    std::cout << "Testing sensor query by external id for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_sqlite3_sensor_uuids) {

    std::cout << "Testing sensor uuids query for SQLite3" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db);
        std::cout << "Created database: " << store->str() << std::endl;

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

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

//BOOST_AUTO_TEST_SUITE_END()
