# -*- mode: cmake; -*-

configure_file(testconfig.h.in ${CMAKE_BINARY_DIR}/testconfig.h)
include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${LIBMYSMARTGRID_INCLUDE_DIRS}
                    ${CURL_INCLUDE_DIR}
)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if(ROCKSDB_FOUND)
  include_directories(
                    ${ROCKSDB_INCLUDE_DIR}
  )
  link_directories(${ROCKSDB_LIBRARY_DIRS})
endif(ROCKSDB_FOUND)

if(REDIS3M_FOUND)
  include_directories(
                    ${REDIS3M_INCLUDE_DIR}
  )
  link_directories(${REDIS3M_LIBRARY_DIRS})
endif(REDIS3M_FOUND)

link_directories(${SQLITE3_LIBRARY_DIRS})
add_subdirectory(store)
add_subdirectory(sensors)
add_subdirectory(time)
add_subdirectory(algorithms)

