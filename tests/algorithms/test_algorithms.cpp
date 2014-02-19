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
#include <boost/test/unit_test.hpp>
#include <libklio/algorithm/collate.hpp>
#include <libklio/types.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <testconfig.h>


klio::Store::Ptr generate_store() {
  klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
  bfs::path db(TEST_DB1_FILE);
  std::cout << "Attempting to create " << db << std::endl;
  klio::Store::Ptr store(factory->open_sqlite3_store(db));
  std::cout << "Created: " << store->str() << std::endl;
  store->open(); // Second call to open - should not break
  store->initialize();
  return store;
}

void fill_symmetric_test_data(
    klio::Store::Ptr store, 
    const klio::sensors_t& sensors,
    uint16_t num_datapoints
    ) 
{
  std::cout << "Generating " << num_datapoints << " readings"
    << " for " << sensors.size() << " sensors." << std::endl;
  for( klio::sensors_cit_t it = sensors.begin(); it < sensors.end(); ++it) {
    klio::Sensor::Ptr current=(*it);
    for(uint16_t i = 0; i < num_datapoints; i++) {
      store->add_reading(current, i, i);
    }
  }
}

BOOST_AUTO_TEST_CASE ( check_collate ) {
  std::cout << std::endl << "*** Testing collate algorithm." << std::endl;
  try {
    klio::Store::Ptr store(generate_store());
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
    klio::Sensor::Ptr sensor1(sensor_factory->createSensor("sensor1", "sensor1", "Watt", "Europe/Berlin")); 
    store->add_sensor(sensor1);
    std::cout << "Created: " << sensor1->str() << std::endl;
    klio::Sensor::Ptr sensor2(sensor_factory->createSensor("sensor2", "sensor2", "Watt", "Europe/Berlin")); 
    store->add_sensor(sensor2);
    std::cout << "Created: " << sensor2->str() << std::endl;
    klio::Sensor::Ptr sensor3(sensor_factory->createSensor("sensor3", "sensor3", "Watt", "Europe/Berlin")); 
    store->add_sensor(sensor3);
    std::cout << "Created: " << sensor3->str() << std::endl;

    klio::sensors_t sensors;
    sensors.push_back(sensor1);
    sensors.push_back(sensor2);
    sensors.push_back(sensor3);

    fill_symmetric_test_data(store, sensors, 10);
    klio::sensordata_table_t table(klio::collate(store, sensors));
    klio::render_sensordata_array(std::cout, table.get<0>());

    std::cout << "Printing first data row: timestamp plus 3 sensor values" 
      << std::endl;
    std::vector<double> data(klio::get_sensordata_row(table, 0));
    std::vector<double>::iterator it;
    for(it = data.begin(); it != data.end(); ++it) {
      std::cout << (*it) << "\t";
    }
    std::cout <<std::endl;

    // provoke dataformat exception
    try {
      store->add_reading(sensor1, 1212, 1212.0);
      klio::collate(store, sensors);
      BOOST_FAIL( "Dimension check failed, but no exception thrown" );
    } catch (klio::DataFormatException& dfe) {
      std::cout << "Caught expected dataformat exception." << std::endl;
    }
  } catch (std::exception const& ex) {
    BOOST_FAIL( "Unexpected exception occured during sensor test" );
  }

  //BOOST_CHECK_EQUAL (timestamp, reversed_ts);
}
