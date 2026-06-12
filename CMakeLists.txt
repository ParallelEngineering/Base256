cmake_minimum_required(VERSION 3.28.3)
project(Base256)

# Enforce C++20 and disable compiler-specific extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(tests)
    endif()
else()
    message(STATUS "Skipping CTest/tests because CMAKE_BUILD_TYPE != Debug")
endif()