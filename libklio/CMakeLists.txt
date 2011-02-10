# -*- mode: cmake; -*-

#set(hdrs 
#  common.hpp
#)
#install(FILES ${hdrs} DESTINATION include/klio)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(sqlite3)

include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS})
file(GLOB all_klio_src *.cpp *.hpp)
set(klio_src ${all_klio_src})
ADD_LIBRARY(klio ${klio_src})

ADD_DEPENDENCIES(klio klio_sqlite3)

target_link_libraries(klio ${Boost_IOSTREAMS_LIBRARY})

if(BUILD_SHARED_LIBS)
  install(TARGETS klio LIBRARY DESTINATION lib)
else(BUILD_SHARED_LIBS)
  install(TARGETS klio ARCHIVE DESTINATION lib)
endif(BUILD_SHARED_LIBS)

