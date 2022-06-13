cmake_minimum_required(VERSION 3.2)

project(deltarune-fr-patcher)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Building Libs

add_subdirectory(external/NFD)

set(BUILD_SHARED_LIBS TRUE)
add_subdirectory(external/CURL)
set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(external/SFML)
add_subdirectory(external/BOXER)

# Sources
add_executable(${PROJECT_NAME} src/main.cpp src/BPSParser.cpp src/CRC32.cpp src/UI/Button.cpp src/UI/ProgressBar.cpp src/UI/UpdaterWindow.cpp src/utils.cpp src/UI/UpdaterWindowInstall.cpp src/UI/UpdaterWindowUninstall.cpp)

# Linking

target_include_directories(${PROJECT_NAME} PUBLIC external/NFD/src/include PUBLIC external/CURL/include PUBLIC external/SFML/include PUBLIC external/BOXER/include PUBLIC inc)
target_link_libraries(${PROJECT_NAME} nfd Boxer ${LIBCURL_OUTPUT_NAME} sfml-system sfml-window sfml-graphics)
