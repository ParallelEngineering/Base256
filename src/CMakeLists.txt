add_library(Base256
        base256.cpp
        math_utils.cpp
        math_utils.h
)

# Add src directory to path for header includes
target_include_directories(Base256
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)