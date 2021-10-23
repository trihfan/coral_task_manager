cmake_minimum_required(VERSION 3.16)

# project
set(version "1.0.0")
project(coral_task_manager LANGUAGES CXX)

# set c++17
set(CMAKE_CXX_STANDARD 17)

# modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# options
option(BUILD_TESTS "build the tests" OFF)

# task manager lib
add_subdirectory(src)

# tests
if(BUILD_TESTS)
    include(set_test)
    add_subdirectory(tests)
endif()