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
#include <map>
#include <libklio/common.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensorfactory.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <testconfig.h>


BOOST_AUTO_TEST_CASE ( check_add_retrieve_reading ) {
  try {
    std::cout << std::endl << "*** Adding & retrieving a reading to/from a sensor." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "MEZ")); 
    std::cout << "Created " << sensor1->str() << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    klio::Store::Ptr store(factory->createStore(klio::SQLITE3, db));
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
      klio::readings_t_Ptr readings = store->get_all_readings(sensor1);
      std::map<klio::timestamp_t, double>::iterator it;
      for(  it = readings->begin(); it != readings->end(); it++) {
        klio::timestamp_t ts1=(*it).first;
        double val1=(*it).second;
        std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
        BOOST_CHECK_EQUAL (timestamp, ts1);
        BOOST_CHECK_EQUAL (reading, val1);
      }
            
      // cleanup 
      store->removeSensor(sensor1);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
      //store->removeSensor(sensor1);
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
  ;;
}

BOOST_AUTO_TEST_CASE ( check_retrieve_last_reading ) {
  try {
    std::cout << std::endl << "*** retrieving the last reading from a sensor." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "MEZ")); 
    std::cout << "Created " << sensor1->str() << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    klio::Store::Ptr store(factory->createStore(klio::SQLITE3, db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->addSensor(sensor1);
      std::cout << "added to store: " << sensor1->str() << std::endl;
      // insert a reading.
      klio::TimeConverter::Ptr tc(new klio::TimeConverter());
      klio::timestamp_t timestamp=tc->get_timestamp();
      klio::timestamp_t timestamp2=(tc->get_timestamp() - 100);
      double reading=23;
      store->add_reading(sensor1, timestamp, reading);
      store->add_reading(sensor1, timestamp2, reading);
      std::cout << "Inserted two readings of " << reading << "value, "
        "using timestamps "<< timestamp << " and " << timestamp2 << std::endl;

      // now, retrieve it and check.
      klio::reading_t last_reading = store->get_last_reading(sensor1);
      klio::timestamp_t ts1=last_reading.first;
      double val1=last_reading.second;
      std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
      BOOST_CHECK_EQUAL (timestamp, ts1);
      BOOST_REQUIRE( timestamp2 != ts1 );      
      BOOST_CHECK_EQUAL (reading, val1);

      // cleanup 
      store->removeSensor(sensor1);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
      //store->removeSensor(sensor1);
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
  ;;
}

BOOST_AUTO_TEST_CASE ( check_bulk_insert ) {
  try {
    std::cout << std::endl << "*** bulk-inserting some readings." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "MEZ")); 
    std::cout << "Created " << sensor1->str() << std::endl;
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    bfs::path db(TEST_DB_FILE);
    klio::Store::Ptr store(factory->createStore(klio::SQLITE3, db));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->addSensor(sensor1);
      std::cout << "added to store: " << sensor1->str() << std::endl;
      // insert a reading.
      klio::TimeConverter::Ptr tc(new klio::TimeConverter());
      klio::readings_t readings;
      size_t num_readings=10;
      for (size_t i=0; i<num_readings; i++) {
        klio::timestamp_t timestamp=tc->get_timestamp()-i;
        double reading=23;
        klio::reading_t foo(timestamp, reading);
        readings.insert(foo);
      }
      std::cout << "Inserting " << readings.size() << " readings." << std::endl;
      store->add_readings(sensor1, readings);
      // now, retrieve it and check.
      klio::readings_t_Ptr loaded_readings = store->get_all_readings(sensor1);
      std::cout << "Loaded " << loaded_readings->size() << " readings." << std::endl;
      klio::readings_cit_t it;
      size_t ret_size=0;
      for(  it = loaded_readings->begin(); it != loaded_readings->end(); ++it) {
        klio::timestamp_t ts1=(*it).first;
        double val1=(*it).second;
        std::cout << "Got timestamp " << ts1 << " -> value " << val1 << std::endl;
        ret_size++;
      }
      BOOST_CHECK_EQUAL (num_readings, ret_size);

      // cleanup 
      store->removeSensor(sensor1);
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
      //store->removeSensor(sensor1);
    } 
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
  ;;
}



//BOOST_AUTO_TEST_SUITE_END()
