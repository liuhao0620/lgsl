#cmake_minimum_required(VERSION 3.11.2)
#project(lkcp)

set(KCP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/kcp)
set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/lua)
set(LKCP_SRC ${KCP_DIR}/ikcp.c ${CMAKE_CURRENT_LIST_DIR}/lkcp.cpp)

add_library(lkcp SHARED ${LKCP_SRC})
target_include_directories(lkcp PUBLIC ${LUA_INC_DIR} ${KCP_DIR})
target_link_libraries(lkcp lualib)
if(WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
