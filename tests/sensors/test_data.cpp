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

//#define BOOST_TEST_MODULE data_test
#include <iostream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/test/unit_test.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>

/**
 * see http://www.boost.org/doc/libs/1_43_0/libs/test/doc/html/tutorials/hello-the-testing-world.html
 */

BOOST_AUTO_TEST_CASE ( check_sensor_data_points ) {
  try {
    std::cout << std::endl << "*** Testing data points for a sensor." << std::endl;
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "Watt", "Europe/Berlin")); 
    std::cout << "Created: " << sensor1->str() << std::endl;
//    try {
//      klio::Sensor::Ptr loadedSensor1(store->get_sensor(sensor1->uuid()));
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
//      store->add_sensor(sensor1);
//      BOOST_FAIL( "No exception occured during duplicate sensor add request" );
//    } catch (klio::StoreException const& ex) {
//      std::cout << "Caught expected exception: " << ex.what() << std::endl;
//      //ok.
//    } 
//    // cleanup.
//    store->remove_sensor(sensor1);
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }
}

//BOOST_AUTO_TEST_SUITE_END()
