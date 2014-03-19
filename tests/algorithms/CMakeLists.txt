# -*- mode: cmake; -*-

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#file(GLOB all_algorithmstest_src *.cpp *.hpp)
set(algorithmstest_src 
  test_algorithms.cpp
  main.cpp
)
          
add_executable(algorithmstest ${algorithmstest_src})

if(ROCKSDB_FOUND)
  set_source_files_properties(${algorithmstest_src} COMPILE_FLAGS "-std=gnu++11")
  target_link_libraries(algorithmstest klio ${ROCKSDB_LIBRARY} snappy)
endif(ROCKSDB_FOUND)

ADD_TEST(AlgorithmTest ${CMAKE_CURRENT_BINARY_DIR}/algorithmstest TestAlgorithms)

# Link the executable 
target_link_libraries(algorithmstest klio ${Boost_LIBRARIES} )
