cmake_minimum_required(VERSION 3.21)

# Options
option(BUILD_TESTS "Build the tests" OFF)

# Project config
project(coral_task_manager LANGUAGES CXX)
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

# Add src and test
add_subdirectory(src)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
