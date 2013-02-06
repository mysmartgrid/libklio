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

#ifndef LIBKLIO_TYPES_HPP
#define LIBKLIO_TYPES_HPP 1

#include <map>
#include <vector>
#include <boost/multi_array.hpp>
#include <boost/tuple/tuple.hpp>
#include <libklio/common.hpp>
#include <libklio/sensor.hpp>


namespace klio {

    // We use unix timestamps to represent time in this library.
    typedef time_t timestamp_t;

    // Basic types for representing time series
    typedef std::map<klio::timestamp_t, double> readings_t;
    typedef std::tr1::shared_ptr< readings_t > readings_t_Ptr;
    typedef std::map<klio::timestamp_t, double>::iterator readings_it_t;
    typedef std::map<klio::timestamp_t, double>::const_iterator readings_cit_t;
    typedef std::pair<klio::timestamp_t, double> reading_t;

    // collection of sensors. The order of the sensors is important!
    typedef std::vector<klio::Sensor::Ptr> sensors_t;
    typedef std::vector<klio::Sensor::Ptr>::const_iterator sensors_cit_t;

    // "tables" of sensor data. 
    typedef boost::multi_array<double, 2 > sensordata_array_t;
    typedef sensordata_array_t::index sensordata_array_idx_t;
    typedef boost::tuple<sensordata_array_t, unsigned long int, unsigned long int> sensordata_table_t;

}

#endif /* LIBKLIO_TYPES_HPP */
