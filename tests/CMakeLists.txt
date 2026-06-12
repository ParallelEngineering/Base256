# Prevent CMake from re-downloading Catch2 every time CLion reloads
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Don't update FetchContent on every config" FORCE)

# Download Catch2
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

# Find Catch 2 CMakeList
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(Base256-Tests
        test_base256.cpp
)
target_link_libraries(Base256-Tests
        PRIVATE
        Base256
        Catch2::Catch2WithMain
)

# Catch2 integration in CTest
catch_discover_tests(Base256-Tests
        TEST_PREFIX base256:
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
