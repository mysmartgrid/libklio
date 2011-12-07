# -*- mode: cmake; -*-

configure_file(testconfig.h.in ${CMAKE_BINARY_DIR}/testconfig.h)

add_subdirectory(store)
add_subdirectory(sensors)
add_subdirectory(time)

