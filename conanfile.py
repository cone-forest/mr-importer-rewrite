from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class mr_importerRecipe(ConanFile):
    name = "mr-importer"
    version = "0.1"
    package_type = "library"

    # Optional metadata
    license = "MIT"
    author = "Michael Tsukanov mt6@4j-company.ru"
    url = "https://github.com/4j-company/mr-importer"
    description = "Opinionated asset importer"
    topics = ("Asset", "3D", "Computer Graphics")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    requires = [
        "folly/2024.08.12.00",
        "efsw/1.4.1",
        "meshoptimizer/0.23",
        "fastgltf/0.8.0",
        "stb/cci.20240531",
        "fmt/10.2.1",
        "gtest/1.14.0",
    ]

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["mr-importer"]

