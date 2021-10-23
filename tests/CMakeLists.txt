project(tests)

# enable testing
enable_testing()
find_package(GTest CONFIG REQUIRED)
find_package(hayai CONFIG REQUIRED)

# tests
add_subdirectory(unit)
add_subdirectory(benchmark)