# -*- mode: cmake; -*-

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#file(GLOB all_sensorstest_src *.cpp *.hpp)
set(sensorstest_src 
  test_data.cpp
  test_sensors.cpp
  test_readings.cpp
  main.cpp
)
          
add_executable(sensorstest ${sensorstest_src})

if(ROCKSDB_FOUND)
  set_source_files_properties(${sensorstest_src} COMPILE_FLAGS "-std=gnu++11")
endif(ROCKSDB_FOUND)

if(REDIS3M_FOUND)
target_link_libraries(sensorstest ${REDIS3M_LIBRARY})
endif(REDIS3M_FOUND)

if(POSTGRESQL_FOUND)
target_link_libraries(sensorstest ${POSTGRESQL_LIBRARY})
endif(POSTGRESQL_FOUND)

ADD_TEST(SensorTest ${CMAKE_CURRENT_BINARY_DIR}/sensorstest TestSensors)

# Link the executable 
target_link_libraries(sensorstest klio ${Boost_LIBRARIES} pthread)


