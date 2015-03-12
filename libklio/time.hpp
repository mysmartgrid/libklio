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

#ifndef LIBKLIO_TIME_HPP
#define LIBKLIO_TIME_HPP 1

#include <ctime>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>
#include <libklio/types.hpp>


namespace klio {

    class TimeConverter {
    public:
        typedef boost::shared_ptr<TimeConverter> Ptr;

        TimeConverter() {
        };

        virtual ~TimeConverter() {
        };

        const timestamp_t get_timestamp();
        const long convert_to_epoch(const timestamp_t time);
        const timestamp_t convert_from_epoch(const long epoch);
        const std::string str_local(const timestamp_t time);
        const std::string str_utc(const timestamp_t time);

    private:
        TimeConverter(const TimeConverter& original);
        TimeConverter& operator=(const TimeConverter& rhs);
    };
};

#endif /* LIBKLIO_TIME_HPP */
