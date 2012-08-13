
include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
                  #file(GLOB all_algorithmstest_src *.cpp *.hpp)
set(algorithmstest_src 
  test_algorithms.cpp
  main.cpp
)
          
add_executable(algorithmstest ${algorithmstest_src})

ADD_TEST(AlgorithmTest ${CMAKE_CURRENT_BINARY_DIR}/algorithmstest TestAlgorithms)

# Link the executable 
target_link_libraries(algorithmstest klio ${Boost_LIBRARIES} )
