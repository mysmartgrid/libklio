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

#include <fcntl.h>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
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

int get_openfiles() {

    fd_set closet;
    int flags;
    int fd;
    int n = 0;

    FD_ZERO(&closet);

    for (fd = 0; fd < (int) FD_SETSIZE; fd++) {

        errno = 0;
        flags = fcntl(fd, F_GETFD, 0);
        if (flags == -1 && errno) {

            if (errno != EBADF) {
                return -1;

            } else {
                continue;
            }
        }
        n++;
    }
    return n;
}

BOOST_AUTO_TEST_CASE(check_open_close_sqlite3_storage) {

    std::cout << "Testing opening and closing SQLite3 store" << std::endl;

    int openfiles = get_openfiles();
    std::cout << "Open files    : " << openfiles << std::endl;

    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;

        int new_openfiles = get_openfiles();
        std::cout << "Open files +0 : " << new_openfiles << std::endl;

        store = store_factory->create_sqlite3_store(db);
        new_openfiles = get_openfiles();
        std::cout << "Open files +1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles + 1, new_openfiles);
        openfiles = new_openfiles;

        store->close();

        new_openfiles = get_openfiles();
        std::cout << "Open files -1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles - 1, new_openfiles);
        openfiles = new_openfiles;

        store->open();

        new_openfiles = get_openfiles();
        std::cout << "Open files +1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles + 1, new_openfiles);
        openfiles = new_openfiles;

        store->open();

        new_openfiles = get_openfiles();
        std::cout << "Open files +0 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles, new_openfiles);
        openfiles = new_openfiles;

        store->close();

        new_openfiles = get_openfiles();
        std::cout << "Open files -1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles - 1, new_openfiles);
        openfiles = new_openfiles;

        store->close();

        new_openfiles = get_openfiles();
        std::cout << "Open files +0 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles, new_openfiles);
        openfiles = new_openfiles;

        store->open();

        new_openfiles = get_openfiles();
        std::cout << "Open files +1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles + 1, new_openfiles);
        openfiles = new_openfiles;

        store->close();

        new_openfiles = get_openfiles();
        std::cout << "Open files -1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles - 1, new_openfiles);
        openfiles = new_openfiles;

        store->open();

        new_openfiles = get_openfiles();
        std::cout << "Open files +1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles + 1, new_openfiles);
        openfiles = new_openfiles;

        store->dispose();

        new_openfiles = get_openfiles();
        std::cout << "Open files -1 : " << new_openfiles << std::endl;
        BOOST_CHECK_EQUAL(openfiles - 1, new_openfiles);

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
        bfs::remove(db);
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

        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                "89c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "this is a sensor description",
                "kwh",
                "Europe/Berlin"));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor1->uuid());

        BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());

        std::cout << "Testing duplicated sensor addition for SQLite3" << std::endl;

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                sensor1->uuid_string(),
                "Test2",
                "Test libklio 2",
                "this is a duplicated sensor",
                "kwh",
                "Europe/Berlin"));

        try {
            store->add_sensor(sensor2);

            BOOST_FAIL("Duplicated addition should have been refused.");

        } catch (klio::StoreException e) {
            //expected
            std::cout << "Cought expected exception for duplicated sensor addition attempt." << std::endl;
        }

        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_no_auto_commit) {

    std::cout << "Testing SQLite3 store with no auto commit" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::SQLite3Store::Ptr store;

    try {
        std::cout << "Attempting to create " << db << std::endl;
        store = store_factory->create_sqlite3_store(db, true, false, true, 600, klio::SQLite3Store::OS_SYNC_OFF);
        std::cout << "Created database: " << store->str() << std::endl;

        store->start_transaction();

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "89c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "this is a sensor description",
                "kwh",
                "Europe/Berlin"));

        store->add_sensor(sensor);

        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        for (size_t i = 0; i < 10; i++) {
            store->add_reading(sensor, tc->get_timestamp() - i, 23);
        }

        store->rollback_transaction();

        try {
            store->get_sensor(sensor->uuid());

            BOOST_FAIL("The sensor addition should have been rolled back.");

        } catch (klio::StoreException e) {
            //expected
        }

        try {
            store->get_num_readings(sensor);

            BOOST_FAIL("The sensor and readings addition should have been rolled back.");

        } catch (klio::StoreException e) {
            //expected
        }

        try {
            store->add_sensor(sensor);

            BOOST_FAIL("When automatic commits are off, a transaction must be open before any operation is invoked.");

        } catch (klio::StoreException e) {
            //expected
        }

        store->start_transaction();

        sensor = sensor_factory->createSensor(
                "89c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "this is a sensor description",
                "kwh",
                "Europe/Berlin");

        store->add_sensor(sensor);

        for (size_t i = 0; i < 10; i++) {
            store->add_reading(sensor, tc->get_timestamp() - i, 23);
        }

        store->commit_transaction();

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

        BOOST_CHECK_EQUAL(sensor->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(10, store->get_num_readings(sensor));

        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_sqlite3_flush_errors_handling) {

    std::cout << "Testing SQLite3 flushing errors handling" << std::endl;
    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    klio::SQLite3Store::Ptr store;

    try {
        store = store_factory->create_sqlite3_store(db, true, true, true, 6000, klio::SQLite3Store::OS_SYNC_FULL);

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "89c18074-8bcf-240b-db7c-c1281038adcb",
                "Test",
                "Test libklio",
                "this is a sensor description",
                "kwh",
                "Europe/Berlin"));

        store->add_sensor(sensor);

        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        klio::timestamp_t timestamp = tc->get_timestamp();

        store->add_reading(sensor, timestamp, 23);
        store->add_reading(sensor, timestamp, 23);

        store->flush(true);

        BOOST_CHECK_EQUAL(1, store->get_num_readings(sensor));

        store->add_reading(sensor, timestamp, 23);
        store->add_reading(sensor, timestamp, 23);

        try {
            store->flush(false);

            BOOST_FAIL("When flushing without ignoring errors, an exception must be raised.");

        } catch (klio::StoreException e) {
            //expected
        }

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

        sensor_factory->createSensor(
                "78c18074-8bcf-890b-db7c-c1281038adcb",
                "GetTest2",
                "GetTest2",
                "GetDescription2",
                "watt",
                "Europe/Berlin");

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
                "Id1",
                "Unique Name",
                "Unique Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor1);

        klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                "88c18074-890b-8bcf-db7c-c1281038adcb",
                "Id2",
                "Duplicated Name",
                "Duplicated Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor2);

        klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
                "99c18074-890b-8bcf-db7c-c1281038adcb",
                "Id3",
                "Duplicated Name",
                "Duplicated Description",
                "watt",
                "Europe/Berlin"));

        store->add_sensor(sensor3);

        std::vector<klio::Sensor::Ptr> sensors = store->get_sensors();
        std::vector<klio::Sensor::Ptr> duplicated = store->get_sensors_by_name("Duplicated Name");
        std::vector<klio::Sensor::Ptr> unique = store->get_sensors_by_name("Unique Name");

        BOOST_CHECK_EQUAL(3, sensors.size());
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

void run_store_rotation_tests(const bool auto_commit, const bool auto_flush, const int flush_timeout, const std::string& synchronous) {

    klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    bfs::path db(TEST_DB1_FILE);
    bfs::path old_db(TEST_DB2_FILE);

    std::cout.precision(7);
    std::cout << std::fixed;

    boost::posix_time::ptime time_before;
    boost::posix_time::ptime time_after;
    boost::posix_time::time_duration elapsed_time;
    double seconds = 0;

    klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
            "89c18074-8bcf-260b-db7c-c1281038adcb",
            "Test1",
            "Test1 libklio",
            "this is the sensor 1 description",
            "watt",
            "Europe/Berlin"));

    klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
            "66c18088-8bcf-280b-db7c-c1281038adcb",
            "Test2",
            "Test2 libklio",
            "this is the sensor 2 description",
            "kwh",
            "Europe/Berlin"));

    klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
            "22c18044-8bcf-220b-db7c-c1281038adcb",
            "Test3",
            "Test3 libklio",
            "this is the sensor 3 description",
            "kwh",
            "Europe/Berlin"));

    std::cout << std::endl << "Performance Test - SQLite3Store - " <<
            "auto commit: " << (auto_commit ? "true" : "false") <<
            ", auto flushing: " << (auto_flush ? "true" : "false") <<
            ", flush timeout: " << flush_timeout <<
            ", synchronous: " << synchronous << std::endl;

    klio::SQLite3Store::Ptr store;
    try {

        store = store_factory->create_sqlite3_store(db, true, auto_commit, auto_flush, flush_timeout, synchronous);

        if (!auto_commit) {
            store->start_transaction();
        }

        store->add_sensor(sensor1);
        store->add_sensor(sensor2);
        store->add_sensor(sensor3);

        for (int i = 0; i < 197; i++) {

            std::ostringstream oss;
            oss << "Extra" << i;

            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    oss.str(),
                    oss.str(),
                    "kwh",
                    "Europe/Berlin"));

            store->add_sensor(sensor);
        }

        klio::TimeConverter::Ptr tc(new klio::TimeConverter());
        klio::timestamp_t timestamp = tc->get_timestamp();
        for (int i = 0; i < 100; i++) {
            store->add_reading(sensor1, timestamp - i, 111);
        }

        timestamp = tc->get_timestamp();
        for (int i = 0; i < 200; i++) {
            store->add_reading(sensor2, timestamp - i, 222);
        }

        store->flush(true);

        if (!auto_commit) {
            store->commit_transaction();
        }

        //Start rotation--------------------------------------------------------
        time_before = boost::posix_time::microsec_clock::local_time();
        store->rotate(old_db);
        time_after = boost::posix_time::microsec_clock::local_time();
        //End rotation----------------------------------------------------------

        elapsed_time = time_after - time_before;
        seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
        std::cout << "Performance Test - SQLite3Store - " <<
                "Store new rotation:                         "
                << seconds << " s" << std::endl;


        //Tests the state of the store after rotation
        std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
        BOOST_CHECK_EQUAL(200, uuids.size());

        klio::Sensor::Ptr retrieved = store->get_sensor(sensor1->uuid());

        BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());

        retrieved = store->get_sensor(sensor2->uuid());

        BOOST_CHECK_EQUAL(sensor2->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor2->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor2->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor2->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor2->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor2->timezone(), retrieved->timezone());

        retrieved = store->get_sensor(sensor3->uuid());

        BOOST_CHECK_EQUAL(sensor3->uuid(), retrieved->uuid());
        BOOST_CHECK_EQUAL(sensor3->name(), retrieved->name());
        BOOST_CHECK_EQUAL(sensor3->external_id(), retrieved->external_id());
        BOOST_CHECK_EQUAL(sensor3->description(), retrieved->description());
        BOOST_CHECK_EQUAL(sensor3->unit(), retrieved->unit());
        BOOST_CHECK_EQUAL(sensor3->timezone(), retrieved->timezone());

        BOOST_CHECK_EQUAL(0, store->get_num_readings(sensor1));
        BOOST_CHECK_EQUAL(0, store->get_num_readings(sensor2));
        BOOST_CHECK_EQUAL(0, store->get_num_readings(sensor3));

        if (!auto_commit) {
            store->start_transaction();
        }

        timestamp = tc->get_timestamp();
        for (int i = 0; i < 50; i++) {
            store->add_reading(sensor1, timestamp - i, 111);
        }

        store->flush(true);

        if (!auto_commit) {
            store->commit_transaction();
        }

        klio::readings_t_Ptr readings1 = store->get_all_readings(sensor1);
        BOOST_CHECK_EQUAL(50, readings1->size());

        store->close();
        store->dispose();

    } catch (klio::GenericException const& ex) {
        store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }

    klio::SQLite3Store::Ptr old_store;
    try {
        //Tests the state of the old store
        klio::SQLite3Store::Ptr old_store = store_factory->open_sqlite3_store(old_db, auto_commit, auto_flush, 6000, synchronous);

        BOOST_CHECK_EQUAL(100, old_store->get_num_readings(sensor1));

        klio::readings_t_Ptr readings1 = old_store->get_all_readings(sensor1);
        BOOST_CHECK_EQUAL(100, readings1->size());

        std::map<klio::timestamp_t, double>::iterator it1;
        for (it1 = readings1->begin(); it1 != readings1->end(); it1++) {
            BOOST_CHECK_EQUAL(111, (*it1).second);
        }

        klio::readings_t_Ptr readings2 = old_store->get_all_readings(sensor2);
        BOOST_CHECK_EQUAL(200, readings2->size());

        std::map<klio::timestamp_t, double>::iterator it2;
        for (it2 = readings2->begin(); it2 != readings2->end(); it2++) {
            BOOST_CHECK_EQUAL(222, (*it2).second);
        }

        BOOST_CHECK_EQUAL(0, old_store->get_num_readings(sensor3));

        old_store->close();
        old_store->dispose();

    } catch (klio::GenericException const& ex) {
        old_store->dispose();
        std::cout << "Caught invalid exception: " << ex.what() << std::endl;
        BOOST_FAIL("Unexpected exception occurred for initialize request");
    }
}

BOOST_AUTO_TEST_CASE(check_store_rotation_tests) {

    std::cout << std::endl << "Performance Test" << std::endl;
    std::cout << "Testing sensors synchronization for SQLite3" <<
            "Performance Test" << std::endl;

    run_store_rotation_tests(true, true, 600, klio::SQLite3Store::OS_SYNC_OFF);
    run_store_rotation_tests(true, true, 0, klio::SQLite3Store::OS_SYNC_OFF);
    run_store_rotation_tests(false, true, 600, klio::SQLite3Store::OS_SYNC_OFF);
    run_store_rotation_tests(false, true, 0, klio::SQLite3Store::OS_SYNC_OFF);
    run_store_rotation_tests(false, false, 600, klio::SQLite3Store::OS_SYNC_OFF);
    run_store_rotation_tests(false, false, 600, klio::SQLite3Store::OS_SYNC_NORMAL);
    run_store_rotation_tests(false, false, 600, klio::SQLite3Store::OS_SYNC_FULL);
}

BOOST_AUTO_TEST_CASE(check_sqlite3_store_creation_performance) {

    try {
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store;

        boost::posix_time::time_duration elapsed_time;
        double seconds = 0;
        boost::posix_time::ptime time_before;
        boost::posix_time::ptime time_after;

        std::cout.precision(7);
        std::cout << std::fixed;

        try {
            std::cout << std::endl << "Performance Test" << std::endl;
            std::cout << "Performance Test - SQLite3Store - " <<
                    "no prepared statements" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store = store_factory->create_sqlite3_store(db, false, false, false, 0, klio::SQLite3Store::OS_SYNC_OFF);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Create store :                              "
                    << seconds << " s" << std::endl;

            store->dispose();

            std::cout << std::endl << "Performance Test" << std::endl;
            std::cout << std::endl << "Performance Test - SQLite3Store - " <<
                    "prepared statements" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store = store_factory->create_sqlite3_store(db, true, false, false, 0, klio::SQLite3Store::OS_SYNC_OFF);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Create store:                               "
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

void run_sqlite3_store_performance_tests(const bool auto_commit, const bool auto_flush, const long flush_timeout, const std::string synchronous, const size_t num_readings) {

    try {
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr store_factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        klio::Store::Ptr store;

        boost::posix_time::time_duration elapsed_time;
        double seconds = 0;
        boost::posix_time::ptime time_before;
        boost::posix_time::ptime time_after;

        std::cout.precision(7);
        std::cout << std::fixed;

        try {
            std::cout << std::endl << "Performance Test" << std::endl;
            std::cout << std::endl << "Performance Test - SQLite3Store - " <<
                    "prepared statements" <<
                    ", auto commit: " << (auto_commit ? "true" : "false") <<
                    ", auto flushing: " << (auto_flush ? "true" : "false") <<
                    ", synchronous: " << synchronous << std::endl;

            store = store_factory->create_sqlite3_store(db, true, auto_commit, auto_flush, flush_timeout, synchronous);

            if (!auto_commit) {
                store->start_transaction();
            }

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_sensor(sensor1);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Add 1st sensor:                             "
                    << seconds << " s" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_sensor(sensor2);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
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
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Add 1st reading:                            "
                    << seconds << " s" << std::endl;

            timestamp -= 3000;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->add_reading(sensor1, timestamp, reading);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
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
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Add " << num_readings << " readings:                          "
                    << seconds << " s" << std::endl;


            if (!auto_flush) {
                time_before = boost::posix_time::microsec_clock::local_time();
                store->flush();
                time_after = boost::posix_time::microsec_clock::local_time();

                elapsed_time = time_after - time_before;
                seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
                std::cout << "Performance Test - SQLite3Store - " <<
                        "Flushing " << num_readings << " readings:                     "
                        << seconds << " s" << std::endl;
            }

            if (!auto_commit) {
                time_before = boost::posix_time::microsec_clock::local_time();
                store->commit_transaction();
                time_after = boost::posix_time::microsec_clock::local_time();

                elapsed_time = time_after - time_before;
                seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
                std::cout << "Performance Test - SQLite3Store - " <<
                        "Committing " << num_readings << " readings:                   "
                        << seconds << " s" << std::endl;
            }

            time_before = boost::posix_time::microsec_clock::local_time();
            store->get_sensors_by_external_id(sensor1->external_id());
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
                    "Get sensors by external id:                 "
                    << seconds << " s" << std::endl;

            time_before = boost::posix_time::microsec_clock::local_time();
            store->get_all_readings(sensor1);
            time_after = boost::posix_time::microsec_clock::local_time();

            elapsed_time = time_after - time_before;
            seconds = ((double) elapsed_time.total_microseconds()) / 1000000;
            std::cout << "Performance Test - SQLite3Store - " <<
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

BOOST_AUTO_TEST_CASE(check_sqlite3_store_performance) {

    run_sqlite3_store_performance_tests(true, true, 0, klio::SQLite3Store::OS_SYNC_FULL, 1000);
    run_sqlite3_store_performance_tests(true, false, 0, klio::SQLite3Store::OS_SYNC_FULL, 1000);
    run_sqlite3_store_performance_tests(false, true, 0, klio::SQLite3Store::OS_SYNC_FULL, 1000);
    run_sqlite3_store_performance_tests(false, false, 0, klio::SQLite3Store::OS_SYNC_FULL, 1000);

    run_sqlite3_store_performance_tests(true, true, 0, klio::SQLite3Store::OS_SYNC_NORMAL, 1000);
    run_sqlite3_store_performance_tests(true, false, 0, klio::SQLite3Store::OS_SYNC_NORMAL, 1000);
    run_sqlite3_store_performance_tests(false, true, 0, klio::SQLite3Store::OS_SYNC_NORMAL, 1000);
    run_sqlite3_store_performance_tests(false, false, 0, klio::SQLite3Store::OS_SYNC_NORMAL, 1000);

    run_sqlite3_store_performance_tests(true, true, 0, klio::SQLite3Store::OS_SYNC_OFF, 1000);
    run_sqlite3_store_performance_tests(true, false, 0, klio::SQLite3Store::OS_SYNC_OFF, 1000);
    run_sqlite3_store_performance_tests(false, true, 0, klio::SQLite3Store::OS_SYNC_OFF, 1000);
    run_sqlite3_store_performance_tests(false, false, 0, klio::SQLite3Store::OS_SYNC_OFF, 1000);
}

//BOOST_AUTO_TEST_SUITE_END()
