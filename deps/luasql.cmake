#cmake_minimum_required(VERSION 3.11.2)
#project(luasql)

set(LUA_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/lua)
if(WIN32)
    set(MYSQL_INC_DIR C:/mysql-8.0.11-winx64/include)
    set(MYSQL_LIB C:/mysql-8.0.11-winx64/lib/libmysql.lib
        C:/mysql-8.0.11-winx64/lib/mysqlclient.lib)
endif(WIN32)

set(LUASQL_SRC ${CMAKE_CURRENT_LIST_DIR}/luasql/src/luasql.c
    ${CMAKE_CURRENT_LIST_DIR}/luasql/src/ls_mysql.c)

add_library(luasql SHARED ${LUASQL_SRC})
target_include_directories(luasql PUBLIC ${LUA_INC_DIR} ${MYSQL_INC_DIR})
target_link_libraries(luasql ${LUA_LIB} ${MYSQL_LIB})
if (WIN32)
    add_definitions(-DLUA_BUILD_AS_DLL -DLUA_LIB -DLUASQL_API=LUALIB_API)
endif(WIN32)
