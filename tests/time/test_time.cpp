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
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <libklio/time.hpp>
#include <libklio/local-time.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensor-factory.hpp>


BOOST_AUTO_TEST_CASE ( check_time ) {
  std::cout << std::endl << "*** Checking time (& space, hrhrr)." << std::endl;
  klio::TimeConverter::Ptr tc(new klio::TimeConverter());

  klio::timestamp_t timestamp = tc->get_timestamp();
  long epochtime = tc->convert_to_epoch(timestamp);
  klio::timestamp_t reversed_ts = tc->convert_from_epoch(epochtime);
  std::cout << "Timestamp: " << tc->str_local(timestamp) << " -> epoch: " <<
    epochtime << std::endl << " -> reversed timestamp: " 
    << tc->str_local(reversed_ts) << std::endl;
  BOOST_CHECK_EQUAL (timestamp, reversed_ts);
}

BOOST_AUTO_TEST_CASE ( check_timezone ) {
  std::cout << std::endl << "*** Checking time (& space, hrhrr)." << std::endl;
  klio::TimeConverter::Ptr tc(new klio::TimeConverter());

  klio::timestamp_t timestamp = tc->get_timestamp();
  long epochtime = tc->convert_to_epoch(timestamp);
  std::string reversed_local = tc->str_local(epochtime);
  std::string reversed_utc = tc->str_utc(epochtime);

  std::cout << "Timestamp: " << timestamp << " -> epoch: " <<
    epochtime << std::endl << " -> reversed utc: " << reversed_utc 
    << " -> reversed local: " << reversed_local << std::endl;
  BOOST_REQUIRE( reversed_local != reversed_utc ); 
}

BOOST_AUTO_TEST_CASE ( check_timezones ) {
  std::cout << std::endl << "*** Checking valid timezone behavior" << std::endl;
  klio::LocalTime::Ptr lt(new klio::LocalTime("../.."));
  BOOST_REQUIRE(lt->is_valid_timezone("America/New_York"));
  BOOST_REQUIRE(! lt->is_valid_timezone("Horst"));
}

BOOST_AUTO_TEST_CASE ( local_time ) {
  using namespace boost::gregorian; 
  using namespace boost::local_time;
  using namespace boost::posix_time;

  std::cout << std::endl << "*** Checking new LocalTime class." << std::endl;

  klio::LocalTime::Ptr lt(new klio::LocalTime("../.."));
  klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
  std::string sensor_id("sensor_id");
  std::string sensor_unit("unit");
  std::string sensor_timezone("America/New_York");
  klio::Sensor::Ptr sensor(sensor_factory->createSensor(
        sensor_id, sensor_id, sensor_unit, sensor_timezone)); 

  try {
    klio::Sensor::Ptr broken_sensor(sensor_factory->createSensor(
          sensor_id, sensor_id, sensor_unit, "HORST")); 
    BOOST_FAIL("Created sensor with invalid timezone HORST");
  } catch (klio::DataFormatException& dfe) {
    std::cout << "Expected Exception: " << dfe.what() << std::endl;
  }

  time_zone_ptr nyc_tz = lt->get_timezone_ptr(sensor);
  std::cout << "Using time zone " << nyc_tz->std_zone_abbrev() << std::endl;
  //date in_date(2004,10,04);
  //time_duration td(12,14,32);
  time_t demotime=1096906472;
  local_date_time nyc_time = lt->get_local_time(sensor, demotime);
  std::cout << "NYC Time: " << nyc_time << std::endl;
  ptime nyc_utc = lt->get_utc_time(sensor, demotime);
  std::cout << "NYC Time (UTC): " << nyc_utc << std::endl;
  //Expected 1096906472
  std::cout << "NYC time unix timestamp: " << lt->get_timestamp(sensor, nyc_time) << std::endl;
  BOOST_REQUIRE( lt->get_timestamp(sensor, nyc_time) == demotime ); 

  ptime ptime_t_epoch(date(1970,1,1)); 
  std::cout << "Epoch: " << ptime_t_epoch << std::endl;
  std::cout << "Epoch as timestamp: " << lt->get_timestamp(ptime_t_epoch) << std::endl;
  BOOST_REQUIRE( lt->get_timestamp(ptime_t_epoch) == 0 ); 
  std::cout << "NYC Date: " << lt->get_local_date(sensor, demotime) << std::endl;
  std::cout << "Day of year: " << lt->get_local_day_of_year(sensor, demotime) << std::endl;
  BOOST_REQUIRE( lt->get_local_day_of_year(sensor, demotime) == 278 ); 
  std::cout << "Hours since midnight: " << lt->get_local_hour(sensor, demotime) << std::endl;
  BOOST_REQUIRE( lt->get_local_hour(sensor, demotime) == 12 ); 

}

//BOOST_AUTO_TEST_SUITE_END()
