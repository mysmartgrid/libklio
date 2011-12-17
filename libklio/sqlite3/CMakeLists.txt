# -*- mode: cmake; -*-

file(GLOB hdrs *.h *.hpp)
install(FILES ${hdrs} DESTINATION include/klio/sqlite3)

include_directories(${CMAKE_SOURCE_DIR}
                    ${Boost_INCLUDE_DIRS}
                    ${SQLITE3_INCLUDE_DIR}
                    )
file(GLOB all_sqlite3_src *.cpp *.hpp)
set(klio_sqlite3_src ${all_sqlite3_src})
ADD_LIBRARY(klio_sqlite3 SHARED ${klio_sqlite3_src})

#target_link_libraries(klio_sqlite3 klio ${SQLITE3_LIBRARIES})
target_link_libraries(klio_sqlite3 ${SQLITE3_LIBRARIES})

#if(BUILD_SHARED_LIBS)
  install(TARGETS klio_sqlite3 LIBRARY DESTINATION lib)
  #else(BUILD_SHARED_LIBS)
  #  install(TARGETS klio_sqlite3 ARCHIVE DESTINATION lib)
  #endif(BUILD_SHARED_LIBS)

