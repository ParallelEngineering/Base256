add_library(BigInt
        bigint.cpp
        math_utils.cpp
)

# Add src directory to path for header includes
target_include_directories(BigInt
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)