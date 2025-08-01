cmake_minimum_required(VERSION 3.15)
project(gossip_glomers CXX)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vortex)
add_subdirectory(echo)
add_subdirectory(unique-ids)
add_subdirectory(broadcast)
add_subdirectory(grow-only)