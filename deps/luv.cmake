#cmake_minimum_required(VERSION 3.11.2)
#project(luv)

set(LUV_DIR ${CMAKE_CURRENT_LIST_DIR}/luv)
set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/lua)
set(LIBUV_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/libuv/include)

set(LUV_SRC ${LUV_DIR}/src/luv.c)

add_library(luv SHARED ${LUV_SRC})
target_include_directories(luv PUBLIC ${LUA_INC_DIR} ${LIBUV_INC_DIR})
target_link_libraries(luv ${LUA_LIB} ${LIBUV_LIB})
if (WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
