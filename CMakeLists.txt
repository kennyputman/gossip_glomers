cmake_minimum_required(VERSION 3.15)
project(gossip_glomers CXX)

# Conan integration
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subprojects
add_subdirectory(vortex)
add_subdirectory(echo)