find_package(doctest CONFIG REQUIRED)

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(taskmanagertest ${sources})
target_link_libraries(taskmanagertest coral_task_manager doctest::doctest)