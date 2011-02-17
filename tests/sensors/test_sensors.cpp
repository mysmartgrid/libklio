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

#define BOOST_TEST_MODULE sensors_test
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensorfactory.hpp>

/**
 * see http://www.boost.org/doc/libs/1_43_0/libs/test/doc/html/tutorials/hello-the-testing-world.html
 */

BOOST_AUTO_TEST_CASE ( check_sanity ) {
  try {
    std::cout << "Demo test case: Checking world sanity." << std::endl;
    BOOST_CHECK_EQUAL (42, 42);
    BOOST_CHECK( 23 != 42 );        // #1 continues on error
    BOOST_REQUIRE( 23 != 42 );      // #2 throws on error

  } catch (std::exception const & ex) {
    BOOST_ERROR ( ex.what() );
  }
  if( 23 == 42 ) {
    BOOST_FAIL( "23 == 42, oh noes");             // #4 throws on error
  }
}

BOOST_AUTO_TEST_CASE ( check_sensor_interface ) {
  klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
  klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", 1)); 
  std::cout << "Created " << sensor1->str() << std::endl;
  klio::Sensor::Ptr sensor1a(sensor_factory->createSensor("sensor1", "Watt", 1)); 
  std::cout << "Created " << sensor1a->str() << std::endl;
  klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "Watt-Hours", 1)); 
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
    std::cout << "Testing sensor creation for the SQLite3 store" << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", 1)); 
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    klio::Store::Ptr store(factory->createStore(klio::SQLITE3));
    std::cout << "Created: " << store->str() << std::endl;
    try {
      store->initialize();
      store->addSensor(sensor1);
      std::cout << "added: " << sensor1->str() << std::endl;
      klio::Sensor::Ptr loadedSensor1(store->getSensor(sensor1->uuid()));
      std::cout << "loaded: " << loadedSensor1->str() << std::endl;

    } catch (klio::StoreException const& ex) {
      std::cout << "Caught invalid exception: " << ex.what() << std::endl;
      BOOST_FAIL( "Unexpected store exception occured during sensor test" );
    } 
    try {
      store->addSensor(sensor1);
      BOOST_FAIL( "No exception occured during duplicate sensor add request" );
    } catch (klio::StoreException const& ex) {
      std::cout << "Caught expected exception: " << ex.what() << std::endl;
      //ok.
    } 

  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
}
//BOOST_AUTO_TEST_SUITE_END()
