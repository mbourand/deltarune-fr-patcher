cmake_minimum_required(VERSION 3.23)

project(deltarune-fr-patcher)

set(CMAKE_CXX_STANDARD 20)

# Sources


add_executable(${PROJECT_NAME} src/main.cpp src/BPSParser.cpp src/CRC32.cpp)

# Building Libs

add_subdirectory(external/NFD)

# Linking

target_include_directories(${PROJECT_NAME} PUBLIC external/NFD/src/include PUBLIC inc)
target_link_libraries(${PROJECT_NAME} nfd)
