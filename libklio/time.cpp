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

#include <sstream>
#include "time.hpp"


using namespace klio;

const timestamp_t TimeConverter::get_timestamp() {

    std::time_t rawtime;
    std::time(&rawtime);
    return rawtime;
}

const long TimeConverter::convert_to_epoch(const timestamp_t time) {
    //TODO
    return time;
}

const timestamp_t TimeConverter::convert_from_epoch(const long epoch) {
    //TODO
    return epoch;
}

const std::string TimeConverter::str_local(const timestamp_t time) {
    struct tm* timeinfo;
    timeinfo = localtime(&time);

    std::ostringstream oss;
    oss << asctime(timeinfo);
    return oss.str();
}

const std::string TimeConverter::str_utc(const timestamp_t time) {
    struct tm* timeinfo;
    timeinfo = gmtime(&time);

    std::ostringstream oss;
    oss << asctime(timeinfo);
    return oss.str();
}
