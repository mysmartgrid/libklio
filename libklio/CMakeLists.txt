# -*- mode: cmake; -*-

if(ROCKSDB_FOUND)
    set(ENABLE_ROCKSDB 1)
endif(ROCKSDB_FOUND)

if(REDIS3M_FOUND)
    set(ENABLE_REDIS3M 1)
endif(REDIS3M_FOUND)

if(POSTGRESQL_FOUND)
    set(ENABLE_POSTGRESQL 1)
endif(POSTGRESQL_FOUND)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/libklio/config.h)
install(FILES ${CMAKE_BINARY_DIR}/libklio/config.h DESTINATION include/libklio)

file(GLOB klio_hdrs *.h *.hpp)
install(FILES ${klio_hdrs} DESTINATION include/libklio)

file(GLOB klio_sqlite3_hdrs sqlite3/*.h sqlite3/*.hpp)
install(FILES ${klio_sqlite3_hdrs} DESTINATION include/libklio/sqlite3)

file(GLOB klio_txt_hdrs txt/*.h txt/*.hpp)
install(FILES ${klio_txt_hdrs} DESTINATION include/libklio/txt)

file(GLOB klio_msg_hdrs msg/*.h msg/*.hpp)
install(FILES ${klio_msg_hdrs} DESTINATION include/libklio/msg)

file(GLOB klio_redis_hdrs redis/*.h redis/*.hpp)
install(FILES ${klio_redis_hdrs} DESTINATION include/libklio/redis)

file(GLOB klio_postgresql_hdrs postgresql/*.h postgresql/*.hpp)
install(FILES ${klio_postgresql_hdrs} DESTINATION include/libklio/postgresql)

if(ROCKSDB_FOUND)
    file(GLOB klio_rocksdb_hdrs rocksdb/*.h rocksdb/*.hpp)
    file(GLOB klio_rocksdb_src rocksdb/*.cpp rocksdb/*.hpp)
    install(FILES ${klio_rocksdb_hdrs} DESTINATION include/libklio/rocksdb)
    set_source_files_properties(${klio_rocksdb_src} COMPILE_FLAGS "-std=gnu++11")
    set_source_files_properties(store-factory.cpp COMPILE_FLAGS "-std=gnu++11")
endif(ROCKSDB_FOUND)

if(REDIS3M_FOUND)
    file(GLOB klio_redis_hdrs redis/*.h redis/*.hpp)
    file(GLOB klio_redis_src redis/*.cpp redis/*.hpp)
    install(FILES ${klio_redis_hdrs} DESTINATION include/libklio/redis)
    set_source_files_properties(${klio_redis_src} COMPILE_FLAGS "-std=gnu++11")
    set_source_files_properties(store-factory.cpp COMPILE_FLAGS "-std=gnu++11")
endif(REDIS3M_FOUND)

if(POSTGRESQL_FOUND)
    file(GLOB klio_postgresql_hdrs postgresql/*.h postgresql/*.hpp)
    file(GLOB klio_postgresql_src postgresql/*.cpp postgresql/*.hpp)
    install(FILES ${klio_postgresql_hdrs} DESTINATION include/libklio/postgresql)
    set_source_files_properties(${klio_postgresql_src} COMPILE_FLAGS "-std=gnu++11")
    set_source_files_properties(store-factory.cpp COMPILE_FLAGS "-std=gnu++11")
endif(POSTGRESQL_FOUND)

file(GLOB klio_algorithm_hdrs algorithm/*.h algorithm/*.hpp)
install(FILES ${klio_algorithm_hdrs} DESTINATION include/libklio/algorithm)

include_directories(${CMAKE_SOURCE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${ROCKSDB_INCLUDE_DIR}
                    ${REDIS3M_INCLUDE_DIR}
                    ${POSTGRESQL_INCLUDE_DIR}
                    ${JSONCPP_INCLUDE}
                    ${LIBMYSMARTGRID_INCLUDE_DIRS}
)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB klio_src *.cpp *.hpp)
file(GLOB klio_sqlite3_src sqlite3/*.cpp sqlite3/*.hpp)
file(GLOB klio_txt_src txt/*.cpp txt/*.hpp)
file(GLOB klio_postgresql_src postgresql/*.cpp postgresql/*.hpp)
file(GLOB klio_msg_src msg/*.cpp msg/*.hpp)

file(GLOB klio_algorithm_src algorithm/*.cpp algorithm/*.hpp)

ADD_LIBRARY(klio STATIC ${klio_src} ${klio_sqlite3_src} ${klio_txt_src} ${klio_msg_src} ${klio_rocksdb_src} ${klio_redis_src} ${klio_postgresql_src} ${klio_algorithm_src})

target_link_libraries(klio
  ${SQLITE3_LIBRARIES}
)
if( ENABLE_ROCKSDB )
  target_link_libraries(klio ${ROCKSDB_LIBRARIES})
endif()
target_link_libraries(klio
  ${Boost_LIBRARIES}
)

if( ENABLE_MSG )
  target_link_libraries(klio ${JSONCPP_LIBRARY} ${LIBMYSMARTGRID_LIBRARIES} ${LIBMYSMARTGRID_LIBRARY})
endif()

if( ENABLE_REDIS3M )
  target_link_libraries(klio ${REDIS3M_LIBRARY})
endif()

if( ENABLE_POSTGRESQL )
  target_link_libraries(klio ${POSTGRESQL_LIBRARY})
endif()

#if(BUILD_SHARED_LIBS)
# install(TARGETS klio LIBRARY DESTINATION lib)
#else(BUILD_SHARED_LIBS)
install(TARGETS klio 
  EXPORT ${PROJECT_NAME}Exports
  ARCHIVE DESTINATION lib
  COMPONENT static-libraries)
#endif(BUILD_SHARED_LIBS)

#install(TARGETS libklio
#  ARCHIVE
#  DESTINATION lib
#  COMPONENT static-libraries)

