#cmake_minimum_required(VERSION 3.11.2)
#project(rabbitmq-c)

set(RABBITMQ_DIR ${CMAKE_CURRENT_LIST_DIR}/rabbitmq-c)
add_subdirectory(${RABBITMQ_DIR} rabbitmq)

set(RMQ_INC_DIR ${RABBITMQ_DIR}/librabbitmq)
if (WIN32)
    set(RMQ_INC_DIR ${RMQ_INC_DIR} ${RABBITMQ_DIR}/librabbitmq/win32)
endif (WIN32)
if (MSVC)
  set(RMQ_INC_DIR ${RMQ_INC_DIR} ${RABBITMQ_DIR}/librabbitmq/win32/msinttypes)
endif (MSVC)
