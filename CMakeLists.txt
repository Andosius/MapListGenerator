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


# Add JSON (nlohmann_json)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(libs/json)

# Add pugixml
add_subdirectory(libs/pugixml)


# Add executable
add_executable(MTAListGenerator ${SOURCE_FILES} ${HEADER_FILES})

# Links libraries with executable
target_link_libraries(MTAListGenerator PRIVATE nlohmann_json::nlohmann_json pugixml)