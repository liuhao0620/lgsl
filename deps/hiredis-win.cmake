#cmake_minimum_required(VERSION 3.11.2)
#project(hiredis)

set(HIREDIS_DIR ${CMAKE_CURRENT_LIST_DIR}/hiredis-win/hiredis)
set(HIREDIS_SRC ${HIREDIS_DIR}/async.c
    ${HIREDIS_DIR}/dict.c
    ${HIREDIS_DIR}/hiredis.c
    ${HIREDIS_DIR}/net.c
    ${HIREDIS_DIR}/sds.c)

add_library(hiredis STATIC ${HIREDIS_SRC})

set(HIREDIS_LIB hiredis 
    ws2_32
    shlwapi
    kernel32
    user32
    gdi32
    winspool
    comdlg32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    odbc32
    odbccp32)
set(HIREDIS_INC_DIR ${HIREDIS_DIR})
