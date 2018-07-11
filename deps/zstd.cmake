#cmake_minimum_required(VERSION 3.11.2)
#project(zstd)

set(ZSTD_DIR ${CMAKE_CURRENT_LIST_DIR}/zstd)

aux_source_directory(${ZSTD_DIR}/lib/common ZSTD_COMMON_SRC)
aux_source_directory(${ZSTD_DIR}/lib/compress ZSTD_COMPRESS_SRC)
aux_source_directory(${ZSTD_DIR}/lib/decompress ZSTD_DECOMPRESS_SRC)
aux_source_directory(${ZSTD_DIR}/lib/deprecated ZSTD_DEPRECATED_SRC)
aux_source_directory(${ZSTD_DIR}/lib/dictBuilder ZSTD_DICTBUILDER_SRC)
aux_source_directory(${ZSTD_DIR}/lib/legacy ZSTD_LEGACY_SRC)

set(ZSTD_SRC ${ZSTD_COMMON_SRC} 
    ${ZSTD_COMPRESS_SRC} 
    ${ZSTD_DECOMPRESS_SRC}
    ${ZSTD_DEPRECATED_SRC}
    ${ZSTD_DICTBUILDER_SRC}
    ${ZSTD_LEGACY_SRC})

add_library(libzstd STATIC ${ZSTD_SRC})

target_include_directories(libzstd PUBLIC 
    ${ZSTD_DIR}/lib
    ${ZSTD_DIR}/lib/common 
    ${ZSTD_DIR}/lib/compress
    ${ZSTD_DIR}/lib/decompress
    ${ZSTD_DIR}/lib/deprecated
    ${ZSTD_DIR}/lib/dictBuilder
    ${ZSTD_DIR}/lib/legacy)

add_definitions(-DZSTD_DLL_EXPORT=1 
    -DZSTD_MULTITHREAD=1 
    -DZSTD_LEGACY_SUPPORT=4)

set(LIBZSTD_LIB libzstd)
