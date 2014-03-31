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

#ifndef LIBKLIO_COMMON_HPP
#define LIBKLIO_COMMON_HPP 1

#include <boost/shared_ptr.hpp>

/* Include TR1 shared ptrs in a portable way. */
#include <cstddef> // for __GLIBCXX__
#ifdef __GLIBCXX__
#include <tr1/memory>
#else
#ifdef __IBMCPP__
#define __IBMCPP_TR1__
#endif
#include <memory>
#endif

//#define ENABLE_LOGGING 0
#include <libklio/config.h>

#ifdef ENABLE_LOGGING
#include <iostream>
#define LOG(msg) std::cout << msg << std::endl;
#else
#define LOG(msg) 
#endif

// Use new boost filesystem implementation. See 
// http://www.boost.org/doc/libs/1_49_0/libs/filesystem/v3/doc/index.htm
#define BOOST_FILESYSTEM_VERSION 3

// See http://stackoverflow.com/questions/15234527/boost-1-53-local-date-time-compiler-error-with-std-c0x
//#define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS 1

#include <libklio/error.hpp>
#include <stdint.h>
#include <string>

namespace klio {

    class VersionInfo {
    public:
        typedef boost::shared_ptr<VersionInfo> Ptr;

        VersionInfo() {
        };

        virtual ~VersionInfo() {
        };
        const std::string getVersion();

    private:
        VersionInfo(const VersionInfo& original);
        VersionInfo& operator=(const VersionInfo& rhs);

    };

};

#endif /* LIBKLIO_COMMON_HPP */
