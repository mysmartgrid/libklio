
include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
                  #file(GLOB all_timetest_src *.cpp *.hpp)
set(timetest_src 
  test_time.cpp
  main.cpp
)
          
add_executable(timetest ${timetest_src})

# Link the executable 
target_link_libraries(timetest klio klio_sqlite3 ${Boost_LIBRARIES} )

# add programs to the install target - do not install the hashbench benchmark program.
#INSTALL(PROGRAMS 
#  ${CMAKE_CURRENT_BINARY_DIR}/klio-add
#  DESTINATION bin)
#
