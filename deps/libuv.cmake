#cmake_minimum_required(VERSION 3.11.2)
#project(libuv)

set(LIBUV_DIR ${CMAKE_CURRENT_LIST_DIR}/libuv)

aux_source_directory(${LIBUV_DIR}/src/win LIBUV_WIN_SRC)
aux_source_directory(${LIBUV_DIR}/src/unix LIBUV_UNIX_SRC)
aux_source_directory(${LIBUV_DIR}/src LIBUV_SRC)

if(WIN32)
    set(LIBUV_SRC ${LIBUV_SRC} ${LIBUV_WIN_SRC})
else(WIN32)
    set(LIBUV_SRC ${LIBUV_SRC} ${LIBUV_UNIX_SRC})
endif(WIN32)

add_library(libuv STATIC ${LIBUV_SRC})
if(WIN32)
    target_include_directories(libuv PUBLIC ${LIBUV_DIR}/include ${LIBUV_DIR}/src)
else(WIN32)
    target_include_directories(libuv PUBLIC ${LIBUV_DIR}/include ${LIBUV_DIR}/src ${LIBUV_DIR}/src/unix)
endif(WIN32)

set(LIBUV_LIB libuv)
if(WIN32)
    set(LIBUV_LIB ${LIBUV_LIB}
        ws2_32
        userenv
        iphlpapi
        psapi)
endif(WIN32)
