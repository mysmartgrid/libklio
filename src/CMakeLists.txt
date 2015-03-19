# -*- mode: cmake; -*-
set(klio-sensor_src klio-sensor.cpp)
set(klio-store_src klio-store.cpp)
set(klio-export_src klio-export.cpp)
set(klio-import_src klio-import.cpp)

link_directories(${SQLITE3_LIBRARY_DIRS})
link_libraries(klio)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${LIBMYSMARTGRID_INCLUDE_DIRS}
)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable(klio-sensor ${klio-sensor_src})
add_executable(klio-store ${klio-store_src})
add_executable(klio-export ${klio-export_src})
add_executable(klio-import ${klio-import_src})


if(ROCKSDB_FOUND)
  set_source_files_properties(${klio-sensor_src} COMPILE_FLAGS "-std=gnu++11")
  set_source_files_properties(${klio-store_src} COMPILE_FLAGS "-std=gnu++11")
  set_source_files_properties(${klio-export_src} COMPILE_FLAGS "-std=gnu++11")
  set_source_files_properties(${klio-import_src} COMPILE_FLAGS "-std=gnu++11")
  include_directories(
                    ${ROCKSDB_INCLUDE_DIR}
  )
#  target_link_libraries(klio-sensor klio ${ROCKSDB_LIBRARIES} snappy)
#  target_link_libraries(klio-store klio ${ROCKSDB_LIBRARIES} snappy)
#  target_link_libraries(klio-export klio ${ROCKSDB_LIBRARIES} snappy)
#  target_link_libraries(klio-import klio ${ROCKSDB_LIBRARIES} snappy)
endif(ROCKSDB_FOUND)

if(REDIS3M_FOUND)
target_link_libraries(klio-sensor klio ${REDIS3M_LIBRARY})
target_link_libraries(klio-store klio ${REDIS3M_LIBRARY})
target_link_libraries(klio-export klio ${REDIS3M_LIBRARY})
target_link_libraries(klio-import klio ${REDIS3M_LIBRARY})
include_directories(
                    ${REDIS3M_INCLUDE_DIR}
)
endif(REDIS3M_FOUND)

if(POSTGRESQL_FOUND)
target_link_libraries(klio-sensor klio ${POSTGRESQL_LIBRARY})
target_link_libraries(klio-store klio ${POSTGRESQL_LIBRARY})
target_link_libraries(klio-export klio ${POSTGRESQL_LIBRARY})
target_link_libraries(klio-import klio ${POSTGRESQL_LIBRARY})
include_directories(
                    ${POSTGRESQL_INCLUDE_DIR}
)
endif(POSTGRESQL_FOUND)

#sensor_executable(foobar sourcetest.cpp)
# Link the executable
target_link_libraries(klio-sensor klio ${Boost_LIBRARIES} pthread)
target_link_libraries(klio-store klio ${Boost_LIBRARIES} pthread)
target_link_libraries(klio-export klio ${Boost_LIBRARIES} pthread)
target_link_libraries(klio-import klio ${Boost_LIBRARIES} pthread)

# sensor programs to the install target
INSTALL(PROGRAMS 
  ${CMAKE_CURRENT_BINARY_DIR}/klio-sensor
  ${CMAKE_CURRENT_BINARY_DIR}/klio-store
  ${CMAKE_CURRENT_BINARY_DIR}/klio-export
  ${CMAKE_CURRENT_BINARY_DIR}/klio-import
  DESTINATION bin)
