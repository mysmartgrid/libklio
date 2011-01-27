
include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
file(GLOB all_storetest_src *.cpp *.hpp)
set(storetest_src ${all_storetest_src})
add_executable(storetest ${storetest_src})

# Link the executable 
target_link_libraries(storetest klio ${Boost_LIBRARIES} boost_test_exec_monitor )

# add programs to the install target - do not install the hashbench benchmark program.
#INSTALL(PROGRAMS 
#  ${CMAKE_CURRENT_BINARY_DIR}/klio-add
#  DESTINATION bin)
#
