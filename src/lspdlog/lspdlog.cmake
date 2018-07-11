#cmake_minimum_required(VERSION 3.11.2)
#project(lspdlog)

set(LSPDLOG_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/spdlog)
set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/lua)
set(LSPDLOG_SRC ${CMAKE_CURRENT_LIST_DIR}/lspdlog.cpp)

add_library(lspdlog SHARED ${LSPDLOG_SRC})
target_include_directories(lspdlog PUBLIC ${LUA_INC_DIR} ${LSPDLOG_DIR}/include/spdlog)
target_link_libraries(lspdlog lualib)
if(WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
