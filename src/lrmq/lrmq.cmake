#cmake_minimum_required(VERSION 3.11.2)
#project(lkcp)

set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../../deps/lua)
set(LAMQ_SRC ${CMAKE_CURRENT_LIST_DIR}/lrmq.cpp)
set(RMQ_LIB ${LIBRARY_OUTPUT_PATH}/Debug/librabbitmq.4.lib)
set(LUA_WRAPPER_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/../lua_wrapper)
if (WIN32)
    set(RMQ_LIB ${RMQ_LIB}
        ws2_32
        userenv
        iphlpapi
        psapi)
endif (WIN32)

add_library(lrmq SHARED ${LAMQ_SRC})
target_include_directories(lrmq PUBLIC ${LUA_INC_DIR} ${RMQ_INC_DIR} ${LUA_WRAPPER_INC_DIR})
target_link_libraries(lrmq ${LUA_LIB} ${RMQ_LIB})
find_package(OpenSSL 0.9.8 REQUIRED)
target_link_libraries(lrmq ${OPENSSL_LIBRARIES})
if(WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB)
endif(WIN32)
