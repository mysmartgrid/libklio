# -*- mode: cmake; -*-
# - Try to find redis3m include dirs and libraries
# Usage of this module as follows:
# This file defines:
# * REDIS3M_FOUND if protoc was found
# * REDIS3M_LIBRARY The lib to link to (currently only a static unix lib, not
# portable)
# * REDIS3M_INCLUDE The include directories for redis3m.

cmake_policy(PUSH)
# when crosscompiling import the executable targets from a file
if(CMAKE_CROSSCOMPILING)
#  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY FIRST)
#  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE FIRST)
endif(CMAKE_CROSSCOMPILING)


message(STATUS "FindRedis3m check")

IF (NOT WIN32)
  include(FindPkgConfig)
  if ( PKG_CONFIG_FOUND )
    pkg_check_modules (PC_REDIS3M redis3m)
    set(REDIS3M_DEFINITIONS ${PC_REDIS3M_CFLAGS_OTHER})
  endif(PKG_CONFIG_FOUND)
endif (NOT WIN32)

#
# set defaults
SET(_redis3m_HOME "/usr/local")
SET(_redis3m_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_redis3m_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${REDIS3M_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{REDIS3M_HOME}")
    message(STATUS "REDIS3M_HOME env is not set, setting it to /usr/local")
    set (REDIS3M_HOME ${_redis3m_HOME})
  else("" MATCHES "$ENV{REDIS3M_HOME}")
    set (REDIS3M_HOME "$ENV{REDIS3M_HOME}")
  endif("" MATCHES "$ENV{REDIS3M_HOME}")
endif( "${REDIS3M_HOME}" STREQUAL "")
message(STATUS "Looking for redis3m in ${REDIS3M_HOME}")

##
IF( NOT ${REDIS3M_HOME} STREQUAL "" )
    SET(_redis3m_INCLUDE_SEARCH_DIRS ${REDIS3M_HOME}/include ${_redis3m_INCLUDE_SEARCH_DIRS})
    SET(_redis3m_LIBRARIES_SEARCH_DIRS ${REDIS3M_HOME}/lib ${_redis3m_LIBRARIES_SEARCH_DIRS})
    SET(_redis3m_HOME ${REDIS3M_HOME})
ENDIF( NOT ${REDIS3M_HOME} STREQUAL "" )

IF( NOT $ENV{REDIS3M_INCLUDEDIR} STREQUAL "" )
  SET(_redis3m_INCLUDE_SEARCH_DIRS $ENV{REDIS3M_INCLUDEDIR} ${_redis3m_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{REDIS3M_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{REDIS3M_LIBRARYDIR} STREQUAL "" )
  SET(_redis3m_LIBRARIES_SEARCH_DIRS $ENV{REDIS3M_LIBRARYDIR} ${_redis3m_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{REDIS3M_LIBRARYDIR} STREQUAL "" )

IF( REDIS3M_HOME )
  SET(_redis3m_INCLUDE_SEARCH_DIRS ${REDIS3M_HOME}/include ${_redis3m_INCLUDE_SEARCH_DIRS})
  SET(_redis3m_LIBRARIES_SEARCH_DIRS ${REDIS3M_HOME}/lib ${_redis3m_LIBRARIES_SEARCH_DIRS})
  SET(_redis3m_HOME ${REDIS3M_HOME})
ENDIF( REDIS3M_HOME )

message(STATUS "root_path: ${CMAKE_FIND_ROOT_PATH}")

find_path(REDIS3M_INCLUDE_DIR redis3m/connection.h
  HINTS
     ${_redis3m_INCLUDE_SEARCH_DIRS}
     ${PC_REDIS3M_INCLUDEDIR}
     ${PC_REDIS3M_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
)

# locate the library
SET(REDIS3M_LIBRARY_NAMES ${REDIS3M_LIBRARY_NAMES} redis3m)
find_library(REDIS3M_LIBRARY NAMES ${REDIS3M_LIBRARY_NAMES}
  HINTS
    ${_redis3m_LIBRARIES_SEARCH_DIRS}
    ${PC_REDIS3M_LIBDIR}
    ${PC_REDIS3M_LIBRARY_DIRS}
)

#
message(STATUS "    Found ${_name}: ${REDIS3M_INCLUDE_DIRS} ${REDIS3M_LIBRARY}")
get_filename_component (REDIS3M_LIBRARY_DIR ${REDIS3M_LIBRARY} PATH)

if( NOT redis3m_IN_CACHE )
  if(EXISTS ${REDIS3M_LIBRARY_DIR}/shared/redis3mConfig.cmake)
    message(STATUS "    Include REDIS3M dependencies.")
    include(${REDIS3M_LIBRARY_DIR}/shared/redis3mConfig.cmake)
    set(REDIS3M_LIBRARY redis3m )
    set(REDIS3M_INCLUDE_DIRS ${REDIS3M_INCLUDE_DIRS} )
  endif(EXISTS ${REDIS3M_LIBRARY_DIR}/shared/redis3mConfig.cmake)
else( NOT redis3m_IN_CACHE )
  message(STATUS "    package ${NAME} was allready in loaded. Do not perform dependencies.")
endif( NOT redis3m_IN_CACHE )

list(APPEND REDIS3M_LIBRARY "hiredis")
list(APPEND REDIS3M_LIBRARY "rt")

message(STATUS "    REDIS3M 2: '-I${REDIS3M_INCLUDE_DIR}' '-L${REDIS3M_LIBRARY_DIR}' ")
message(STATUS "             '${REDIS3M_LIBRARIES}' '${REDIS3M_LIBRARY}'")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Redis3m  DEFAULT_REDIS3M REDIS3M_LIBRARY_NAMES REDIS3M_INCLUDE_DIR)

# if the include and the program are found then we have it
IF(REDIS3M_INCLUDE_DIR AND REDIS3M_LIBRARY)
  SET(REDIS3M_FOUND "YES")
ENDIF(REDIS3M_INCLUDE_DIR AND REDIS3M_LIBRARY)

# if( NOT WIN32)
#   list(APPEND REDIS3M_LIBRARY "-lrt")
# endif( NOT WIN32)

MARK_AS_ADVANCED(
  REDIS3M_FOUND
  REDIS3M_LIBRARY
  REDIS3M_INCLUDE_DIR
  REDIS3M_INCLUDE_DIRS
)
if(CMAKE_CROSSCOMPILING)
#  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif(CMAKE_CROSSCOMPILING)
cmake_policy(POP)
