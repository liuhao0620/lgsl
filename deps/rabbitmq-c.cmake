#cmake_minimum_required(VERSION 3.11.2)
#project(rabbitmq-c)

set(RABBITMQ_DIR ${CMAKE_CURRENT_LIST_DIR}/rabbitmq-c)
add_subdirectory(${RABBITMQ_DIR} rabbitmq)
