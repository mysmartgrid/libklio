# -*- mode: cmake; -*-

set(ENV{https_proxy} "http://squid.itwm.fhg.de:3128/")
include(Tools.cmake)
include(CTestConfigLibKlio.cmake)
# set(_ctest_type "Nightly")
# set(_ctest_type "Continuous")
set(_ctest_type "Coverage")

set(URL "https://github.com/mysmartgrid/libklio.git")

set(KDE_CTEST_DASHBOARD_DIR "/tmp/msgrid")
set(CTEST_BASE_DIRECTORY "${KDE_CTEST_DASHBOARD_DIR}/${_projectNameDir}/${_ctest_type}")
set(CTEST_SOURCE_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_srcDir}-${_git_branch}" )
set(CTEST_BINARY_DIRECTORY "${CTEST_BASE_DIRECTORY}/${_buildDir}-${CTEST_BUILD_NAME}")
set(CTEST_INSTALL_DIRECTORY "${CTEST_BASE_DIRECTORY}/install-${CTEST_BUILD_NAME}")
set(KDE_CTEST_VCS "git")
set(KDE_CTEST_VCS_REPOSITORY ${URL})

set(CMAKE_INSTALL_PREFIX "/usr")
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
#set(CTEST_BUILD_CONFIGURATION "Profiling")

configure_ctest_config(${KDE_CTEST_VCS_REPOSITORY} "CTestConfigLibKlio.cmake")

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
ctest_start(${_ctest_type})
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}")
ctest_submit(PARTS Update)

execute_process(
  COMMAND ${CTEST_GIT_COMMAND} checkout  ${_git_branch}
  WORKING_DIRECTORY ${CTEST_SOURCE_DIRECTORY}
  )


set(ENABLE_CODECOVERAGE 1)
set(CMAKE_BUILD_TYPE Profile)

if(CMAKE_TOOLCHAIN_FILE)
  kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
    CMAKE_TOOLCHAIN_FILE
    CMAKE_INSTALL_PREFIX
    ENABLE_CODECOVERAGE
    CMAKE_BUILD_TYPE
    )
else(CMAKE_TOOLCHAIN_FILE)
  set(BOOST_ROOT /homes/krueger/external_software/ubuntu_100403/${CMAKE_SYSTEM_PROCESSOR}/boost/1.46)
  kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}"
    BOOST_ROOT
    CMAKE_INSTALL_PREFIX
    ENABLE_CODECOVERAGE
    CMAKE_BUILD_TYPE
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

ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE test_res)
message("====> TESTS: ${test_res}")

ctest_submit(RETURN_VALUE res)

message("DONE")
