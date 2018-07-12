#cmake_minimum_required(VERSION 3.11.2)
#project(checks)

set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/lua)
set(CHECKS_DIR ${CMAKE_CURRENT_LIST_DIR}/checks)
set(CHECKS_SRC ${CHECKS_DIR}/checks.c)

add_library(checks SHARED ${CHECKS_SRC})
target_include_directories(checks PUBLIC ${LUA_INC_DIR})
target_link_libraries(checks ${LUA_LIB})
if (WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB -DCHECKS_API=LUALIB_API)
endif(WIN32)
