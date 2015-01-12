# -*- mode: cmake; -*-
# bernd.loerwald@itwm.fraunhofer.de
# update by kai.krueger@itwm.fraunhofer.de

include(Tools.cmake)
FindOS(OS_NAME OS_VERSION)

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

set(CTEST_PACKAGE_SITE "msgrid@packages.mysmartgrid.de")

site_name(CTEST_SITE)

# test configuration ###############################################################
# parse arguments
string (REPLACE "," ";" SCRIPT_ARGUMENTS "${CTEST_SCRIPT_ARG}")
foreach (ARGUMENT ${SCRIPT_ARGUMENTS})
  if ("${ARGUMENT}" MATCHES "^([^=]+)=(.+)$" )
    set ("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
  endif()
endforeach()

if (NOT DEBUG)
  set(doSubmit 1)
else()
  set(doSubmit 0)
endif()

if (NOT FORCE_CONTINUOUS)
  set(FORCE_CONTINUOUS 0)
endif()

if (NOT TESTING_MODEL)
  message (FATAL_ERROR "No TESTING_MODEL given (available: Nightly, Coverage)")
endif()

if (NOT GIT_BRANCH)
  set (GIT_BRANCH "develop")
endif()

check_Boost(FOUND_BOOST_VERSION FOUND_BOOST_LIB_VERSION)
string(REGEX REPLACE "_" "." SYSTEM_BOOST_VERSION ${FOUND_BOOST_LIB_VERSION})
message("===> Found installed boost version '${FOUND_BOOST_VERSION}'  '${FOUND_BOOST_LIB_VERSION}' '${SYSTEM_BOOST_VERSION}'")

# check for boost and default to version 1.49 if directory does not exists

if (NOT BOOST_VERSION)
  if(${SYSTEM_BOOST_VERSION} LESS "1.49") 
    set(BOOST_VERSION "1.49")
  else()
    set(BOOST_VERSION ${SYSTEM_BOOST_VERSION})
  endif()
else()
  if( NOT CMAKE_TOOLCHAIN_FILE )
    set (EXTERNAL_SOFTWARE "$ENV{HOME}/external_software")
    if (EXISTS ${EXTERNAL_SOFTWARE}/boost/${BOOST_VERSION})
      message("====> Path for boost ${BOOST_VERSION} found.")
    else()
      message("====> Path for boost ${BOOST_VERSION} does not exists, defaulting to 1.49")
      if(${SYSTEM_BOOST_VERSION} LESS "1.49") 
	set(BOOST_VERSION "1.49")
      else()
	set(BOOST_VERSION ${SYSTEM_BOOST_VERSION})
      endif()
    endif()
  endif()
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
  set (BUILD_TMP_DIR "/tmp/$ENV{USER}")
endif ()

if (${COMPILER} STREQUAL "gcc")
  set (CMAKE_C_COMPILER "gcc")
  set (CMAKE_CXX_COMPILER "g++")
elseif( ${COMPILER} STREQUAL "clang" )
  # check if compiler exists
  find_program( CLANG_CC clang )
  find_program( CLANG_CXX clang++ )
  if( ${CLANG_CC} STREQUAL "CLANG_CC-NOTFOUND" OR ${CLANG_CXX} STREQUAL "CLANG_CC-NOTFOUND")
    message(FATAL_ERROR "clang compiler not found. stopping here.")
  else()
    set (CMAKE_C_COMPILER "clang")
    set (CMAKE_CXX_COMPILER "clang++")
  endif()
elseif (${COMPILER} STREQUAL "intel")
  set (CMAKE_C_COMPILER "icc")
  set (CMAKE_CXX_COMPILER "icpc")
else()
  message (FATAL_ERROR "unknown compiler '${COMPILER}'")
endif()
find_program( COMPILER_CC ${CMAKE_C_COMPILER} )
find_program( COMPILER_CXX ${CMAKE_CXX_COMPILER} )
if( ${COMPILER_CC} STREQUAL "COMPILER_CC-NOTFOUND" OR ${COMPILER_CXX} STREQUAL "COMPILER_CXX-NOTFOUND")
  message(FATAL_ERROR "Compiler not found. Stopping here.")
  return(1)
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

if(BOOST_VERSION)
  set(_boost_str "-boost${BOOST_VERSION}")
else()
  set(_boost_str "")
endif()
if(CMAKE_COMPILER_VERSION)
  set(_compiler_str "${COMPILER}${CMAKE_COMPILER_VERSION}")
else()
  set(_compiler_str "${COMPILER}")
endif()

set (CTEST_BUILD_NAME "${CMAKE_SYSTEM_PROCESSOR}-${_compiler_str}${_boost_str}-${GIT_BRANCH}")
set (CTEST_BUILD_NAME_DEVEL  "${CMAKE_SYSTEM_PROCESSOR}-${_compiler_str}${_boost_str}-development")
set (CTEST_BUILD_NAME_MASTER "${CMAKE_SYSTEM_PROCESSOR}-${_compiler_str}${_boost_str}-master")

set (CTEST_BASE_DIRECTORY   "${BUILD_TMP_DIR}/${CTEST_PROJECT_NAME}/${TESTING_MODEL}")
set (CTEST_SOURCE_DIRECTORY "${CTEST_BASE_DIRECTORY}/src-${GIT_BRANCH}-${CMAKE_SYSTEM_PROCESSOR}" )
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
    message("====> Path ${NAME}=${PATH} found.")
  else()
    message("====> Path ${PATH} does not exists.")
  endif()
endmacro()


if( NOT CMAKE_TOOLCHAIN_FILE )
  set (EXTERNAL_SOFTWARE "$ENV{HOME}/external_software")
  set_if_exists (BOOST_ROOT ${EXTERNAL_SOFTWARE}/boost/${BOOST_VERSION})
  set_if_exists (GRAPHVIZ_HOME ${EXTERNAL_SOFTWARE}/graphviz/2.24)
  set_if_exists (ROCKSDB_HOME ${EXTERNAL_SOFTWARE}/rocksdb)
  set_if_exists (SQLITE3_HOME ${EXTERNAL_SOFTWARE}/sqlite3/3.8.5)
else()
  set_if_exists (EXTERNAL_SOFTWARE "${_baseDir}/opt")
  set_if_exists (BOOST_ROOT ${EXTERNAL_SOFTWARE}/boost/${BOOST_VERSION})
  set_if_exists (SQLITE3_HOME ${EXTERNAL_SOFTWARE}/sqlite/3.8.5)
  set_if_exists (REDIS3M_HOME ${EXTERNAL_SOFTWARE}/redis3m)
endif()
set_if_exists (LIBMYSMARTGRID_HOME "${BUILD_TMP_DIR}/libmysmartgrid/${TESTING_MODEL}/install-${CTEST_BUILD_NAME}")
if(NOT LIBMYSMARTGRID_HOME)
  set_if_exists (LIBMYSMARTGRID_HOME "${BUILD_TMP_DIR}/libmysmartgrid/${TESTING_MODEL}/install-${CTEST_BUILD_NAME_DEVEL}")
endif()
if(NOT LIBMYSMARTGRID_HOME)
  set_if_exists (LIBMYSMARTGRID_HOME "${BUILD_TMP_DIR}/libmysmartgrid/${TESTING_MODEL}/install-${CTEST_BUILD_NAME_MASTER}")
endif()

# LIBKLIO_HOME
# LIBHXB_HOME


# cmake options ####################################################################

set (CMAKE_INSTALL_PREFIX "/usr/")
set (CTEST_BUILD_FLAGS "-k -j ${PARALLEL_JOBS}")
set (CTEST_CMAKE_GENERATOR "Unix Makefiles")


# prepare binary directory (clear, write initial cache) ############################

if (NOT "${TESTING_MODEL}" STREQUAL "Continuous")
  file (REMOVE_RECURSE "${CTEST_INSTALL_DIRECTORY}")
endif()
ctest_empty_binary_directory ("${CTEST_BINARY_DIRECTORY}")

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

    LIBMYSMARTGRID_HOME
    BOOST_ROOT
    GRAPHVIZ_HOME
    ROCKSDB_HOME
    SQLITE3_HOME
    REDIS3M_HOME
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
if(FORCE_CONTINUOUS)
  set (first_checkout 1)
endif()

# do testing #######################################################################

set (UPDATE_RETURN_VALUE 0)

ctest_start (${TESTING_MODEL})

ctest_update (RETURN_VALUE UPDATE_RETURN_VALUE)
message("Update returned: ${UPDATE_RETURN_VALUE}")

if ("${TESTING_MODEL}" STREQUAL "Continuous" AND first_checkout EQUAL 0 AND FORCE_CONTINUOUS EQUAL 0)
  if (UPDATE_RETURN_VALUE EQUAL 0)
    return()
  endif ()
endif ()

set(LAST_RETURN_VALUE 0)
ctest_configure (BUILD ${CTEST_BINARY_DIRECTORY} RETURN_VALUE LAST_RETURN_VALUE)

if( STAGING_DIR)
  include(${CTEST_BINARY_DIRECTORY}/CMakeCache.txt)
  set(ENV{STAGING_DIR}     ${OPENWRT_STAGING_DIR})
endif( STAGING_DIR)

if (${LAST_RETURN_VALUE} EQUAL 0)
  ctest_build (NUMBER_ERRORS BUILD_ERRORS
    RETURN_VALUE LAST_RETURN_VALUE)

  message("======> run Install:  <===")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CTEST_INSTALL_DIRECTORY} -P cmake_install.cmake
    WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
    RESULT_VARIABLE INSTALL_ERRORS
    )
  message("======> Install: ${INSTALL_ERRORS} <===")

  if (${BUILD_ERRORS} EQUAL 0 AND ${INSTALL_ERRORS} EQUAL 0)
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

if(doSubmit)
  ctest_submit (RETURN_VALUE ${LAST_RETURN_VALUE})
endif()

# do the packing only if switch is on and
if( (${UPDATE_RETURN_VALUE} GREATER 0 AND PROPERLY_BUILT_AND_INSTALLED) OR ${first_checkout})
  include(${CTEST_BINARY_DIRECTORY}/CPackConfig.cmake)
  if( STAGING_DIR)
    set(ENV{PATH}            ${OPENWRT_STAGING_DIR}/host/bin:$ENV{PATH})
  endif( STAGING_DIR)
  # do the packaging
  execute_process(
    COMMAND cpack -G DEB
    WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
    )
  # install for other packages
  #execute_process(
  #  COMMAND make install
  #  WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
  #  )

  # upload files
  if( ${CTEST_PUSH_PACKAGES})
    message( "OS_NAME .....: ${OS_NAME}")
    message( "OS_VERSION ..: ${OS_VERSION}")
    message( "CMAKE_SYSTEM_PROCESSOR ..: ${CMAKE_SYSTEM_PROCESSOR}")

    if(CPACK_ARCHITECTUR)
      set(OPKG_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_ARCHITECTUR}")
      set(_package_file "${OPKG_FILE_NAME}.ipk")
    else(CPACK_ARCHITECTUR)
      set(_package_file "${CPACK_PACKAGE_FILE_NAME}.deb")
    endif(CPACK_ARCHITECTUR)
    message("==> Upload packages - ${_package_file}")
    set(_export_host ${CTEST_PACKAGE_SITE})
    set(_remote_dir "packages/${OS_NAME}/${OS_VERSION}/${CMAKE_SYSTEM_PROCESSOR}")
    if( NOT ${GIT_BRANCH} STREQUAL "master")
      set(_remote_dir "packages/${OS_NAME}/${OS_VERSION}/${CMAKE_SYSTEM_PROCESSOR}/${GIT_BRANCH}")
    endif()
    message("Execute: ssh ${_export_host} mkdir -p ${_remote_dir}")
    execute_process(
      COMMAND ssh ${_export_host} mkdir -p ${_remote_dir}
      )
    message("Execute scp -p ${_package_file} ${_export_host}:${_remote_dir}/${_package_file}")
    execute_process(
      COMMAND scp -p ${_package_file} ${_export_host}:${_remote_dir}/${_package_file}
      WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}/${subproject}
      )
  endif()
else()
  message("Do not push the packages.")
endif()

#file (REMOVE_RECURSE "${CTEST_INSTALL_DIRECTORY}")
#if (NOT "${TESTING_MODEL}" STREQUAL "Continuous")
#  file (REMOVE_RECURSE "${CTEST_BINARY_DIRECTORY}")
#endif()

return (${LAST_RETURN_VALUE})
