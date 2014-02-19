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

#define BOOST_TEST_MODULE msg_store_test

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

BOOST_AUTO_TEST_CASE(check_create_msg_storage) {

    try {
        std::cout << "Testing storage creation for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d8736",
                "d221f4de-3ecd-f3d3-00db-3e96755d8733",
                "libklio test desc",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();

            BOOST_CHECK_EQUAL(store->url(), url);
            BOOST_CHECK_EQUAL(store->id(), "d271f4de3ecdf3d300db3e96755d8736");
            BOOST_CHECK_EQUAL(store->key(), "d221f4de3ecdf3d300db3e96755d8733");
            BOOST_CHECK_EQUAL(store->description(), "libklio test desc");
            BOOST_CHECK_EQUAL(store->type(), "libklio");
            BOOST_CHECK_EQUAL(store->activation_code(), "d271f4de3e");

            store->dispose();

            try {
                store = factory->create_msg_store(url,
                        "d281f4de-3ecd-f3d3-00db-3e96755d8736",
                        "d271f4de-3ecd-f3d3-00db-3e96755d8733",
                        "libklio test desc",
                        "xxx");

                store->open();
                store->initialize();

                BOOST_FAIL("A DataFormatException must be thrown when an invalid type is informed.");

            } catch (klio::GenericException const& e) {
                //This exception was expected
            }
            store->dispose();

            try {
                store = factory->create_msg_store(url,
                        "####################################",
                        "d271f4de-3ecd-f3d3-00db-3e96755d8733",
                        "libklio test desc",
                        "libklio");

                store->open();
                store->initialize();

                BOOST_FAIL("A DataFormatException must be thrown when an invalid uuid is informed.");

            } catch (klio::GenericException const& ex) {
                //This exception was expected
            }
            store->dispose();

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_add_msg_sensor) {

    try {
        std::cout << "Testing sensor addition for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "755d8736-3ecd-f3d3-00db-3e96755d8736",
                "755d8736-3ecd-f3d3-00db-3e96755d8733",
                "libklio test desc",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "89c18074-8bcf-240b-db7c-c1281038adcb",
                    "Test libklio - long id - 94843838239293848349393203994349939äß´€",
                    "Test äß´€",
                    "this is a sensor description äß´€",
                    "kwh",
                    "Europe/Berlin"));

            store->add_sensor(sensor);

            klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

            BOOST_CHECK_EQUAL(retrieved->uuid(), sensor->uuid());
            BOOST_CHECK_EQUAL(retrieved->external_id(), "Test libklio - long id - 94843838239293848349393203994349939äß´€");
            BOOST_CHECK_EQUAL(retrieved->name(), "Test äß´€");
            BOOST_CHECK_EQUAL(retrieved->description(), "this is a sensor description äß´€");
            BOOST_CHECK_EQUAL(retrieved->unit(), "kwh");
            BOOST_CHECK_EQUAL(retrieved->timezone(), "Europe/Berlin");

            try {
                sensor = sensor_factory->createSensor(
                        "89c74018-8bcf-240b-db7c-c1281038adcb",
                        "Test 89c74018-8bcf-240b-db7c-c1281038adcb",
                        "Test libklio",
                        "changed description",
                        "xxx",
                        "Europe/Berlin");

                store->add_sensor(sensor);

                BOOST_FAIL("A DataFormatException must be thrown when an invalid unit is informed.");

            } catch (klio::GenericException const& ex) {
                //This exception was expected
            }

            try {
                sensor = sensor_factory->createSensor(
                        "89c74018-8bcf-240b-db7c-c1281038adcb",
                        "Test libklio - this id is too long - 9484383823929384834939320399434993932200000",
                        "Test",
                        "changed description",
                        "xxx",
                        "Europe/Berlin");

                store->add_sensor(sensor);

                BOOST_FAIL("A DataFormatException must be thrown when an invalid external_id is informed.");

            } catch (klio::GenericException const& ex) {
                //This exception was expected
            }

            store->dispose();

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_update_msg_sensor) {

    try {
        std::cout << "Testing sensor update for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d8736",
                "d221f4de-3ecd-f3d3-00db-3e96755d8733",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "89c18074-8bcf-240b-db7c-c1281038adcb",
                    "Test 89c18074-8bcf-240b-db7c-c1281038adcb",
                    "Test libklio",
                    "description",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor);

            klio::Sensor::Ptr changed = sensor_factory->createSensor(
                    "89c18074-8bcf-240b-db7c-c1281038adcb",
                    "Test 89c18074-8bcf-240b-db7c-c1281038adcb",
                    "Test libklio",
                    "changed description",
                    "watt",
                    "Europe/Berlin");

            store->update_sensor(changed);

            klio::Sensor::Ptr retrieved = store->get_sensor(changed->uuid());

            BOOST_CHECK_EQUAL(retrieved->name(), changed->name());
            BOOST_CHECK_EQUAL(retrieved->external_id(), changed->external_id());
            BOOST_CHECK_EQUAL(retrieved->name(), changed->name());
            BOOST_CHECK_EQUAL(retrieved->description(), changed->description());
            BOOST_CHECK_EQUAL(retrieved->unit(), changed->unit());
            BOOST_CHECK_EQUAL(retrieved->timezone(), changed->timezone());

            store->dispose();

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_remove_msg_sensor) {

    try {
        std::cout << "Testing sensor removal for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d3333",
                "d221f4de-3ecd-f3d3-00db-3e96755d3333",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor(sensor_factory->createSensor(
                    "98c18074-8bcf-890b-db7c-c12810383333",
                    "Test 98c18074-8bcf-890b-db7c-c12810383333",
                    "Test libklio",
                    "description",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor);

            store->remove_sensor(sensor);

            try {
                klio::Sensor::Ptr retrieved = store->get_sensor(sensor->uuid());

                BOOST_FAIL("An exception must be thrown if the sensor is found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }
            store->dispose();

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor) {

    try {
        std::cout << "Testing sensor query by uuid for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d3333",
                "d221f4de-3ecd-f3d3-00db-3e96755d3333",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                    "98c18074-8bcf-890b-db7c-c12810383333",
                    "Id123123123",
                    "Test1",
                    "Description1",
                    "watt",
                    "Europe/Berlin"));

            klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                    "89c18074-8bcf-890b-db7c-c12810383333",
                    "Id123123123",
                    "Test2",
                    "Description2",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor1);
            store->add_sensor(sensor2);

            
            klio::Sensor::Ptr retrieved = store->get_sensor(sensor1->uuid());

            BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
            BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
            BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
            BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());

            store->remove_sensor(sensor1);

            try {
                //Non existent
                store->get_sensor(sensor1->uuid());
                store->dispose();

                BOOST_FAIL("An exception must be raised if the sensor is found.");

            } catch (klio::StoreException const& ok) {
                //This exception is expected
            }

            store->dispose();

        } catch (klio::GenericException const& ex) {
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor_by_name) {

    try {
        std::cout << "Testing sensor query by name for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d3333",
                "d221f4de-3ecd-f3d3-00db-3e96755d3333",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                    "98c18074-8bcf-890b-db7c-c12810383333",
                    "Unique External Id",
                    "Unique Name",
                    "Unique Description",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor1);

            klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                    "88c18074-890b-8bcf-db7c-c12810383333",
                    "External Id 88c18074-890b-8bcf-db7c-c12810383333",
                    "Duplicated Name",
                    "Duplicated Description",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor2);

            klio::Sensor::Ptr sensor3(sensor_factory->createSensor(
                    "99c18074-890b-8bcf-db7c-c12810383333",
                    "External Id 99c18074-890b-8bcf-db7c-c12810383333",
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
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor_by_external_id) {

    try {
        std::cout << "Testing sensor query by external id for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d8736",
                "d221f4de-3ecd-f3d3-00db-3e96755d8733",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                    "82c18074-8bcf-890b-db7c-c1281038adcb",
                    "External Id 82c18074-8bcf-890b-db7c-c1281038adcb",
                    "Sensor 1",
                    "Description 1",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor1);

            klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                    "74c18074-890b-8bcf-db7c-c1281038adcb",
                    "External Id 74c18074-890b-8bcf-db7c-c1281038adcb",
                    "Sensor 2",
                    "Description 2",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor2);

            klio::Sensor::Ptr retrieved = store->get_sensor_by_external_id("External Id 82c18074-8bcf-890b-db7c-c1281038adcb");

            BOOST_CHECK_EQUAL(sensor1->uuid(), retrieved->uuid());
            BOOST_CHECK_EQUAL(sensor1->external_id(), retrieved->external_id());
            BOOST_CHECK_EQUAL(sensor1->name(), retrieved->name());
            BOOST_CHECK_EQUAL(sensor1->unit(), retrieved->unit());
            BOOST_CHECK_EQUAL(sensor1->timezone(), retrieved->timezone());
            BOOST_CHECK_EQUAL(sensor1->description(), retrieved->description());

            try {
                store->get_sensor_by_external_id("External Id 3");
                store->dispose();
                BOOST_FAIL("An exception must be raised if the sensor is not found.");

            } catch (klio::StoreException const& ex) {
                //This exception is expected
            }

            store->dispose();

        } catch (klio::GenericException const& ex) {
            store->dispose();
            std::cout << "Caught invalid exception: " << ex.what() << std::endl;
            BOOST_FAIL("Unexpected exception occurred for initialize request");
        }
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensors_by_external_id) {

    try {
        std::cout << "Testing sensor query by external id for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d8736",
                "d221f4de-3ecd-f3d3-00db-3e96755d8733",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                    "82c18074-8bcf-890b-db7c-c1281038adcb",
                    "External Id 82c18074-8bcf-890b-db7c-c1281038adcb",
                    "Sensor 1",
                    "Description 1",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor1);

            klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                    "74c18074-890b-8bcf-db7c-c1281038adcb",
                    "External Id 74c18074-890b-8bcf-db7c-c1281038adcb",
                    "Sensor 2",
                    "Description 2",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor2);

            std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id("External Id 82c18074-8bcf-890b-db7c-c1281038adcb");

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
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

BOOST_AUTO_TEST_CASE(check_get_msg_sensor_uuids) {

    try {
        std::cout << "Testing sensor uuids query for mSG" << std::endl;
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        std::string url = "https://dev3-api.mysmartgrid.de:8443";

        std::cout << "Attempting to create MSG store " << url << std::endl;
        klio::MSGStore::Ptr store(factory->create_msg_store(url,
                "d271f4de-3ecd-f3d3-00db-3e96755d3687",
                "d221f4de-3ecd-f3d3-00db-3e96755d8733",
                "libklio test",
                "libklio"));
        std::cout << "Created: " << store->str() << std::endl;

        try {
            store->open();
            store->initialize();
            klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

            klio::Sensor::Ptr sensor1(sensor_factory->createSensor(
                    "98c17480-8bcf-890b-db7c-c1081038adcb",
                    "TestA 98c17480-8bcf-890b-db7c-c1081038adcb",
                    "TestA",
                    "DescriptionA",
                    "watt",
                    "Europe/Berlin"));

            store->add_sensor(sensor1);

            klio::Sensor::Ptr sensor2(sensor_factory->createSensor(
                    "88c17480-890b-8bcf-db7c-c1181038adcb",
                    "TestB 98c17480-8bcf-890b-db7c-c1081038adcb",
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
    } catch (std::exception const& ex) {
        BOOST_FAIL("Unexpected exception occurred during MSGStore test");
    }
}

//BOOST_AUTO_TEST_SUITE_END()
