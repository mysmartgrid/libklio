# -*- mode: cmake; -*-

set(ENV{https_proxy} "http://squid.itwm.fhg.de:3128/")
include(Tools.cmake)
include(CTestConfigLibKlio.cmake)

set(URL "https://github.com/mysmartgrid/libklio.git")

set(KDE_CTEST_DASHBOARD_DIR "/tmp/msgrid")
set(CTEST_BASE_DIRECTORY "${KDE_CTEST_DASHBOARD_DIR}/${_projectNameDir}")
set(CTEST_SOURCE_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_srcDir}-${_git_branch}" )
set(CTEST_BINARY_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_buildDir}-${CTEST_BUILD_NAME}")
set(CTEST_INSTALL_DIRECTORY "${CTEST_BASE_DIRECTORY}/install-${CTEST_BUILD_NAME}")
set(KDE_CTEST_VCS "git")
set(KDE_CTEST_VCS_REPOSITORY ${URL})

set(CMAKE_INSTALL_PREFIX "/usr")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(CTEST_BUILD_CONFIGURATION "Profiling")

string(REGEX REPLACE "[ /:\\.]" "_" _tmpDir ${KDE_CTEST_VCS_REPOSITORY})
set(_tmpDir "${KDE_CTEST_DASHBOARD_DIR}/tmp/${_tmpDir}")
configure_file("CTestConfigLibKlio.cmake" ${_tmpDir}/CTestConfig.cmake COPYONLY)

# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake")
kde_ctest_setup()

FindOS(OS_NAME OS_VERSION)

set(ctest_config ${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake)
#######################################################################
ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})


find_program(CTEST_GIT_COMMAND NAMES git)
set(CTEST_UPDATE_TYPE git)

set(CTEST_UPDATE_COMMAND  ${CTEST_GIT_COMMAND})
if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.git/HEAD")
  set(CTEST_CHECKOUT_COMMAND "${CTEST_GIT_COMMAND} clone ${URL} ${CTEST_SOURCE_DIRECTORY}")
endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.git/HEAD")

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
set(_ctest_type "Nightly")
# set(_ctest_type "Continuous")
ctest_start(${_ctest_type})
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
ctest_submit(PARTS Update)

execute_process(
  COMMAND ${CTEST_GIT_COMMAND} checkout  ${_git_branch}
  WORKING_DIRECTORY ${CTEST_SOURCE_DIRECTORY}
  )

if(CMAKE_TOOLCHAIN_FILE)
  kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
    CMAKE_TOOLCHAIN_FILE
    CMAKE_INSTALL_PREFIX
    )
else(CMAKE_TOOLCHAIN_FILE)
  kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
    BOOST_ROOT
    CMAKE_INSTALL_PREFIX
)
endif(CMAKE_TOOLCHAIN_FILE)


ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}"  RETURN_VALUE resultConfigure)
message("====> Configure: ${resultConfigure}")

if( STAGING_DIR)
  include(${CTEST_BINARY_DIRECTORY}/CMakeCache.txt)
  set(ENV{STAGING_DIR}     ${OPENWRT_STAGING_DIR})
endif( STAGING_DIR)

ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE build_res)
message("====> BUILD: ${build_res}")

ctest_submit(RETURN_VALUE res)

# package files
include(${CTEST_BINARY_DIRECTORY}/CPackConfig.cmake)
if( STAGING_DIR)
  set(ENV{PATH}            ${OPENWRT_STAGING_DIR}/host/bin:$ENV{PATH})
endif( STAGING_DIR)

if( NOT ${build_res})
  execute_process(
    COMMAND cpack -G DEB
    WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}
    )
endif( NOT ${build_res})

# upload files
if( NOT ${build_res} AND ${CTEST_PUSH_PACKAGES})
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
  execute_process(
    COMMAND ssh ${_export_host} mkdir -p ${_remote_dir}
    )
  execute_process(
    COMMAND scp -p ${_package_file} ${_export_host}:${_remote_dir}/${_package_file}
    WORKING_DIRECTORY ${CTEST_BINARY_DIRECTORY}/${subproject}
    )
endif( NOT ${build_res} AND ${CTEST_PUSH_PACKAGES})

message("DONE")
