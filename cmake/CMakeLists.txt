cmake_minimum_required (VERSION 3.11.2)

project(glory)

include(../deps/libuv.cmake)
include(../deps/zstd.cmake)
include(../deps/lua.cmake)
include(../deps/luasql.cmake)
include(../deps/luv.cmake)
include(../deps/lua-protobuf.cmake)
include(../deps/checks.cmake)
include(../src/lkcp/lkcp.cmake)
include(../src/lspdlog/lspdlog.cmake)
include(../src/lzstd/lzstd.cmake)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/../output)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/../output)
