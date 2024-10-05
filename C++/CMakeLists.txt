cmake_minimum_required(VERSION 3.19)
project(DeepLearningRecilinearSteiner LANGUAGES CXX)

option(EnableTests "EnableTests" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   add_definitions("-DDLRS_DEBUG")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Output)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY}/Debug/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY}/Release/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY}/Debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY}/Release/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY}/Debug/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY}/Release/lib)

include_directories(Src)
add_subdirectory(Src)

message(STATUS "===================================================")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enable Tests: ${EnableTests}")
message(STATUS "Debug Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Output Directories:")
message(STATUS "    Runtime (Debug): ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
message(STATUS "    Runtime (Release): ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}")
message(STATUS "    Library (Debug): ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}")
message(STATUS "    Library (Release): ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}")
message(STATUS "    Archive (Debug): ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}")
message(STATUS "    Archive (Release): ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}")
message(STATUS "===================================================\n")

if(EnableTests)
   add_subdirectory(Test)
endif()