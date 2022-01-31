from conans import ConanFile, Meson

class coral_task_manager(ConanFile):
    # Parameters
    name = "coral_task_manager"
    version = "1.0"
    url = "https://github.com/trihfan/coral_task_manager"
    license = "MIT"
    description = "Task manager for coral engine"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "conanfile.py"
    generators = "cmake_find_package"
    requires = [
        "doctest/2.4.8"
    ]

