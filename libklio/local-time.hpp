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

#ifndef LIBKLIO_LOCAL_TIME_HPP
#define LIBKLIO_LOCAL_TIME_HPP 1

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <libklio/types.hpp>


namespace bpt = boost::posix_time;
namespace bg = boost::gregorian;

using namespace boost::gregorian;
using namespace boost::local_time;
using namespace boost::posix_time;

namespace klio {

    class LocalTime {
    public:
        typedef boost::shared_ptr<LocalTime> Ptr;
        LocalTime(const char* cmd);

        virtual ~LocalTime() {
        };

        std::vector<std::string> get_valid_timezones();
        const bool is_valid_timezone(const std::string& zone);
        const time_zone_ptr get_timezone_ptr(const Sensor::Ptr sensor);

        const local_date_time get_local_time(const Sensor::Ptr sensor, const timestamp_t time);
        const bpt::ptime get_utc_time(const Sensor::Ptr sensor, const timestamp_t time);
        const bg::date get_local_date(const Sensor::Ptr sensor, const timestamp_t time);
        const uint16_t get_local_day_of_year(const Sensor::Ptr sensor, const timestamp_t time);
        const uint16_t get_local_hour(const Sensor::Ptr sensor, const timestamp_t time);

        const timestamp_t get_timestamp(const ptime ptime);
        const timestamp_t get_timestamp(const Sensor::Ptr sensor, const local_date_time time);

    private:
        LocalTime(const LocalTime& original);
        LocalTime& operator=(const LocalTime& rhs);

        tz_database _tz_db;
    };
};

#endif /* LIBKLIO_LOCAL_TIME_HPP */
