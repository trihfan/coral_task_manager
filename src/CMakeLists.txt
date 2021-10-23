# project
project(coral_task_manager LANGUAGES CXX)

# sources
add_library(coral_task_manager INTERFACE)

# includes
target_include_directories(coral_task_manager INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# directory
set_target_properties(coral_task_manager PROPERTIES FOLDER "coral/task_manager")