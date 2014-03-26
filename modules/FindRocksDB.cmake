# -*- mode: cmake; -*-
# - Try to find rocksdb include dirs and libraries
# Usage of this module as follows:
# This file defines:
# * ROCKSDB_FOUND if protoc was found
# * ROCKSDB_LIBRARY The lib to link to (currently only a static unix lib, not
# portable)
# * ROCKSDB_INCLUDE The include directories for rocksdb.

message(STATUS "FindRocksDB check")
IF (NOT WIN32)
  include(FindPkgConfig)
  if ( PKG_CONFIG_FOUND )

     # pkg_check_modules (PC_ROCKSDB rocksdb>=2.7)

     set(ROCKSDB_DEFINITIONS ${PC_ROCKSDB_CFLAGS_OTHER})
  endif(PKG_CONFIG_FOUND)
endif (NOT WIN32)

#
# set defaults
SET(_rocksdb_HOME "/opt/rocksdb")
SET(_rocksdb_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  /opt/rocksdb/include
  )

SET(_rocksdb_LIBRARIES_SEARCH_DIRS
  ${CMAKE_LIBRARY_PATH}
  /usr/local/lib
  /usr/lib
  /opt/rocksdb
  )

##
if( "${ROCKSDB_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{ROCKSDB_HOME}")
    message(STATUS "ROCKSDB_HOME env is not set, setting it to /opt/rocksdb")
    set (ROCKSDB_HOME ${_rocksdb_HOME})
  else("" MATCHES "$ENV{ROCKSDB_HOME}")
    set (ROCKSDB_HOME "$ENV{ROCKSDB_HOME}")
  endif("" MATCHES "$ENV{ROCKSDB_HOME}")
else( "${ROCKSDB_HOME}" STREQUAL "")
  message(STATUS "ROCKSDB_HOME is not empty: \"${ROCKSDB_HOME}\"")
endif( "${ROCKSDB_HOME}" STREQUAL "")
##

message(STATUS "Looking for rocksdb in ${ROCKSDB_HOME}")

IF( NOT ${ROCKSDB_HOME} STREQUAL "" )
    SET(_rocksdb_INCLUDE_SEARCH_DIRS ${ROCKSDB_HOME}/include ${_rocksdb_INCLUDE_SEARCH_DIRS})
    SET(_rocksdb_LIBRARIES_SEARCH_DIRS ${ROCKSDB_HOME}/lib ${_rocksdb_LIBRARIES_SEARCH_DIRS})
    SET(_rocksdb_HOME ${ROCKSDB_HOME})
ENDIF( NOT ${ROCKSDB_HOME} STREQUAL "" )

IF( NOT $ENV{ROCKSDB_INCLUDEDIR} STREQUAL "" )
  SET(_rocksdb_INCLUDE_SEARCH_DIRS $ENV{ROCKSDB_INCLUDEDIR} ${_rocksdb_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{ROCKSDB_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{ROCKSDB_LIBRARYDIR} STREQUAL "" )
  SET(_rocksdb_LIBRARIES_SEARCH_DIRS $ENV{ROCKSDB_LIBRARYDIR} ${_rocksdb_LIBRARIES_SEARCH_DIRS})
ENDIF( NOT $ENV{ROCKSDB_LIBRARYDIR} STREQUAL "" )

IF( ROCKSDB_HOME )
  SET(_rocksdb_INCLUDE_SEARCH_DIRS ${ROCKSDB_HOME}/include ${_rocksdb_INCLUDE_SEARCH_DIRS})
  SET(_rocksdb_LIBRARIES_SEARCH_DIRS ${ROCKSDB_HOME}/lib ${_rocksdb_LIBRARIES_SEARCH_DIRS})
  SET(_rocksdb_HOME ${ROCKSDB_HOME})
ENDIF( ROCKSDB_HOME )

message("RocksDB search: '${_rocksdb_INCLUDE_SEARCH_DIRS}' ${CMAKE_INCLUDE_PATH}")
# find the include files
FIND_PATH(ROCKSDB_INCLUDE_DIR rocksdb/db.h
   HINTS
     ${_rocksdb_INCLUDE_SEARCH_DIRS}
     ${PC_ROCKSDB_INCLUDEDIR}
     ${PC_ROCKSDB_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
)

# locate the library
if(WIN32)
  set(ROCKSDB_LIBRARY_NAMES ${ROCKSDB_LIBRARY_NAMES} librocksdb.lib)
  set(ROCKSDB_STATIC_LIBRARY_NAMES ${ROCKSDB_LIBRARY_NAMES} librocksdb.lib)
else(WIN32)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # On MacOS
    set(ROCKSDB_LIBRARY_NAMES ${ROCKSDB_LIBRARY_NAMES} librocksdb.dylib)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # On Linux
    set(ROCKSDB_LIBRARY_NAMES ${ROCKSDB_LIBRARY_NAMES} librocksdb.so)
  else()
    set(ROCKSDB_LIBRARY_NAMES ${ROCKSDB_LIBRARY_NAMES} librocksdb.a)
  endif()
  set(ROCKSDB_STATIC_LIBRARY_NAMES ${ROCKSDB_STATIC_LIBRARY_NAMES} librocksdb.a)
endif(WIN32)

FIND_LIBRARY(ROCKSDB_LIBRARIES NAMES ${ROCKSDB_LIBRARY_NAMES}
  HINTS
    ${_rocksdb_LIBRARIES_SEARCH_DIRS}
    ${PC_ROCKSDB_LIBDIR}
    ${PC_ROCKSDB_LIBRARY_DIRS}
)
find_library(ROCKSDB_STATIC_LIBRARIES NAMES ${ROCKSDB_STATIC_LIBRARY_NAMES}
  HINTS
  ${_rocksdb_LIBRARIES_SEARCH_DIRS}
  ${PC_ROCKSDB_LIBDIR}
  ${PC_ROCKSDB_LIBRARY_DIRS}
  )
if( NOT WIN32)
#  list(APPEND ROCKSDB_LIBRARY "-lrt")
  list(APPEND ROCKSDB_STATIC_LIBRARIES "-lpthread")
endif( NOT WIN32)

  message("==> ROCKSDB_LIBRARIES='${ROCKSDB_LIBRARIES}'")
  message("==> ROCKSDB_STATIC_LIBRARIES='${ROCKSDB_STATIC_LIBRARIES}'")
  message("==> ROCKSDB_SHARED_LIBRARIES='${ROCKSDB_SHARED_LIBRARIES}'")
  message("==> ROCKSDB_SHARED_LIBRARY='${ROCKSDB_SHARED_LIBRARY}'")
  message("==> ROCKSDB_LIBRARIES='${ROCKSDB_LIBRARIES}'")
  message("==> ROCKSDB_LIBRARY='${ROCKSDB_LIBRARY}'")

  find_package_handle_standard_args(ROCKSDB DEFAULT_MSG ROCKSDB_LIBRARIES ROCKSDB_SHARED_LIBRARIES ROCKSDB_STATIC_LIBRARIES ROCKSDB_INCLUDE_DIR)

# if the include and the program are found then we have it
IF(ROCKSDB_INCLUDE_DIR AND ROCKSDB_LIBRARY)
  SET(ROCKSDB_FOUND "YES")
ENDIF(ROCKSDB_INCLUDE_DIR AND ROCKSDB_LIBRARY)
