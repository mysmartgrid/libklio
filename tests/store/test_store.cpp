/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>, 2010
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
 *
 */

#define BOOST_TEST_MODULE store_test
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <libklio/store.hpp>
#include <libklio/msg/msgstore.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensorfactory.hpp>
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
    std::cout << "Testing create storage utility for SQLite3" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    bfs::path db(TEST_DB_FILE);
    try {
        std::cout << "Attempting to create " << db << std::endl;
        klio::Store::Ptr store(factory->createStore(klio::SQLITE3, db));
        std::cout << "Created: " << store->str() << std::endl;
        store->open(); // Second call to open - should not break
        store->initialize();
    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
    try {
        klio::Store::Ptr invalid_store(factory->createStore(klio::UNDEFINED, db));
        BOOST_FAIL("No exception occurred for invalid createStore request");
    } catch (klio::GenericException const & ex) {
        std::cout << "Caught valid exception: " << ex.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(check_open_storage_sqlite3) {
    std::cout << "Testing open storage utility for SQLite3" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    bfs::path db(TEST_DB_FILE);
    try {
        std::cout << "Attempting to create " << db << std::endl;
        klio::Store::Ptr store(factory->createStore(klio::SQLITE3, db));
        std::cout << "Created database: " << store->str() << std::endl;
        store->initialize();
        store->close();

        klio::Store::Ptr loaded(factory->openStore(klio::SQLITE3, db));
        std::cout << "Opened database: " << loaded->str() << std::endl;
    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
    try {
        klio::Store::Ptr invalid_store(factory->createStore(klio::UNDEFINED, db));
        BOOST_FAIL("No exception occurred for invalid createStore request");
    } catch (klio::GenericException const & ex) {
        std::cout << "Caught valid exception: " << ex.what() << std::endl;
    }

}

BOOST_AUTO_TEST_CASE(check_create_storage_msg) {

    std::cout << "Testing create storage utility for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::Store::Ptr store(factory->createMSGStore(url));
        std::cout << "Created: " << store->str() << std::endl;
        store->open(); // Second call to open - should not break
        store->initialize();

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_get_all_readings_msg) {

    std::cout << "Testing get_all_readings for MSG" << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory());
    std::string url = "https://dev3-api.mysmartgrid.de:8443";

    try {
        std::cout << "Attempting to create MSG store " << url << std::endl;

        klio::Store::Ptr store(factory->createMSGStore(url));

        std::cout << "Created: " << store->str() << std::endl;
        store->open(); // Second call to open - should not break
        store->initialize();
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                std::string("90c180748bcf240bdb7cc1281038adcb"),
                std::string("Test V2"),
                std::string("description"),
                std::string("watt"),
                std::string("Europe/Berlin")));

        klio::readings_t_Ptr readings = store->get_all_readings(sensor);

        //TODO: complete this test
        BOOST_CHECK_EQUAL(61, readings->size());

    } catch (klio::StoreException const& ex) {
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

//BOOST_AUTO_TEST_SUITE_END()
