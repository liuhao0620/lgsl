#cmake_minimum_required(VERSION 3.11.2)
#project(lredis)

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/../../deps/hiredis-win.cmake)
else(WIN32)
endif(WIN32)
set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/lua)
set(LUA_WRAPPER_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../lua_wrapper)
set(LREDIS_SRC ${CMAKE_CURRENT_LIST_DIR}/lredis.cpp)

add_library(lredis SHARED ${LREDIS_SRC})
target_include_directories(lredis PUBLIC ${LUA_INC_DIR} ${HIREDIS_INC_DIR} ${LUA_WRAPPER_INC_DIR})
target_link_libraries(lredis ${LUA_LIB} ${HIREDIS_LIB})
if(WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
