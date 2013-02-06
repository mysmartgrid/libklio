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
#include <boost/test/unit_test.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>
#include <testconfig.h>


BOOST_AUTO_TEST_CASE ( check_sensor_interface ) {
  std::cout << std::endl << "*** Checking sensor semantics." << std::endl;
  klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
  klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
  std::cout << "Created " << sensor1->str() << std::endl;
  klio::Sensor::Ptr sensor1a(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
  std::cout << "Created " << sensor1a->str() << std::endl;
  klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "Watt-Hours", "Europe/Berlin")); 
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
 ;;
}

BOOST_AUTO_TEST_CASE ( check_create_sensor_sqlite3 ) {
  try {
    std::cout << std::endl << "*** Testing sensor creation for the SQLite3 store" << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    bfs::remove(db);
    klio::Store::Ptr store(factory->create_sqlite3_store(db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->add_sensor(sensor1);
      std::cout << "added: " << sensor1->str() << std::endl;
      klio::Sensor::Ptr loadedSensor1(store->get_sensor(sensor1->uuid()));
      std::cout << "loaded: " << loadedSensor1->str() << std::endl;
      // We did not specify a description, so we expect the default one.
      BOOST_CHECK_EQUAL (loadedSensor1->description(), klio::DEFAULT_SENSOR_DESCRIPTION);
      if ((*sensor1) != (*loadedSensor1)) {
        BOOST_FAIL("loaded sensor differs from its original.");
      } else {
        std::cout << "WIN! sensor restored successfully." << std::endl;
      }
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occurred during sensor test" );
    } 
    try {
      store->add_sensor(sensor1);
      BOOST_FAIL( "No exception occurred during duplicate sensor add request" );
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught expected exception: " << ex.what() << std::endl;
      //ok.
    } 
    // cleanup.
    store->remove_sensor(sensor1);
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occurred during sensor test" );
  }
}

BOOST_AUTO_TEST_CASE ( check_add_sensor_description ) {
  try {
    std::cout << std::endl << "*** Testing add sensor description " << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    std::string sensor1_name("sensor1");
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor(sensor1_name, "Watt", "Europe/Berlin")); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    bfs::remove(db);
    klio::Store::Ptr store(factory->create_sqlite3_store(db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->add_sensor(sensor1);
      // add a description
      std::string description("Test Description");
      store->add_description(sensor1, description);
      // Test unique sensor id retrieval
      std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_name(sensor1_name);
      BOOST_CHECK_EQUAL (sensors.size(), 1);
      std::vector<klio::Sensor::Ptr>::iterator it;
      for(  it = sensors.begin(); it < sensors.end(); it++) {
        klio::Sensor::Ptr sensor=(*it);
        std::cout << "Found Sensor: " << sensor->name() << std::endl;
        BOOST_CHECK_EQUAL (sensor->name(), sensor1_name);
        BOOST_CHECK_EQUAL (sensor->uuid(), sensor1->uuid());
        BOOST_CHECK_EQUAL (sensor->description(), description);
      }
      store->remove_sensor(sensor1);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occurred during sensor test" );
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occurred during sensor test" );
  }

}

BOOST_AUTO_TEST_CASE ( check_retrieve_sensor_by_name ) {
  try {
    std::cout << std::endl << "*** Testing sensor query by name" << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    std::string sensor1_name("sensor_1");
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor(sensor1_name, "Watt", "Europe/Berlin")); 
    std::string sensor2_name("sensor2");
    klio::Sensor::Ptr sensor2a(sensor_factory->createSensor(sensor2_name, "Watt", "Europe/Berlin")); 
    klio::Sensor::Ptr sensor2b(sensor_factory->createSensor(sensor2_name, "Watt", "Europe/Berlin")); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    bfs::remove(db);
    klio::Store::Ptr store(factory->create_sqlite3_store(db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->add_sensor(sensor1);
      store->add_sensor(sensor2a);
      store->add_sensor(sensor2b);
      // Test unique sensor id retrieval
      std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_name(sensor1_name);
      BOOST_CHECK_EQUAL (sensors.size(), 1);
      std::vector<klio::Sensor::Ptr>::iterator it;
      for(  it = sensors.begin(); it < sensors.end(); it++) {
        klio::Sensor::Ptr sensor=(*it);
        std::cout << "Found Sensor: " << sensor->name() << std::endl;
        BOOST_CHECK_EQUAL (sensor->name(), sensor1_name);
        BOOST_CHECK_EQUAL (sensor->uuid(), sensor1->uuid());
      }
      // Test duplicate sensor name retrieval
      sensors = store->get_sensors_by_name(sensor2_name);
      BOOST_CHECK_EQUAL (sensors.size(), 2);
      for(  it = sensors.begin(); it < sensors.end(); it++) {
        klio::Sensor::Ptr sensor=(*it);
        std::cout << "Found Sensor: " << sensor->name() << std::endl;
        BOOST_CHECK_EQUAL (sensor->name(), sensor2_name);
        //BOOST_CHECK_EQUAL (sensor->uuid(), sensor1->uuid());
      }
      // cleanup.
      store->remove_sensor(sensor1);
      store->remove_sensor(sensor2a);
      store->remove_sensor(sensor2b);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occurred during sensor test" );
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occurred during sensor test" );
  }
}

BOOST_AUTO_TEST_CASE ( check_retrieve_sensor_uuids_sqlite3 ) {
  try {
    std::cout << std::endl << "*** Testing sensor uuid query for the SQLite3 store" << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
    klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "Watt", "Europe/Berlin")); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    bfs::remove(db);
    klio::Store::Ptr store(factory->create_sqlite3_store(db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->add_sensor(sensor1);
      store->add_sensor(sensor2);
      std::cout << "added: " << sensor1->str() << std::endl;
      std::cout << "added: " << sensor2->str() << std::endl;
      std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
      std::vector<klio::Sensor::uuid_t>::iterator it;
      for(  it = uuids.begin(); it < uuids.end(); it++) {
        std::cout << "Found Sensor: " << to_string(*it) << std::endl;
      }
     
      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
      if (*it != sensor1->uuid())
        BOOST_FAIL("sensor1 not retrieved from store.");

      it = find (uuids.begin(), uuids.end(), sensor2->uuid());
      if (*it != sensor2->uuid())
        BOOST_FAIL("sensor2 not retrieved from store.");

      // cleanup.
      store->remove_sensor(sensor1);
      store->remove_sensor(sensor2);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occurred during sensor test" );
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occurred during sensor test" );
  }
}

BOOST_AUTO_TEST_CASE ( check_remove_sensor_sqlite3 ) {
  try {
    std::cout << std::endl << "*** Testing sensor removal SQLite3 store" << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    bfs::remove(db);
    klio::Store::Ptr store(factory->create_sqlite3_store(db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->add_sensor(sensor1);
      std::cout << "added: " << sensor1->str() << std::endl;

      std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
      std::vector<klio::Sensor::uuid_t>::iterator it;
      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
      if (*it != sensor1->uuid())
        BOOST_FAIL("sensor1 not retrieved from store.");
      else
        std::cout << "Sensor1 is in the Database." << std::endl;

      // Now: Delete sensor1
      store->remove_sensor(sensor1);
      uuids = store->get_sensor_uuids();
      std::cout << "Found " << uuids.size() << " sensor(s) in the database." << std::endl;
      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
      for(  it = uuids.begin(); it != uuids.end(); ++it) {
        std::cout << to_string(*it)<< std::endl;
      }
      //if ((*it == sensor1->uuid()) && uuids.size() > 0)
      if (uuids.size() > 0)
        BOOST_FAIL("sensor1 not deleted from store.");

    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occurred during sensor test" );
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occurred during sensor test" );
  }
}

//BOOST_AUTO_TEST_SUITE_END()
