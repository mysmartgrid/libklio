# -*- mode: cmake; -*-
set(klio-add_src klio-add.cpp)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR} 
                    ${Boost_INCLUDE_DIRS})
add_executable(klio-add ${klio-add_src})
#add_executable(foobar sourcetest.cpp)

# Link the executable 
target_link_libraries(klio-add klio ${Boost_IOSTREAMS_LIBRARY})

# add programs to the install target - do not install the hashbench benchmark program.
INSTALL(PROGRAMS 
  ${CMAKE_CURRENT_BINARY_DIR}/klio-add
  DESTINATION bin)

