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
#ifndef LIBKLIO_TIME_HPP
#define LIBKLIO_TIME_HPP 1


#include <boost/date_time/posix_time/posix_time.hpp>

namespace klio {
  // We use unix timestamps to represent time in this library.
  typedef boost::posix_time::ptime timestamp_t;
  timestamp_t get_timestamp();
  long convert_to_epoch(klio::timestamp_t time);
};


#endif /* LIBKLIO_TIME_HPP */

