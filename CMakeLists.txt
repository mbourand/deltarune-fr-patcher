cmake_minimum_required(VERSION 3.23)

project(deltarune-fr-patcher)

set(CMAKE_CXX_STANDARD 20)

# Building Libs

add_subdirectory(external/NFD)

set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(external/CURL)
add_subdirectory(external/SFML)
add_subdirectory(external/BOXER)

# Sources
add_executable(${PROJECT_NAME} src/main.cpp src/BPSParser.cpp src/CRC32.cpp src/UI/Button.cpp src/UI/ProgressBar.cpp src/UI/UpdaterWindow.cpp src/utils.cpp src/UI/UpdaterWindowInstall.cpp)

# Linking

target_include_directories(${PROJECT_NAME} PUBLIC external/NFD/src/include PUBLIC external/CURL/include PUBLIC external/SFML/include PUBLIC external/BOXER/include PUBLIC inc)
target_link_libraries(${PROJECT_NAME} nfd Boxer ${CURL_LIBRARIES} sfml-system sfml-window sfml-graphics)
