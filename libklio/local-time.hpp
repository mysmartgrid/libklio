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


namespace klio {

    class LocalTime {
    public:
        typedef std::tr1::shared_ptr<LocalTime> Ptr;
        LocalTime(const char* cmd);

        virtual ~LocalTime() {
        };

        std::vector<std::string> get_valid_timezones();
        bool is_valid_timezone(const std::string& zone);

        boost::local_time::time_zone_ptr
        get_timezone_ptr(klio::Sensor::Ptr sensor);

        boost::local_time::local_date_time
        get_local_time(klio::Sensor::Ptr sensor, klio::timestamp_t time);

        boost::posix_time::ptime
        get_utc_time(klio::Sensor::Ptr sensor, klio::timestamp_t time);

        boost::gregorian::date
        get_local_date(klio::Sensor::Ptr sensor, klio::timestamp_t time);

        uint16_t
        get_local_day_of_year(klio::Sensor::Ptr sensor, klio::timestamp_t time);
        uint16_t
        get_local_hour(klio::Sensor::Ptr sensor, klio::timestamp_t time);

        klio::timestamp_t
        get_timestamp(boost::posix_time::ptime ptime);

        klio::timestamp_t
        get_timestamp(klio::Sensor::Ptr sensor, boost::local_time::local_date_time time);

    private:
        LocalTime(const LocalTime& original);
        LocalTime& operator=(const LocalTime& rhs);
        boost::local_time::tz_database _tz_db;

    };
};

#endif /* LIBKLIO_LOCAL_TIME_HPP */
