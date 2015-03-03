# -*- mode: cmake; -*-
# locates the curl library
# This file defines:
# * CURL_FOUND if curl was found
# * CURL_LIBRARY The lib to link to (currently only a static unix lib) 
# * CURL_INCLUDE_DIR

if (NOT CURL_FIND_QUIETLY)
  message(STATUS "FindCURL check")
endif (NOT CURL_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FindPackageHandleStandardArgs)

  if (NOT WIN32)
    include(FindPkgConfig)
    if ( PKG_CONFIG_FOUND OR NOT ${PKG_CONFIG_EXECUTABLE} STREQUAL "PKG_CONFIG_EXECUTABLE-NOTFOUND")

      pkg_check_modules (PC_CURL libcurl>=7.19)

      set(CURL_DEFINITIONS ${PC_CURL_CFLAGS_OTHER})
      message(STATUS "==> '${PC_CURL_CFLAGS_OTHER}'")
    else(PKG_CONFIG_FOUND OR NOT ${PKG_CONFIG_EXECUTABLE} STREQUAL "PKG_CONFIG_EXECUTABLE-NOTFOUND")
      message(STATUS "==> N '${PC_CURL_CFLAGS_OTHER}'")
    endif(PKG_CONFIG_FOUND OR NOT ${PKG_CONFIG_EXECUTABLE} STREQUAL "PKG_CONFIG_EXECUTABLE-NOTFOUND")
  endif (NOT WIN32)

  #
  # set defaults
  set(_curl_HOME "/usr/local")
  set(_curl_INCLUDE_SEARCH_DIRS
    ${CMAKE_INCLUDE_PATH}
    /usr/local/include
    /usr/local/opt/curl/include
    /usr/include
    )

  set(_curl_LIBRARIES_SEARCH_DIRS
    ${CMAKE_LIBRARY_PATH}
    /usr/local/lib
    /usr/local/opt/curl/lib
    /usr/lib
    )

  ##
  if( "${CURL_HOME}" STREQUAL "")
    if("" MATCHES "$ENV{CURL_HOME}")
      message(STATUS "CURL_HOME env is not set, setting it to /usr/local")
      set (CURL_HOME ${_curl_HOME})
    else("" MATCHES "$ENV{CURL_HOME}")
      set (CURL_HOME "$ENV{CURL_HOME}")
    endif("" MATCHES "$ENV{CURL_HOME}")
  else( "${CURL_HOME}" STREQUAL "")
    message(STATUS "CURL_HOME is not empty: \"${CURL_HOME}\"")
  endif( "${CURL_HOME}" STREQUAL "")
  ##

  message(STATUS "Looking for curl in ${CURL_HOME}")

  if( NOT ${CURL_HOME} STREQUAL "" )
    set(_curl_INCLUDE_SEARCH_DIRS ${CURL_HOME}/include ${_curl_INCLUDE_SEARCH_DIRS})
    set(_curl_LIBRARIES_SEARCH_DIRS ${CURL_HOME}/lib ${_curl_LIBRARIES_SEARCH_DIRS})
    set(_curl_HOME ${CURL_HOME})
  endif( NOT ${CURL_HOME} STREQUAL "" )

  if( NOT $ENV{CURL_INCLUDEDIR} STREQUAL "" )
    set(_curl_INCLUDE_SEARCH_DIRS $ENV{CURL_INCLUDEDIR} ${_curl_INCLUDE_SEARCH_DIRS})
  endif( NOT $ENV{CURL_INCLUDEDIR} STREQUAL "" )

  if( NOT $ENV{CURL_LIBRARYDIR} STREQUAL "" )
    set(_curl_LIBRARIES_SEARCH_DIRS $ENV{CURL_LIBRARYDIR} ${_curl_LIBRARIES_SEARCH_DIRS})
  endif( NOT $ENV{CURL_LIBRARYDIR} STREQUAL "" )

  if( CURL_HOME )
    set(_curl_INCLUDE_SEARCH_DIRS ${CURL_HOME}/include ${_curl_INCLUDE_SEARCH_DIRS})
    set(_curl_LIBRARIES_SEARCH_DIRS ${CURL_HOME}/lib ${_curl_LIBRARIES_SEARCH_DIRS})
    set(_curl_HOME ${CURL_HOME})
  endif( CURL_HOME )

  # find the include files
  find_path(CURL_INCLUDE_DIR curl/curl.h
    HINTS
    ${_curl_INCLUDE_SEARCH_DIRS}
    ${PC_CURL_INCLUDEDIR}
    ${PC_CURL_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
    )
  message("==> CURL_INCLUDE_DIR='${CURL_INCLUDE_DIR}'")

  # locate the library
  if(WIN32)
    set(CURL_LIBRARY_NAMES ${CURL_LIBRARY_NAMES} libcurl.lib)
    set(CURL_STATIC_LIBRARY_NAMES ${CURL_LIBRARY_NAMES} libcurl.lib)
  else(WIN32)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      # On MacOS
      set(CURL_LIBRARY_NAMES ${CURL_LIBRARY_NAMES} libcurl.dylib)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      # On Linux
      set(CURL_LIBRARY_NAMES ${CURL_LIBRARY_NAMES} libcurl.so)
    else()
      set(CURL_LIBRARY_NAMES ${CURL_LIBRARY_NAMES} libcurl.a)
    endif()
    set(CURL_STATIC_LIBRARY_NAMES ${CURL_STATIC_LIBRARY_NAMES} libcurl.a)
  endif(WIN32)

  if( PC_CURL_STATIC_LIBRARIES )
    foreach(lib ${PC_CURL_STATIC_LIBRARIES})
      string(TOUPPER ${lib} _NAME_UPPER)

      find_library(CURL_${_NAME_UPPER}_LIBRARY NAMES "lib${lib}.a"
				HINTS
				${_curl_LIBRARIES_SEARCH_DIRS}
				${PC_CURL_LIBDIR}
				${PC_CURL_LIBRARY_DIRS}
				)
      #list(APPEND CURL_LIBRARIES ${_dummy})
    endforeach()
    set(_CURL_LIBRARIES "")
    set(_CURL_STATIC_LIBRARIES "")
    set(_CURL_SHARED_LIBRARIES "")
    foreach(lib ${PC_CURL_STATIC_LIBRARIES})
      if ( ${lib} STREQUAL "gnutls" )
				include(FindGnutls)
				set(_CURL_LIBRARIES ${_CURL_LIBRARIES} ${GNUTLS_LIBRARIES})
				#elseif ( ${lib} STREQUAL "ldap" )
				#include(FindLdap)
				#set(_CURL_LIBRARIES ${_CURL_LIBRARIES} ${LDAP_LIBRARIES})
      else()
				string(TOUPPER ${lib} _NAME_UPPER)
				if( NOT ${CURL_${_NAME_UPPER}_LIBRARY} STREQUAL "CURL_${_NAME_UPPER}_LIBRARY-NOTFOUND" )
					set(_CURL_LIBRARIES ${_CURL_LIBRARIES} ${CURL_${_NAME_UPPER}_LIBRARY})
					set(_CURL_STATIC_LIBRARIES ${_CURL_STATIC_LIBRARIES} ${CURL_${_NAME_UPPER}_LIBRARY})
					#set(_CURL_LIBRARIES ${_CURL_LIBRARIES} -l${lib})
				else( NOT ${CURL_${_NAME_UPPER}_LIBRARY} STREQUAL "CURL_${_NAME_UPPER}_LIBRARY-NOTFOUND" )
					set(_CURL_LIBRARIES ${_CURL_LIBRARIES} -l${lib})
					set(_CURL_SHARED_LIBRARIES ${_CURL_SHARED_LIBRARIES} -l${lib})
				endif( NOT ${CURL_${_NAME_UPPER}_LIBRARY} STREQUAL "CURL_${_NAME_UPPER}_LIBRARY-NOTFOUND" )
      endif()
    endforeach()
    # set(CURL_LIBRARIES ${PC_CURL_STATIC_LDFLAGS} CACHE FILEPATH "")
    # set(CURL_LIBRARIES ${_CURL_LIBRARIES} CACHE FILEPATH "")
    #set(CURL_STATIC_LIBRARIES ${_CURL_STATIC_LIBRARIES} )#CACHE FILEPATH "")
    set(CURL_SHARED_LIBRARIES ${_CURL_SHARED_LIBRARIES} )#CACHE FILEPATH "")
  endif( PC_CURL_STATIC_LIBRARIES )
  message("==> _CURL_LIBRARIES='${_CURL_LIBRARIES}'")
  message("==> CURL_LIBRARIES='${CURL_LIBRARIES}'")

  message("==> CURL_LIBRARY_NAMES='${CURL_LIBRARY_NAMES}'")
  message("==> CURL_STATIC_LIBRARY_NAMES='${CURL_STATIC_LIBRARY_NAMES}'")

  # message("Looking for ${CURL_LIBRARY_NAMES} in location ${_curl_LIBRARIES_SEARCH_DIRS}")
  find_library(CURL_LIBRARIES NAMES ${CURL_LIBRARY_NAMES}
    HINTS
    ${_curl_LIBRARIES_SEARCH_DIRS}
    ${PC_CURL_LIBDIR}
    ${PC_CURL_LIBRARY_DIRS}
    )

  # message("Looking for ${CURL_STATIC_LIBRARY_NAMES} in location ${_curl_LIBRARIES_SEARCH_DIRS}")
  find_library(CURL_STATIC_LIBRARIES NAMES ${CURL_STATIC_LIBRARY_NAMES}
    HINTS
    ${_curl_LIBRARIES_SEARCH_DIRS}
    ${PC_CURL_LIBDIR}
    ${PC_CURL_LIBRARY_DIRS}
    )
	if( ${CURL_LIBRARIES} STREQUAL "CURL_LIBRARIES-NOTFOUND") 
		set(CURL_LIBRARIES ${_CURL_LIBRARIES})
	endif()

  message("==> CURL_LIBRARIES='${CURL_LIBRARIES}'")
  message("==> CURL_STATIC_LIBRARIES='${CURL_STATIC_LIBRARIES}'")
  message("==> CURL_SHARED_LIBRARIES='${CURL_SHARED_LIBRARIES}'")
  message("==> CURL_SHARED_LIBRARY='${CURL_SHARED_LIBRARY}'")

  # find_package_handle_standard_args(CURL DEFAULT_MSG CURL_LIBRARIES CURL_SHARED_LIBRARIES CURL_STATIC_LIBRARIES CURL_INCLUDE_DIR)
  find_package_handle_standard_args(CURL DEFAULT_MSG CURL_LIBRARIES CURL_INCLUDE_DIR)

else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(CURL_FOUND true)
  set(CURL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/curl ${CMAKE_BINARY_DIR}/curl)
  set(CURL_LIBRARY_DIR "")
  set(CURL_LIBRARY curl)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
