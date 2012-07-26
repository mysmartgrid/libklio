#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <iostream>
#include <libklio/sensor.hpp>
#include <libklio/sensorfactory.hpp>
#include <libklio/local_time.hpp>


//http://www.crystalclearsoftware.com/libraries/date_time/release_1_33/date_time/examples.html
int main (int argc, char const* argv[]) {
  using namespace boost::gregorian; 
  using namespace boost::local_time;
  using namespace boost::posix_time;

  
  klio::LocalTime::Ptr lt(new klio::LocalTime(argv[0]));
  klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());
  std::string sensor_id("sensor_id");
  std::string sensor_unit("unit");
  std::string sensor_timezone("America/New_York");
  klio::Sensor::Ptr sensor(sensor_factory->createSensor(
        sensor_id, sensor_unit, sensor_timezone)); 

  time_zone_ptr nyc_tz = lt->get_timezone_ptr(sensor);
  std::cout << "Using time zone " << nyc_tz->std_zone_abbrev() << std::endl;
  //date in_date(2004,10,04);
  //time_duration td(12,14,32);
  time_t demotime=1096906472;
  // construct with local time value
  // create not-a-date-time if invalid (eg: in dst transition)
  //local_date_time nyc_time(in_date, 
  //    td, 
  //    nyc_tz, 
  //    local_date_time::NOT_DATE_TIME_ON_ERROR);

  local_date_time nyc_time = lt->get_local_time(sensor, demotime);
  std::cout << "NYC Time: " << nyc_time << std::endl;
  ptime nyc_utc = lt->get_utc_time(sensor, demotime);
  std::cout << "NYC Time (UTC): " << nyc_utc << std::endl;
  //Expected 1096906472
  std::cout << "NYC time unix timestamp: " << lt->get_timestamp(sensor, nyc_time) << std::endl;

  ptime ptime_t_epoch(date(1970,1,1)); 
  std::cout << "Epoch: " << ptime_t_epoch << std::endl;
  std::cout << "Epoch as timestamp: " << lt->get_timestamp(ptime_t_epoch) << std::endl;
  std::cout << "NYC Date: " << lt->get_local_date(sensor, demotime) << std::endl;
  std::cout << "Day of year: " << lt->get_local_day_of_year(sensor, demotime) << std::endl;
  std::cout << "Hours since midnight: " << lt->get_local_hour(sensor, demotime) << std::endl;

  //time_duration nyc_time_of_day(nyc_ptime.time_of_day());
  //std::cout << "Hours since midnight: " << nyc_time_of_day.hours() << std::endl;

}

