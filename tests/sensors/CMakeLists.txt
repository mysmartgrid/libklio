
include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
                  #file(GLOB all_sensorstest_src *.cpp *.hpp)
set(sensorstest_src 
  test_data.cpp
  test_sensors.cpp
  test_readings.cpp
  main.cpp
)
          
add_executable(sensorstest ${sensorstest_src})

ADD_TEST(SensorTest ${CMAKE_CURRENT_BINARY_DIR}/sensorstest TestSensors)

# Link the executable 
target_link_libraries(sensorstest klio ${Boost_LIBRARIES} )


