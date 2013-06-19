# -*- mode: cmake; -*-
# bernd.loerwald@itwm.fraunhofer.de
# update by kai.krueger@itwm.fraunhofer.de

# cdash / ctest information ########################################################

set(CTEST_PROJECT_NAME "libklio")
set(CTEST_NIGHTLY_START_TIME "04:00:00 CET")

set(CTEST_DROP_METHOD     "http")
set(CTEST_DROP_SITE       "cdash.hexabus.de")
set(CTEST_DROP_LOCATION   "/submit.php?project=${CTEST_PROJECT_NAME}")
set(CTEST_DROP_SITE_CDASH TRUE)
set(CTEST_USE_LAUNCHERS   0)
set(CTEST_TEST_TIMEOUT   180)
set(KDE_CTEST_DASHBOARD_DIR "/tmp/$ENV{USER}")

set(CTEST_PACKAGE_SITE "packages.mysmartgrid.de")

site_name(CTEST_SITE)

# test configuration ###############################################################
# parse arguments
string (REPLACE "," ";" SCRIPT_ARGUMENTS "${CTEST_SCRIPT_ARG}")
foreach (ARGUMENT ${SCRIPT_ARGUMENTS})
  if ("${ARGUMENT}" MATCHES "^([^=]+)=(.+)$" )
    set ("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
  endif()
endforeach()

if (NOT TESTING_MODEL)
  message (FATAL_ERROR "No TESTING_MODEL given (available: Nightly, Coverage)")
endif()

if (NOT GIT_BRANCH)
  set (GIT_BRANCH "develop")
endif()

if (NOT BOOST_VERSION)
  set (BOOST_VERSION "1.49")
endif()

if (NOT COMPILER)
  set (COMPILER "gcc")
endif()

if (NOT PARALLEL_JOBS)
  set (PARALLEL_JOBS 1)
endif()

if (NOT REPOSITORY_URL)
  set (REPOSITORY_URL "git_gpispace:gpispace.git")
endif ()

if (NOT BUILD_TMP_DIR)
  set (BUILD_TMP_DIR "/tmp")
endif ()

if (${COMPILER} STREQUAL "gcc")
  set (CMAKE_C_COMPILER "gcc")
  set (CMAKE_CXX_COMPILER "g++")
elseif( ${COMPILER} STREQUAL "clang" )
  set (CMAKE_C_COMPILER "clang")
  set (CMAKE_CXX_COMPILER "clang++")
elseif (${COMPILER} STREQUAL "intel")
  set (CMAKE_C_COMPILER "icc")
  set (CMAKE_CXX_COMPILER "icpc")
else()
  message (FATAL_ERROR "unknown compiler '${COMPILER}'")
endif()

set (ENV{CC} ${CMAKE_C_COMPILER})
set (ENV{CXX} ${CMAKE_CXX_COMPILER})

if(CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
  if( openwrt_arch ) 
    set(CMAKE_SYSTEM_PROCESSOR ${openwrt_arch})
  endif()
endif()

# variables / configuration based on test configuration ############################
set(_projectNameDir "${CTEST_PROJECT_NAME}")

set (CTEST_BUILD_NAME "${CMAKE_SYSTEM_PROCESSOR}-${COMPILER}-boost${BOOST_VERSION}-${GIT_BRANCH}")

set (CTEST_BASE_DIRECTORY   "${BUILD_TMP_DIR}/krueger/${CTEST_PROJECT_NAME}/${TESTING_MODEL}")
set (CTEST_SOURCE_DIRECTORY "${CTEST_BASE_DIRECTORY}/src-${GIT_BRANCH}" )
set (CTEST_BINARY_DIRECTORY "${CTEST_BASE_DIRECTORY}/build-${CTEST_BUILD_NAME}")
set (CTEST_INSTALL_DIRECTORY "${CTEST_BASE_DIRECTORY}/install-${CTEST_BUILD_NAME}")

if (${TESTING_MODEL} STREQUAL "Nightly")
  set (CMAKE_BUILD_TYPE "Release")
elseif (${TESTING_MODEL} STREQUAL "Continuous")
  set (CMAKE_BUILD_TYPE "Release")
elseif (${TESTING_MODEL} STREQUAL "Coverage")
  set (CMAKE_BUILD_TYPE "Profile")
  set (ENABLE_CODECOVERAGE 1)

  find_program (CTEST_COVERAGE_COMMAND NAMES gcov)
  find_program (CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
else()
  message (FATAL_ERROR "Unknown TESTING_MODEL ${TESTING_MODEL} (available: Nightly, Coverage, Continuous)")
endif()


set(URL "https://github.com/mysmartgrid/libklio.git")

# external software ################################################################

if (${COMPILER} STREQUAL "intel")
  set (ADDITIONAL_SUFFIX "intel")
else()
  set (ADDITIONAL_SUFFIX "gcc")
endif()

macro (set_if_exists NAME PATH)
  if (EXISTS ${PATH})
    set (${NAME} ${PATH})
    set ($ENV{Name} ${PATH})
  endif()
endmacro()


if( NOT CMAKE_TOOLCHAIN_FILE )
  set (EXTERNAL_SOFTWARE "$ENV{HOME}/external_software")
  set_if_exists (BOOST_ROOT ${EXTERNAL_SOFTWARE}/boost/${BOOST_VERSION}/${ADDITIONAL_SUFFIX})
  set_if_exists (GRAPHVIZ_HOME ${EXTERNAL_SOFTWARE}/graphviz/2.24)
else()
  set_if_exists (EXTERNAL_SOFTWARE "/home/projects/msgrid/x-tools/arm-unknown-linux-gnueabihf/opt")
  set_if_exists (BOOST_ROOT ${EXTERNAL_SOFTWARE}/boost/${BOOST_VERSION})
endif()
# LIBKLIO_HOME
# LIBHXB_HOME


# cmake options ####################################################################

set (CMAKE_INSTALL_PREFIX ${CTEST_INSTALL_DIRECTORY})
set (CTEST_BUILD_FLAGS "-k -j ${PARALLEL_JOBS}")
set (CTEST_CMAKE_GENERATOR "Unix Makefiles")


# prepare binary directory (clear, write initial cache) ############################

if (NOT "${TESTING_MODEL}" STREQUAL "Continuous")
  file (REMOVE_RECURSE "${CTEST_INSTALL_DIRECTORY}")
  ctest_empty_binary_directory ("${CTEST_BINARY_DIRECTORY}")
endif()

if (NOT EXISTS "${CTEST_BINARY_DIRECTORY}")
  file (MAKE_DIRECTORY "${CTEST_BINARY_DIRECTORY}")
endif()

file (WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt"
  "# Automatically generated in ctest script (write_initial_cache())\n\n")

foreach (VARIABLE_NAME
    CMAKE_BUILD_TYPE
    CMAKE_INSTALL_PREFIX
    CMAKE_TOOLCHAIN_FILE
    CMAKE_CXX_COMPILER
    CMAKE_C_COMPILER
    CMAKE_SYSTEM_PROCESSOR
#    OS_NAME
#    OS_VERSION

    CTEST_TIMEOUT
    CTEST_USE_LAUNCHERS

    ENABLE_CODECOVERAGE

    BOOST_ROOT
    GRAPHVIZ_HOME

    )
  if (DEFINED ${VARIABLE_NAME})
    file (APPEND "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "${VARIABLE_NAME}:STRING=${${VARIABLE_NAME}}\n")
  endif()
endforeach()

# prepare source directory (do initial checkout, switch branch) ####################
find_program (CTEST_GIT_COMMAND NAMES git)

if (NOT EXISTS ${CTEST_SOURCE_DIRECTORY}/.git)
  set (CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone -b ${GIT_BRANCH} ${URL} ${CTEST_SOURCE_DIRECTORY}")
  set (first_checkout 1)
else()
  set (first_checkout 0)
endif()

# do testing #######################################################################

set (LAST_RETURN_VALUE 0)

ctest_start (${TESTING_MODEL})

ctest_update (RETURN_VALUE LAST_RETURN_VALUE)
message("Update returned: ${LAST_RETURN_VALUE}")

if ("${TESTING_MODEL}" STREQUAL "Continuous" AND first_checkout EQUAL 0)
  if (LAST_RETURN_VALUE EQUAL 0)
    return()
  endif ()
endif ()

ctest_configure (BUILD ${CTEST_BINARY_DIRECTORY} RETURN_VALUE LAST_RETURN_VALUE)

if( STAGING_DIR)
  include(${CTEST_BINARY_DIRECTORY}/CMakeCache.txt)
  set(ENV{STAGING_DIR}     ${OPENWRT_STAGING_DIR})
endif( STAGING_DIR)

if (${LAST_RETURN_VALUE} EQUAL 0)
  ctest_build (TARGET install NUMBER_ERRORS BUILD_ERRORS
    RETURN_VALUE LAST_RETURN_VALUE)

  if (${BUILD_ERRORS} EQUAL 0)
    set (PROPERLY_BUILT_AND_INSTALLED TRUE)
  endif()

  if (${TESTING_MODEL} STREQUAL "Coverage")
    ctest_coverage (RETURN_VALUE LAST_RETURN_VALUE)
    ctest_memcheck (RETURN_VALUE LAST_RETURN_VALUE)
  endif()

  if( NOT CMAKE_TOOLCHAIN_FILE )
    if (PROPERLY_BUILT_AND_INSTALLED)
      ctest_test (SCHEDULE_RANDOM true RETURN_VALUE LAST_RETURN_VALUE PARALLEL_LEVEL ${PARALLEL_JOBS})
    else()
      ctest_test (SCHEDULE_RANDOM true EXCLUDE_LABEL "requires_installation"
	RETURN_VALUE LAST_RETURN_VALUE)
    endif()
  endif( NOT CMAKE_TOOLCHAIN_FILE )
endif()

# todo: create package, upload to distribution server

ctest_submit (RETURN_VALUE ${LAST_RETURN_VALUE})

#file (REMOVE_RECURSE "${CTEST_INSTALL_DIRECTORY}")
#if (NOT "${TESTING_MODEL}" STREQUAL "Continuous")
#  file (REMOVE_RECURSE "${CTEST_BINARY_DIRECTORY}")
#endif()

return (${LAST_RETURN_VALUE})
