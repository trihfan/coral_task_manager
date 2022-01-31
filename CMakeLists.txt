cmake_minimum_required(VERSION 3.22)

# Options
option(BUILD_TESTS "Build the tests" OFF)
OPTION(BUILD_SHARED_LIBS "Build as shared library" ON)

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
