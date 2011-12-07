# -*- mode: cmake; -*-
set(klio-sensor_src klio-sensor.cpp)
set(klio-store_src klio-store.cpp)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR} 
                    ${Boost_INCLUDE_DIRS})
add_executable(klio-sensor ${klio-sensor_src})
add_executable(klio-store ${klio-store_src})
#sensor_executable(foobar sourcetest.cpp)

# Link the executable 
target_link_libraries(klio-sensor klio klio_sqlite3 ${Boost_LIBRARIES})
target_link_libraries(klio-store klio klio_sqlite3 ${Boost_LIBRARIES})

# sensor programs to the install target - do not install the hashbench benchmark program.
INSTALL(PROGRAMS 
  ${CMAKE_CURRENT_BINARY_DIR}/klio-sensor
  ${CMAKE_CURRENT_BINARY_DIR}/klio-store
  DESTINATION bin)

