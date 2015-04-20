# -*- mode: cmake; -*-
# - Try to find PostgreSQL include dirs and libraries
# Usage of this module as follows:
# This file defines:
# * POSTGRESQL_FOUND if protoc was found
# * POSTGRESQL_LIBRARY The lib to link to (currently only a static unix lib, not
# portable)
# * POSTGRESQL_INCLUDE The include directories for PostgreSQL.

cmake_policy(PUSH)
# when crosscompiling import the executable targets from a file
if(CMAKE_CROSSCOMPILING)
#  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY FIRST)
#  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE FIRST)
endif(CMAKE_CROSSCOMPILING)


message(STATUS "FindPostgreSQL check")

IF (NOT WIN32)
  include(FindPkgConfig)
  if ( PKG_CONFIG_FOUND )
    pkg_check_modules (PC_POSTGRESQL pq)
    set(POSTGRESQL_DEFINITIONS ${PC_POSTGRESQL_CFLAGS_OTHER})
  endif(PKG_CONFIG_FOUND)
endif (NOT WIN32)

#
# set defaults
SET(_postgresql_HOME "/usr/local")
SET(_postgresql_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_postgresql_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${POSTGRESQL_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{POSTGRESQL_HOME}")
    message(STATUS "POSTGRESQL_HOME env is not set, setting it to /usr")
    set (POSTGRESQL_HOME ${_postgresql_HOME})
  else("" MATCHES "$ENV{POSTGRESQL_HOME}")
    set (POSTGRESQL_HOME "$ENV{POSTGRESQL_HOME}")
  endif("" MATCHES "$ENV{POSTGRESQL_HOME}")
endif( "${POSTGRESQL_HOME}" STREQUAL "")
message(STATUS "Looking for libpq in ${POSTGRESQL_HOME}")

##
IF( NOT ${POSTGRESQL_HOME} STREQUAL "" )
    SET(_postgresql_INCLUDE_SEARCH_DIRS ${POSTGRESQL_HOME}/include ${_postgresql_INCLUDE_SEARCH_DIRS})
    SET(_postgresql_LIBRARIES_SEARCH_DIRS ${POSTGRESQL_HOME}/lib ${_postgresql_LIBRARIES_SEARCH_DIRS})
    SET(_postgresql_HOME ${POSTGRESQL_HOME})
ENDIF( NOT ${POSTGRESQL_HOME} STREQUAL "" )

IF( NOT $ENV{POSTGRESQL_INCLUDEDIR} STREQUAL "" )
  SET(_postgresql_INCLUDE_SEARCH_DIRS $ENV{POSTGRESQL_INCLUDEDIR} ${_postgresql_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{POSTGRESQL_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{POSTGRESQL_LIBRARYDIR} STREQUAL "" )
  SET(_postgresql_LIBRARIES_SEARCH_DIRS $ENV{POSTGRESQL_LIBRARYDIR} ${_postgresql_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{POSTGRESQL_LIBRARYDIR} STREQUAL "" )

IF( POSTGRESQL_HOME )
  SET(_postgresql_INCLUDE_SEARCH_DIRS ${POSTGRESQL_HOME}/include ${_postgresql_INCLUDE_SEARCH_DIRS})
  SET(_postgresql_LIBRARIES_SEARCH_DIRS ${POSTGRESQL_HOME}/lib ${_postgresql_LIBRARIES_SEARCH_DIRS})
  SET(_postgresql_HOME ${POSTGRESQL_HOME})
ENDIF( POSTGRESQL_HOME )

message(STATUS "root_path: ${CMAKE_FIND_ROOT_PATH}")

find_path(POSTGRESQL_INCLUDE_DIR postgresql/libpq-fe.h
  HINTS
     ${_postgresql_INCLUDE_SEARCH_DIRS}
     ${PC_POSTGRESQL_INCLUDEDIR}
     ${PC_POSTGRESQL_INCLUDE_DIRS}
     ${CMAKE_INCLUDE_PATH}
)

# locate the library
SET(POSTGRESQL_LIBRARY_NAMES ${POSTGRESQL_LIBRARY_NAMES} pq)
find_library(POSTGRESQL_LIBRARY NAMES ${POSTGRESQL_LIBRARY_NAMES}
  HINTS
    ${_postgresql_LIBRARIES_SEARCH_DIRS}
    ${PC_POSTGRESQL_LIBDIR}
    ${PC_POSTGRESQL_LIBRARY_DIRS}
)

#
message(STATUS "    Found ${_name}: ${POSTGRESQL_INCLUDE_DIRS} ${POSTGRESQL_LIBRARY}")
get_filename_component (POSTGRESQL_LIBRARY_DIR ${POSTGRESQL_LIBRARY} PATH)

if( NOT postgresql_IN_CACHE )
  if(EXISTS ${POSTGRESQL_LIBRARY_DIR}/shared/postgresqlConfig.cmake)
    message(STATUS "    Include POSTGRESQL dependencies.")
    include(${POSTGRESQL_LIBRARY_DIR}/shared/postgresqlConfig.cmake)
    set(POSTGRESQL_LIBRARY postgresql )
    set(POSTGRESQL_INCLUDE_DIRS ${POSTGRESQL_INCLUDE_DIRS} )
  endif(EXISTS ${POSTGRESQL_LIBRARY_DIR}/shared/postgresqlConfig.cmake)
else( NOT postgresql_IN_CACHE )
  message(STATUS "    package ${NAME} was allready in loaded. Do not perform dependencies.")
endif( NOT postgresql_IN_CACHE )

list(APPEND POSTGRESQL_LIBRARY "rt")

message(STATUS "    POSTGRESQL 2: '-I${POSTGRESQL_INCLUDE_DIR}' '-L${POSTGRESQL_LIBRARY_DIR}' ")
message(STATUS "             '${POSTGRESQL_LIBRARIES}' '${POSTGRESQL_LIBRARY}'")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PostgreSQL DEFAULT_POSTGRESQL POSTGRESQL_LIBRARY_NAMES POSTGRESQL_INCLUDE_DIR)

# if the include and the program are found then we have it
IF(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARY)
  SET(POSTGRESQL_FOUND "YES")
ENDIF(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARY)

# if( NOT WIN32)
#   list(APPEND POSTGRESQL_LIBRARY "-lrt")
# endif( NOT WIN32)

MARK_AS_ADVANCED(
  POSTGRESQL_FOUND
  POSTGRESQL_LIBRARY
  POSTGRESQL_INCLUDE_DIR
  POSTGRESQL_INCLUDE_DIRS
)
if(CMAKE_CROSSCOMPILING)
#  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif(CMAKE_CROSSCOMPILING)
cmake_policy(POP)
