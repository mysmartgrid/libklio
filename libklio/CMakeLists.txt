# -*- mode: cmake; -*-

file(GLOB klio_hdrs *.h *.hpp)
install(FILES ${klio_hdrs} DESTINATION include/libklio)
file(GLOB klio_sqlite3_hdrs sqlite3/*.h sqlite3/*.hpp)
install(FILES ${klio_sqlite3_hdrs} DESTINATION include/libklio/sqlite3)
file(GLOB klio_algorithm_hdrs algorithm/*.h algorithm/*.hpp)
install(FILES ${klio_algorithm_hdrs} DESTINATION include/libklio/algorithm)
file(GLOB klio_msg_hdrs msg/*.h msg/*.hpp)
install(FILES ${klio_msg_hdrs} DESTINATION include/libklio/msg)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/libklio/config.h)
install(FILES ${CMAKE_BINARY_DIR}/libklio/config.h DESTINATION include/libklio)


include_directories(${CMAKE_SOURCE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIR}
                    ${OPENSSL_INCLUDE_DIR}
)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB klio_src *.cpp *.hpp)
file(GLOB klio_sqlite3_src sqlite3/*.cpp sqlite3/*.hpp)
file(GLOB klio_msg_src msg/*.cpp msg/*.hpp)
file(GLOB klio_algorithm_src algorithm/*.cpp algorithm/*.hpp)
ADD_LIBRARY(klio STATIC ${klio_src} ${klio_sqlite3_src} ${klio_msg_src} ${klio_algorithm_src})

target_link_libraries(klio
#  ${GNUTLS_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CURL_SHARED_LIBRARY}
  ${JSON_LIBRARY}
  ${OPENSSL_CRYPTO_LIBRARY}
)

#if(BUILD_SHARED_LIBS)
# install(TARGETS klio LIBRARY DESTINATION lib)
  #else(BUILD_SHARED_LIBS)
   install(TARGETS klio ARCHIVE DESTINATION lib)
  #endif(BUILD_SHARED_LIBS)

