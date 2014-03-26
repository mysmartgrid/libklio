# -*- mode: cmake; -*-
set(klio-sensor_src klio-sensor.cpp)
set(klio-store_src klio-store.cpp)
set(klio-export_src klio-export.cpp)

link_directories(${SQLITE3_LIBRARY_DIRS})

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${ROCKSDB_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIR}
)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(klio-sensor ${klio-sensor_src})
add_executable(klio-store ${klio-store_src})
add_executable(klio-export ${klio-export_src})


if(ROCKSDB_FOUND)
  set_source_files_properties(${klio-sensor_src} COMPILE_FLAGS "-std=gnu++11")
  set_source_files_properties(${klio-store_src} COMPILE_FLAGS "-std=gnu++11")
  set_source_files_properties(${klio-export_src} COMPILE_FLAGS "-std=gnu++11")
#  target_link_libraries(klio-sensor klio ${ROCKSDB_LIBRARIES} snappy)
#  target_link_libraries(klio-store klio ${ROCKSDB_LIBRARIES} snappy)
#  target_link_libraries(klio-export klio ${ROCKSDB_LIBRARIES} snappy)
endif(ROCKSDB_FOUND)

#sensor_executable(foobar sourcetest.cpp)
# Link the executable
target_link_libraries(klio-sensor klio ${Boost_LIBRARIES})
target_link_libraries(klio-store klio ${Boost_LIBRARIES})
target_link_libraries(klio-export klio ${Boost_LIBRARIES})

# sensor programs to the install target
INSTALL(PROGRAMS 
  ${CMAKE_CURRENT_BINARY_DIR}/klio-sensor
  ${CMAKE_CURRENT_BINARY_DIR}/klio-store
  ${CMAKE_CURRENT_BINARY_DIR}/klio-export
  DESTINATION bin)
