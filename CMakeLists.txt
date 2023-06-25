cmake_minimum_required(VERSION 3.12)
project(MTAListGenerator)


# Set the C++-Standard to C++17 for std::filesystem support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set output directory to "bin" in root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


# Set source & header files
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB HEADER_FILES src/*.hpp src/*.h)


# Enable dependency automation (does copy libraries into output dir)
set(VCPKG_APPLOCAL_DEPS ON)

# Add dependencies from vcpkg
find_package(nlohmann_json CONFIG REQUIRED)
find_package(minizip-ng CONFIG REQUIRED)

# Add pugixml
add_subdirectory(libs/pugixml)


# Add executable
add_executable(MTAListGenerator ${SOURCE_FILES} ${HEADER_FILES})


# Links libraries with executable
target_link_libraries(MTAListGenerator PRIVATE nlohmann_json::nlohmann_json pugixml MINIZIP::minizip-ng)


# Set project name as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MTAListGenerator)
# Set vcpkg enabled to true in Visual Studio
set_target_properties(MTAListGenerator PROPERTIES VS_GLOBAL_VcpkgEnabled true)
