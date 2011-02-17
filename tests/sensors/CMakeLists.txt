
include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
file(GLOB all_sensorstest_src *.cpp *.hpp)
set(sensorstest_src ${all_sensorstest_src})
add_executable(sensorstest ${sensorstest_src})

# Link the executable 
target_link_libraries(sensorstest klio klio_sqlite3 ${Boost_LIBRARIES} )

# add programs to the install target - do not install the hashbench benchmark program.
#INSTALL(PROGRAMS 
#  ${CMAKE_CURRENT_BINARY_DIR}/klio-add
#  DESTINATION bin)
#
