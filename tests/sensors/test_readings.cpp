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

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensorfactory.hpp>
#include <boost/uuid/uuid_io.hpp>


BOOST_AUTO_TEST_CASE ( check_add_retrieve_reading ) {
  try {
    std::cout << std::endl << "*** Adding a reading to a sensor." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "MEZ")); 
    std::cout << "Created " << sensor1->str() << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    klio::Store::Ptr store(factory->createStore(klio::SQLITE3));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->addSensor(sensor1);
      std::cout << "added to store: " << sensor1->str() << std::endl;
      // insert a reading.
      klio::TimeConverter::Ptr tc(new klio::TimeConverter());
      klio::timestamp_t timestamp=tc->get_timestamp();
      double reading=23;
      store->add_reading(sensor1, timestamp, reading);

      // now, retrieve it and check.
      std::map<klio::timestamp_t, double> readings = 
        store->get_all_readings(sensor1);
      std::map<klio::timestamp_t, double>::iterator it;
      for(  it = readings.begin(); it != readings.end(); it++) {
        std::cout << (*it).first << " - " << (*it).second << std::endl;
      }
      
      // TODO: Implement check for correct values.

      // cleanup 
      store->removeSensor(sensor1);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
  ;;
}

//BOOST_AUTO_TEST_CASE ( check_create_sensor_sqlite3 ) {
//  try {
//    std::cout << std::endl << "*** Testing sensor creation for the SQLite3 store" << std::endl;
//    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
//    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", 1)); 
//    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
//    klio::Store::Ptr store(factory->createStore(klio::SQLITE3));
//    std::cout << "Created: " << store->str() << std::endl;
//    try {
//      store->initialize();
//      store->addSensor(sensor1);
//      std::cout << "added: " << sensor1->str() << std::endl;
//      klio::Sensor::Ptr loadedSensor1(store->getSensor(sensor1->uuid()));
//      std::cout << "loaded: " << loadedSensor1->str() << std::endl;
//      if ((*sensor1) != (*loadedSensor1)) {
//        BOOST_FAIL("loaded sensor differs from its original.");
//      } else {
//        std::cout << "WIN! sensor restored successfully." << std::endl;
//      }
//    } catch (klio::StoreException const& ex) {
//      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
//      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
//    } 
//    try {
//      store->addSensor(sensor1);
//      BOOST_FAIL( "No exception occured during duplicate sensor add request" );
//    } catch (klio::StoreException const& ex) {
//      std::cout << "Caught expected exception: " << ex.what() << std::endl;
//      //ok.
//    } 
//    // cleanup.
//    store->removeSensor(sensor1);
//  } catch (std::exception const& ex) {
//    BOOST_FAIL( "Unexpected exception occured during sensor test" );
//  }
//}
//
//
//BOOST_AUTO_TEST_CASE ( check_retrieve_sensor_uuids_sqlite3 ) {
//  try {
//    std::cout << std::endl << "*** Testing sensor uuid query for the SQLite3 store" << std::endl;
//    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
//    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", 1)); 
//    klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "Watt", 1)); 
//    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
//    klio::Store::Ptr store(factory->createStore(klio::SQLITE3));
//    std::cout << "Created: " << store->str() << std::endl;
//    try {
//      store->initialize();
//      store->addSensor(sensor1);
//      store->addSensor(sensor2);
//      std::cout << "added: " << sensor1->str() << std::endl;
//      std::cout << "added: " << sensor2->str() << std::endl;
//      std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
//      std::vector<klio::Sensor::uuid_t>::iterator it;
//      for(  it = uuids.begin(); it < uuids.end(); it++) {
//        std::cout << "Found Sensor: " << to_string(*it) << std::endl;
//      }
//     
//      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
//      if (*it != sensor1->uuid())
//        BOOST_FAIL("sensor1 not retrieved from store.");
//
//      it = find (uuids.begin(), uuids.end(), sensor2->uuid());
//      if (*it != sensor2->uuid())
//        BOOST_FAIL("sensor2 not retrieved from store.");
//
//      // cleanup.
//      store->removeSensor(sensor1);
//      store->removeSensor(sensor2);
//    } catch (klio::StoreException const& ex) {
//      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
//      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
//    } 
//  } catch (std::exception const& ex) {
//    BOOST_FAIL( "Unexpected exception occured during sensor test" );
//  }
//}
//
//BOOST_AUTO_TEST_CASE ( check_remove_sensor_sqlite3 ) {
//  try {
//    std::cout << std::endl << "*** Testing sensor removal SQLite3 store" << std::endl;
//    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
//    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", 1)); 
//    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
//    klio::Store::Ptr store(factory->createStore(klio::SQLITE3));
//    std::cout << "Created: " << store->str() << std::endl;
//    try {
//      store->initialize();
//      store->addSensor(sensor1);
//      std::cout << "added: " << sensor1->str() << std::endl;
//
//      std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
//      std::vector<klio::Sensor::uuid_t>::iterator it;
//      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
//      if (*it != sensor1->uuid())
//        BOOST_FAIL("sensor1 not retrieved from store.");
//      else
//        std::cout << "Sensor1 is in the Database." << std::endl;
//
//      // Now: Delete sensor1
//      store->removeSensor(sensor1);
//      uuids = store->getSensorUUIDs();
//      std::cout << "Found " << uuids.size() << " sensor(s) in the database." << std::endl;
//      it = find (uuids.begin(), uuids.end(), sensor1->uuid());
//      for(  it = uuids.begin(); it != uuids.end(); ++it) {
//        std::cout << to_string(*it)<< std::endl;
//      }
//      //if ((*it == sensor1->uuid()) && uuids.size() > 0)
//      if (uuids.size() > 0)
//        BOOST_FAIL("sensor1 not deleted from store.");
//
//    } catch (klio::StoreException const& ex) {
//      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
//      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
//    } 
//  } catch (std::exception const& ex) {
//    BOOST_FAIL( "Unexpected exception occured during sensor test" );
//  }
//}
//
//
//
//
//
//
//
//
//
//
//
////BOOST_AUTO_TEST_SUITE_END()
