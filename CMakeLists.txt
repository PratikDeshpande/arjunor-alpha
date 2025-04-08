cmake_minimum_required(VERSION 3.10)
project(arjunor-alpha)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files for the main executable
set(SERVER_SOURCES
    src/server.cpp
    src/resp.cpp
    src/eval.cpp
    src/store.cpp
    src/comm.cpp
)

# Create the main executable
add_executable(arjunor-alpha-server ${SERVER_SOURCES})

# Add include directories
target_include_directories(arjunor-alpha-server PRIVATE src)

# Add test executable
add_executable(test tests/test_resp.cpp src/resp.cpp)
target_include_directories(test PRIVATE src)
target_link_libraries(test PRIVATE pthread gtest)

# Add debug configuration
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Add custom clean target
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}"
    COMMENT "Cleaning all build files"
) 