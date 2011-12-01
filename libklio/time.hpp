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

#include <libklio/common.hpp>
#include <ctime>

namespace klio {
  // We use unix timestamps to represent time in this library.
  typedef time_t timestamp_t;
  class TimeConverter {
    public:
      typedef std::tr1::shared_ptr<TimeConverter> Ptr;
      TimeConverter () {};
      virtual ~TimeConverter() {};

      timestamp_t get_timestamp();
      long convert_to_epoch(klio::timestamp_t time);
      timestamp_t convert_from_epoch(long epoch);
      std::string str_local(timestamp_t time);
      std::string str_utc(timestamp_t time);

    private:
      TimeConverter (const TimeConverter& original);
      TimeConverter& operator= (const TimeConverter& rhs);

  };

};


#endif /* LIBKLIO_TIME_HPP */

