# -*- mode: cmake; -*-

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#file(GLOB all_timetest_src *.cpp *.hpp)
set(timetest_src 
  test_time.cpp
  main.cpp
)
          
add_executable(timetest ${timetest_src})

ADD_TEST(TimeTest ${CMAKE_CURRENT_BINARY_DIR}/timetest TestTime)

# Link the executable 
target_link_libraries(timetest klio ${Boost_LIBRARIES} )
if(REDIS3M_FOUND)
target_link_libraries(timetest ${REDIS3M_LIBRARY})
endif(REDIS3M_FOUND)
