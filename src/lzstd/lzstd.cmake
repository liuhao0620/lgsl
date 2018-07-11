#cmake_minimum_required(VERSION 3.11.2)
#project(lzstd)

set(LIBZSTD_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/zstd/lib)
set(LZSTD_SRC ${CMAKE_CURRENT_LIST_DIR}/lzstd.cpp)

add_library(lzstd SHARED ${LZSTD_SRC})
target_include_directories(lzstd PUBLIC ${LUA_INC_DIR} ${LIBZSTD_INC_DIR})
target_link_libraries(lzstd ${LUA_LIB} ${LIBZSTD_LIB})
if(WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
