# -*- mode: cmake; -*-

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB all_storetest_src *.cpp *.hpp)
set(storetest_src ${all_storetest_src})
add_executable(storetest ${storetest_src})

if(ROCKSDB_FOUND)
  set_source_files_properties(${storetest_src} COMPILE_FLAGS "-std=gnu++11")
endif(ROCKSDB_FOUND)
if(REDIS3M_FOUND)
target_link_libraries(storetest ${REDIS3M_LIBRARY})
endif(REDIS3M_FOUND)

# Link the executable 
link_directories(${SQLITE3_LIBRARY_DIRS})
link_directories(${ROCKSDB_LIBRARY_DIRS})
link_directories(${REDIS3M_LIBRARY_DIRS})
target_link_libraries(storetest klio ${Boost_LIBRARIES} )

ADD_TEST(StoreTest ${CMAKE_CURRENT_BINARY_DIR}/storetest TestStore)

# add programs to the install target - do not install the hashbench benchmark program.
#INSTALL(PROGRAMS 
#  ${CMAKE_CURRENT_BINARY_DIR}/klio-add
#  DESTINATION bin)
#
