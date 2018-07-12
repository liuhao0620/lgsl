#cmake_minimum_required(VERSION 3.11.2)
#project(lpb)

set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/lua)
set(LPB_DIR ${CMAKE_CURRENT_LIST_DIR}/lua-protobuf)
set(LPB_SRC ${LPB_DIR}/pb.c)

add_library(lpb SHARED ${LPB_SRC})
set_target_properties(lpb PROPERTIES OUTPUT_NAME "pb")
target_include_directories(lpb PUBLIC ${LUA_INC_DIR})
target_link_libraries(lpb ${LUA_LIB})
if (WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB -DCHECKS_API=LUALIB_API)
endif(WIN32)