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
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/test/unit_test.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/device-type.hpp>
#include <testconfig.h>

BOOST_AUTO_TEST_CASE(check_sensor_interface) {

    std::cout << std::endl << "*** Checking sensor semantics." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
    std::cout << "Created " << sensor1->str() << std::endl;

    klio::Sensor::Ptr sensor1a(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
    std::cout << "Created " << sensor1a->str() << std::endl;

    klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt-Hours", "Europe/Berlin"));
    std::cout << "Created " << sensor2->str() << std::endl;

    if (sensor1 != sensor1) {
        BOOST_FAIL("Sensor is not identical to itself.");
    }
    if (sensor1 == sensor1a) {
        BOOST_FAIL("Sensor1 == Sensor2, oh noes");
    }
    if (sensor1 == sensor2) {
        BOOST_FAIL("Sensor1 == Sensor2, oh noes");
    }
}

BOOST_AUTO_TEST_CASE(check_create_sensor_sqlite3) {

    try {
        std::cout << std::endl << "*** Testing sensor creation for the SQLite3 store" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                "70c18074-8bcf-240b-db7c-c1281038adcb",
                "sensor1",
                "sensor1",
                "description",
                "Watt",
                "Europe/Berlin",
                klio::DeviceType::COFFEE_MACHINE));

        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            std::cout << "trying to add: " << sensor1->str() << std::endl;
            store->add_sensor(sensor1);
            std::cout << "added: " << sensor1->str() << std::endl;

            klio::Sensor::Ptr loadedSensor1(store->get_sensor(sensor1->uuid()));
            std::cout << "loaded: " << loadedSensor1->str() << std::endl;

            BOOST_CHECK_EQUAL(loadedSensor1->description(), "description");
            if ((*sensor1) != (*loadedSensor1)) {
                BOOST_FAIL("loaded sensor differs from its original.");
            } else {
                std::cout << "WIN! sensor restored successfully." << std::endl;
            }

        } catch (klio::StoreException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }

        std::cout << "TEST1 " << sensor1->str() << std::endl;
        
        try {
            store->add_sensor(sensor1);
            BOOST_FAIL("No exception occurred during duplicate sensor add request");
        } catch (klio::StoreException const& ex) {
            std::cout << "Caught expected exception: " << ex.what() << std::endl;
            //ok.
        }

        // cleanup.
        store->dispose();

    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_update_sensor) {

    try {
        std::cout << std::endl << "*** Testing update sensor" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "79c18074-8bcf-240b-db7c-c1281038adcb",
                "original_external_id",
                "sensor1",
                "description",
                "Watt",
                "Europe/Berlin",
                klio::DeviceType::COFFEE_MACHINE));

        boost::uuids::uuid sensor_id = sensor->uuid();

        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor);

            sensor->external_id("Changed External id");
            sensor->name("Changed Name");
            sensor->description("Changed Description");
            sensor->unit("kWh");
            sensor->timezone("Europe/Paris");
            sensor->device_type(klio::DeviceType::AIR_CONDITIONER);

            store->update_sensor(sensor);
            std::cout << "Updated: " << store->str() << std::endl;

            // Test unique sensor name retrieval
            std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_name("Changed Name");

            BOOST_CHECK_EQUAL(sensors.size(), 1U);

            std::vector<klio::Sensor::Ptr>::iterator it;
            for (it = sensors.begin(); it < sensors.end(); it++) {

                klio::Sensor::Ptr found = (*it);
                std::cout << "Found Sensor: " << found->name() << std::endl;

                BOOST_CHECK_EQUAL(found->uuid(), sensor_id);
                BOOST_CHECK_EQUAL(found->external_id(), "Changed External id");
                BOOST_CHECK_EQUAL(found->name(), "Changed Name");
                BOOST_CHECK_EQUAL(found->description(), "Changed Description");
                BOOST_CHECK_EQUAL(found->unit(), "kWh");
                BOOST_CHECK_EQUAL(found->timezone(), "Europe/Paris");
                BOOST_CHECK_EQUAL(found->device_type()->id(), klio::DeviceType::AIR_CONDITIONER->id());
            }
            // cleanup.
            store->dispose();

        } catch (klio::StoreException const& ex) {
            // cleanup.
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_sensor_by_uuid) {

    try {
        std::cout << std::endl << "*** Testing sensor query by uuid" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                "79c18074-8bcf-240b-db7c-c1281038adcb",
                "original_external_id",
                "sensor1",
                "description",
                "Watt",
                "Europe/Berlin",
                klio::DeviceType::AQUARIUM));

        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor);

            klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

            // cleanup.
            store->dispose();

            // Test unique sensor id retrieval
            BOOST_CHECK_EQUAL(sensor->uuid(), retrieved->uuid());
            BOOST_CHECK_EQUAL(sensor->external_id(), retrieved->external_id());
            BOOST_CHECK_EQUAL(sensor->name(), retrieved->name());
            BOOST_CHECK_EQUAL(sensor->unit(), retrieved->unit());
            BOOST_CHECK_EQUAL(sensor->timezone(), retrieved->timezone());
            BOOST_CHECK_EQUAL(sensor->device_type(), retrieved->device_type());

        } catch (klio::StoreException const& ex) {
            // cleanup.
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_sensor_by_name) {

    try {
        std::cout << std::endl << "*** Testing sensor query by name" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        std::string sensor1_name("sensor_1");
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", sensor1_name, "Watt", "Europe/Berlin"));
        std::string sensor2_name("sensor2");
        klio::Sensor::Ptr sensor2a(sensor_factory->createSensor("sensor2a", sensor2_name, "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2b(sensor_factory->createSensor("sensor2b", sensor2_name, "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor1);
            store->add_sensor(sensor2a);
            store->add_sensor(sensor2b);
            // Test unique sensor id retrieval
            std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_name(sensor1_name);
            BOOST_CHECK_EQUAL(sensors.size(), 1U);
            std::vector<klio::Sensor::Ptr>::iterator it;
            for (it = sensors.begin(); it < sensors.end(); it++) {
                klio::Sensor::Ptr found = (*it);
                std::cout << "Found Sensor: " << found->name() << std::endl;
                BOOST_CHECK_EQUAL(found->name(), sensor1_name);
                BOOST_CHECK_EQUAL(found->uuid(), sensor1->uuid());
            }
            // Test duplicate sensor name retrieval
            sensors = store->get_sensors_by_name(sensor2_name);
            BOOST_CHECK_EQUAL(sensors.size(), 2U);
            for (it = sensors.begin(); it < sensors.end(); it++) {
                klio::Sensor::Ptr found = (*it);
                std::cout << "Found Sensor: " << found->name() << std::endl;
                BOOST_CHECK_EQUAL(found->name(), sensor2_name);
                //BOOST_CHECK_EQUAL (sensor->uuid(), sensor1->uuid());
            }

            // cleanup.
            store->dispose();

        } catch (klio::StoreException const& ex) {
            // cleanup.
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_retrieve_sensor_uuids_sqlite3) {

    try {
        std::cout << std::endl << "*** Testing sensor uuid query for the SQLite3 store" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor1);
            store->add_sensor(sensor2);
            std::cout << "added: " << sensor1->str() << std::endl;
            std::cout << "added: " << sensor2->str() << std::endl;

            std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();

            // cleanup.
            store->dispose();

            std::vector<klio::Sensor::uuid_t>::iterator it;
            for (it = uuids.begin(); it < uuids.end(); it++) {
                std::cout << "Found Sensor: " << to_string(*it) << std::endl;
            }

            it = find(uuids.begin(), uuids.end(), sensor1->uuid());
            if (*it != sensor1->uuid())
                BOOST_FAIL("sensor1 not retrieved from store.");

            it = find(uuids.begin(), uuids.end(), sensor2->uuid());
            if (*it != sensor2->uuid())
                BOOST_FAIL("sensor2 not retrieved from store.");

        } catch (klio::StoreException const& ex) {
            // cleanup.
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_sensor_sqlite3) {

    try {
        std::cout << std::endl << "*** Testing sensor removal SQLite3 store" << std::endl;
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
        klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin"));
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        bfs::path db(TEST_DB1_FILE);
        bfs::remove(db);
        klio::Store::Ptr store(factory->create_sqlite3_store(db));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->add_sensor(sensor1);
            std::cout << "added: " << sensor1->str() << std::endl;

            std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
            std::vector<klio::Sensor::uuid_t>::iterator it;
            it = find(uuids.begin(), uuids.end(), sensor1->uuid());
            if (*it != sensor1->uuid())
                BOOST_FAIL("sensor1 not retrieved from store.");
            else
                std::cout << "Sensor1 is in the Database." << std::endl;

            // Now: Delete sensor1
            store->remove_sensor(sensor1);
            uuids = store->get_sensor_uuids();
            std::cout << "Found " << uuids.size() << " sensor(s) in the database." << std::endl;
            it = find(uuids.begin(), uuids.end(), sensor1->uuid());
            for (it = uuids.begin(); it != uuids.end(); ++it) {
                std::cout << to_string(*it) << std::endl;
            }
            //if ((*it == sensor1->uuid()) && uuids.size() > 0)
            if (uuids.size() > 0)
                BOOST_FAIL("sensor1 not deleted from store.");
            
            // cleanup.
            store->dispose();

        } catch (klio::StoreException const& ex) {
            // cleanup.
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected store exception occurred during sensor test");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during sensor test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_device_type_by_id) {

    klio::DeviceType::Ptr type = klio::DeviceType::get_by_id(300);
    BOOST_CHECK_EQUAL(klio::DeviceType::FRIDGE, type);
    BOOST_CHECK_EQUAL(300, type->id());
    BOOST_CHECK_EQUAL("Fridge", type->name());
    
    type = klio::DeviceType::get_by_id(404);
    BOOST_CHECK_EQUAL(klio::DeviceType::COMPUTER, type);
    BOOST_CHECK_EQUAL(404, type->id());
    BOOST_CHECK_EQUAL("Computer", type->name());

    try {
        klio::DeviceType::get_by_id(999);

        BOOST_FAIL("An exception should be raised when an invalid device type id is informed.");

    } catch (klio::DataFormatException const& ex) {
        //This exception is expected
    }
}

//BOOST_AUTO_TEST_SUITE_END()
