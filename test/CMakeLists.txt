find_package(doctest REQUIRED)

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(test ${sources})
target_link_libraries(test coral_task_manager doctest::doctest)