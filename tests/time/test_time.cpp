/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>, 2011
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
#include <libklio/time.hpp>


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

BOOST_AUTO_TEST_CASE ( get_day_of_year ) {
  std::cout << std::endl << "*** Checking get_day_of_year" << std::endl;
  klio::TimeConverter::Ptr tc(new klio::TimeConverter());

  //Timestamp: 1357039094
  //UTC: Jul 24, 2012 12:18:14 PM
  klio::timestamp_t timestamp = 1357039094;
  std::string reversed_local = tc->str_local(timestamp);

  std::cout << "Timestamp: " << timestamp 
    << " -> reversed local: " << reversed_local << std::endl;
  //BOOST_REQUIRE( reversed_local != reversed_utc ); 
}


//BOOST_AUTO_TEST_SUITE_END()
