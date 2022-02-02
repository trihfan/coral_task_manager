# List the sources
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Configure lib
include_directories(${CMAKE_SOURCE_DIR}/src)
add_library(coral_task_manager ${sources})
target_include_directories(coral_task_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Install headers