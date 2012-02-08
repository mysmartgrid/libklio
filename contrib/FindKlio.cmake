# -*- mode: cmake; -*-
# locates the klio library
# This file defines:
# * KLIO_FOUND if klio was found
# * KLIO_LIBRARY The lib to link to (currently only a static unix lib) 
# * KLIO_INCLUDE_DIR

if (NOT KLIO_FIND_QUIETLY)
  message(STATUS "FindKlio check")
endif (NOT KLIO_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  #  include(FindPackageHelper)
  #  check_package(FhgLog fhglog/fhglog.hpp fhglog 1.0)

  find_path (KLIO_INCLUDE_DIR
    NAMES "libklio/common.hpp"
    HINTS ${KLIO_HOME} ENV KLIO_HOME
    PATH_SUFFIXES include
  )

#  find_library (KLIO_LIBRARY
#    NAMES libklio.so
#    HINTS ${KLIO_HOME} ENV KLIO_HOME
#    PATH_SUFFIXES lib
#  )

#  find_library (KLIO_SQLITE3_LIBRARY
#    NAMES libklio_sqlite3.a
#    HINTS ${KLIO_HOME} ENV KLIO_HOME
#    PATH_SUFFIXES lib
#  )

  IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # On MacOS
    find_library (KLIO_LIBRARY
      NAMES libklio.dylib
      HINTS ${KLIO_HOME} ENV KLIO_HOME
      PATH_SUFFIXES lib
      )
  ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # On Linux
    find_library (KLIO_LIBRARY
      NAMES libklio.a
      HINTS ${KLIO_HOME} ENV KLIO_HOME
      PATH_SUFFIXES lib
      )
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


  if (KLIO_INCLUDE_DIR AND KLIO_LIBRARY)
  set (KLIO_FOUND TRUE)
  if (NOT KLIO_FIND_QUIETLY)
    message (STATUS "Found klio headers in ${KLIO_INCLUDE_DIR} and library ${KLIO_LIBRARY} ${KLIO_SQLITE3_LIBRARY} ${KLIO_LIBRARY_SHARED}")
  endif (NOT KLIO_FIND_QUIETLY)
  else (KLIO_INCLUDE_DIR AND KLIO_LIBRARY)
    if (KLIO_FIND_REQUIRED)
      message (FATAL_ERROR "klio could not be found! Cannot compile without. Try setting $KLIO_HOME")
    endif (KLIO_FIND_REQUIRED)
  endif (KLIO_INCLUDE_DIR AND KLIO_LIBRARY)

else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(KLIO_FOUND true)
  set(KLIO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/klio ${CMAKE_BINARY_DIR}/klio)
  #set(KLIO_LIBRARY_DIR "")
  set(KLIO_LIBRARY klio)
  set(KLIO_SQLITE3_LIBRARY klio_sqlite3)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
