# Dependencies
find_package(Threads REQUIRED)

# List the sources
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Configure lib
add_library(coral_task_manager STATIC ${sources})
target_include_directories(coral_task_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(coral_task_manager PRIVATE Threads::Threads)

# Install headers