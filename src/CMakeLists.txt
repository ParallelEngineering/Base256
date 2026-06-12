add_library(Base256
        base256.cpp
)

# Add src directory to path for header includes
target_include_directories(Base256
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)