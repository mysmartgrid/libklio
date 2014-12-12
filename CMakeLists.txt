# -*- mode: cmake; -*-
project(libklio)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

OPTION(ENABLE_LOGGING
  "Build the project with logging enabled"
  OFF)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 5)
set(V_PATCH 6)

# use ctest
include(CTest)
enable_testing()

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH "$ENV{CMAKE_ADDITIONAL_PATH}" CACHE PATH "Path where many locally installed libraries can be found")

OPTION(ENABLE_ROCKSDB
  "Build the project with rockskdb enabled"
  ON)
OPTION(ENABLE_MSG
  "Build the project with mysmartgrid enabled"
  ON)
OPTION(ENABLE_REDIS3M
  "Build the project with redis3m enabled"
  ON)
OPTION(ENABLE_POSTGRESQL
  "Build the project with PostgreSQL enabled"
  ON)

################################################################
  ###################################
# No user settings below
###################################################################################################

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/modules ${CMAKE_MODULE_PATH})

include (CheckCompiler)
info_compiler()

include (UseCodeCoverage)

add_definitions(-std=c++0x)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wno-non-virtual-dtor -g -fomit-frame-pointer")

set(CMAKE_CXX_FLAGS_EXPERIMENTAL "-O0 -g -ggdb -Wall  -W -Wshadow")
#set(CMAKE_CXX_FLAGS_EXPERIMENTAL "-O0 -g -ggdb -Wall -Werror -W -Wshadow")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-variable")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-parameter")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused-function")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wunused -Woverloaded-virtual")
#set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wno-system-headers")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -Wno-non-virtual-dtor")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -pg -fprofile-generate ")
set(CMAKE_CXX_FLAGS_EXPERIMENTAL "${CMAKE_CXX_FLAGS_EXPERIMENTAL} -fprofile-arcs -ftest-coverage")

# gprof and gcov support
set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -W -Wreturn-type -Wno-shadow")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wunused-variable -Wunused-parameter")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wunused-function -Wunused")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Woverloaded-virtual -Wno-system-headers")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-non-virtual-dtor")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RROFILE} -pg -fprofile-arcs")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -ftest-coverage")

# Releases are made with the release build. Optimize code and strip binaries.
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-non-virtual-dtor")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-but-set-variable")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-variable")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-sign-compare")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-uninitialized")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-attributes")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror")
endif()

# always set these flags
set(CMAKE_CXX_FLAGS "-Wall")

FIND_PACKAGE(Sqlite)
# We depend on boost >= 1.46.1 because of the updated filesystem
# library. The boost version detection is a mess, see 
# $ cmake --help-module FindBoost
# , section " The mess that is Boost_ADDITIONAL_VERSIONS (sorry?)"
SET(Boost_ADDITIONAL_VERSIONS "1.46.1" "1.47" "1.47.1" "1.48.0" "1.48"
  "1.49.0" "1.49" "1.50.0" "1.50")
set (Boost_USE_STATIC_LIBS ON)
SET(Boost_DETAILED_FAILURE_MSG true)
FIND_PACKAGE(Boost 1.46.1 
  COMPONENTS test_exec_monitor program_options filesystem system date_time)

# RocksDB
# check if the compiler version supports c++ 2011
if( ENABLE_ROCKSDB )
  if( ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.6")  
    include(FindRocksDB)
    if(ROCKSDB_FOUND)
      include_directories(${ROCKSDB_INCLUDE_DIR})
    else(ROCKSDB_FOUND)
      set(ROCKSDB_INCLUDE_DIR "")
      message(STATUS "RocksDB support is disabled because the library is not found. Install RocksDB or call cmake -DROCKSDB_HOME=path_to_rocksdb_install in order to enable it.")
      set(ENABLE_ROCKSDB False)
    endif(ROCKSDB_FOUND)
  else()
    set(ENABLE_ROCKSDB False)
  endif()
endif()

if( ENABLE_MSG )
  include(FindLibMysmartGrid)
  if( NOT LIBMYSMARTGRID_FOUND)
    message(STATUS "libmysmartgrid is required.
            Install libmysmartgrid or call cmake -DLIBMYSMARTGRID_HOME=path_to_libmysmartgrid_install")
    set(ENABLE_MSG False)
  else()
    include(FindJsonCpp)
    if( NOT JSONCPP_FOUND)
      message(STATUS "jsoncpp is required.
              Install jsoncpp or call cmake -DJSONCPP_HOME=path_to_jsoncpp_install")
      set(ENABLE_MSG False)
    endif( NOT JSONCPP_FOUND)
  endif( NOT LIBMYSMARTGRID_FOUND)
endif()

if( ENABLE_REDIS3M )
  include(FindRedis3m)
  if( NOT REDIS3M_FOUND)
    message(STATUS "redis3m is required.
            Install redis3m or call cmake -DREDIS3M_HOME=path_to_redis3m_install")
    set(ENABLE_REDIS3M Off)
  endif( NOT REDIS3M_FOUND)
endif()

if( ENABLE_POSTGRESQL )
  include(FindPostgreSQL)
  if( NOT POSTGRESQL_FOUND)
    message(STATUS "PostgreSQL is required.
                    Install PostgreSQL or call cmake -DPOSTGRESQL_HOME=path_to_postgresql_install")
    set(ENABLE_POSTGRESQL Off)
  endif( NOT POSTGRESQL_FOUND)
endif()

# this file is installed and contains all necessary information about the installed libseda,
# it also loads the file with the exported targets
configure_file(${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

include_directories(${CMAKE_BINARY_DIR})

# Check for some compiler stuff
include(CMakeBackwardCompatibilityCXX)

# Reflect the package structure
add_subdirectory(src)
add_subdirectory(libklio)
add_subdirectory(tests)

# add some files to the installation target
INSTALL(FILES README.md LICENSE 
    DESTINATION share/doc/libklio
  )
INSTALL(FILES share/libklio/date_time_zonespec.csv 
    DESTINATION share/libklio
  )
configure_file(share/libklio/date_time_zonespec.csv 
  ${CMAKE_BINARY_DIR}/
  )

install(EXPORT ${PROJECT_NAME}Exports
    DESTINATION lib/shared
    FILE ${PROJECT_NAME}Targets.cmake
    COMPONENT libraries)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/shared
  COMPONENT libraries)

# CPACK packaging
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_GENERATOR "DEB" "STGZ" "TGZ" "TBZ2")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "klio library")
SET(CPACK_PACKAGE_VENDOR "Fraunhofer ITWM")
set(CPACK_PACKAGE_CONTACT "Fraunhofer ITWM <support@mysmartgrid.de>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_NAME "libklio")
SET(CPACK_PACKAGE_VERSION_MAJOR ${V_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${V_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${V_PATCH})

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libstdc++6 (>= 4.6.0), libc6 (>= 2.13)")

#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_PACKAGE_EXECUTABLES "klio-sensor" "Functions for handling sensors")
SET(CPACK_PACKAGE_EXECUTABLES "klio-store" "Tool for dealing with sensor data stores")
set(CPACK_STRIP_FILES "usr/bin/klio-import;usr/bin/klio-export;usr/bin/klio-sensor;usr/bin/klio-store")
SET(CPACK_SOURCE_STRIP_FILES "")
SET(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_SECTION "net")
include(CPackTools)


### print some output for the user
message("")
message("***** Configuration *****")
message("  prefix: ${CMAKE_INSTALL_PREFIX}")
message("  boost include: ${Boost_INCLUDE_DIRS}")
message("  boost libraries shortcut: ${Boost_LIBRARIES}")
message("  boost program options lib: ${Boost_PROGRAM_OPTIONS_LIBRARY}")
message("  boost system lib: ${Boost_SYSTEM_LIBRARY}")
message("  boost filesystem lib: ${Boost_FILESYSTEM_LIBRARY}")
message("  libmysmartgrid include: ${LIBMYSMARTGRID_INCLUDE_DIR}, lib: ${LIBMYSMARTGRID_LIBRARY}")
message("  jsoncpp include: ${JSONCPP_INCLUDE_DIR}, lib: ${JSONCPP_LIBRARY}")
message("  sqlite3 include: ${SQLITE3_INCLUDE_DIR}, lib: ${SQLITE3_LIBRARIES}")
message("  rocksdb include: ${ROCKSDB_INCLUDE_DIR}, lib: ${ROCKSDB_STATIC_LIBRARIES}")
message("  redis3m include: ${REDIS3M_INCLUDE_DIR}, lib: ${REDIS3M_LIBRARY}")
message("  PostgreSQL include: ${POSTGRESQL_INCLUDE_DIR}, lib: ${POSTGRESQL_LIBRARY}")
message("  extended logging: ${ENABLE_LOGGING}")
message("  rocksdb: ${ENABLE_ROCKSDB}")
message("  redis3m: ${ENABLE_REDIS3M}")
message("  PostgreSQL: ${ENABLE_POSTGRESQL}")
message("  mysmartgrid: ${ENABLE_MSG}")

