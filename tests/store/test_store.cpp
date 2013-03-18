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

#define BOOST_TEST_MODULE store_test

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/store.hpp>
#include <libklio/msg/msg-store.hpp>
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

BOOST_AUTO_TEST_CASE(check_create_storage_sqlite3) {

    std::cout << "Testing open storage utility for SQLite3" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    bfs::path db(TEST_DB_FILE);
    try {
        std::cout << "Attempting to create " << db << std::endl;
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created database: " << store->str() << std::endl;
        store->initialize();
        store->close();

        klio::Store::Ptr loaded(factory->create_sqlite3_store(db));
        std::cout << "Opened database: " << loaded->str() << std::endl;
    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_create_storage_msg) {

    std::cout << "Testing create storage utility for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "d271f4de36cdf3d300db3e96755d8736", "d271f4de36cdf3d300db3e96755d8736"));
        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();

        store->dispose();

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_add_msg_sensor) {

    std::cout << "Testing add_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "89c180748bcf240bdb7cc1281038adcb", "89c180748bcf240bdb7cc1281038adcb"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                std::string("89c18074-8bcf-240b-db7c-c1281038adcb"),
                std::string("Test libklio"),
                std::string("description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor);

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

        store->dispose();

        BOOST_CHECK_EQUAL(sensor->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor->description(), retrieved->description());

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_update_msg_sensor) {

    std::cout << "Testing update_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "89c180748bcf240bdb7cc1281038adcb", "89c180748bcf240bdb7cc1281038adcb"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                std::string("89c18074-8bcf-240b-db7c-c1281038adcb"),
                std::string("Test libklio"),
                std::string("description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor);

        klio::Sensor::Ptr changed(sensor_factory->createSensor(
                std::string("89c18074-8bcf-240b-db7c-c1281038adcb"),
                std::string("Test libklio"),
                std::string("changed description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->update_sensor(changed);


        klio::Sensor::Ptr retrieved = store->get_sensor(changed->uuid());
        store->dispose();

        BOOST_CHECK_EQUAL(changed->name(), retrieved->name());
        BOOST_CHECK_EQUAL(changed->description(), retrieved->description());

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_msg_sensor) {

    std::cout << "Testing remove_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "d271f4de36cdf3d300db3e96755d8736", "d271f4de36cdf3d300db3e96755d8736"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                std::string("89c18074-8bcf-890b-db7c-c1281038adcb"),
                std::string("Test libklio"),
                std::string("description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor);

        store->remove_sensor(sensor);

        try {
            klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

            BOOST_FAIL("An exception must be raised if the sensor is not found.");

        } catch (klio::StoreException const& ok) {
            //This exception is expected
        }
        store->dispose();

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor) {

    std::cout << "Testing get_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "98c180748bcf890bdb7cc1281038adcb", "98c180748bcf890bdb7cc1281038adcb"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                std::string("98c18074-8bcf-890b-db7c-c1281038adcb"),
                std::string("GetTest"),
                std::string("GetDescription"),
                std::string("watt"),
                std::string("Europe/Berlin")));

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
        
    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor_by_name) {

    std::cout << "Testing get_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "98c180748bcf890bdb7cc1281038adcb", "98c180748bcf890bdb7cc1281038adcb"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                std::string("98c18074-8bcf-890b-db7c-c1281038adcb"),
                std::string("Unique"),
                std::string("Unique Description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                std::string("88c18074-890b-8bcf-db7c-c1281038adcb"),
                std::string("Duplicated"),
                std::string("Duplicated"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor2);

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
                std::string("99c18074-890b-8bcf-db7c-c1281038adcb"),
                std::string("Duplicated"),
                std::string("Duplicated"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor3);

        std::vector<klio::Sensor::Ptr> duplicated = store->get_sensors_by_name("Duplicated");
        std::vector<klio::Sensor::Ptr> unique = store->get_sensors_by_name("Unique");
        store->dispose();

        BOOST_CHECK_EQUAL(2, duplicated.size());
        BOOST_CHECK_EQUAL(1, unique.size());

        std::vector<klio::Sensor::Ptr>::iterator it = unique.begin();
        klio::Sensor::Ptr retrieved = (*it);

        BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor_uuids) {

    std::cout << "Testing get_sensor for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->create_msg_store(url, "22c180748bcf890bdb7cc1281038adcb", "98c180748bcf890bdb7cc1281038adcb"));

        std::cout << "Created: " << store->str() << std::endl;
        store->open();
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                std::string("98c18074-8bcf-890b-db7c-c1081038adcb"),
                std::string("TestA"),
                std::string("DescriptionA"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                std::string("88c18074-890b-8bcf-db7c-c1181038adcb"),
                std::string("TestB"),
                std::string("DescriptionB"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        store->add_sensor(sensor2);

        std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();

        store->dispose();

        BOOST_CHECK_EQUAL(2, uuids.size());

        std::vector<klio::Sensor::uuid_t>::iterator it = uuids.begin();
        klio::Sensor::uuid_t uuid2 = (*it++);
        klio::Sensor::uuid_t uuid1 = (*it);

        BOOST_CHECK_EQUAL(uuid1, sensor1->uuid());
        BOOST_CHECK_EQUAL(uuid2, sensor2->uuid());

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

//BOOST_AUTO_TEST_SUITE_END()
