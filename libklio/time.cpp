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
#include "time.hpp"
#include "boost/date_time/local_time/local_time.hpp"

using namespace boost::posix_time;
using namespace boost::local_time;
using namespace boost::gregorian;

klio::timestamp_t klio::get_timestamp() {
  return second_clock::local_time();
}

long klio::convert_to_epoch(klio::timestamp_t time) {
  timestamp_t time_t_epoch(date(1970,1,1));
  time_zone_ptr zone(new posix_time_zone("MST-07"));
  local_date_time local_time(time, zone);
  return (local_time.utc_time() - time_t_epoch).total_seconds();
}

klio::timestamp_t klio::convert_from_epoch(long epoch) {
  ptime pt(not_a_date_time);
  std::time_t t=epoch;
  pt = from_time_t(t);
  return pt; 
}

