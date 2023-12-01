cmake_minimum_required(VERSION 3.22)

project(gossip_glomers 
        VERSION 1.0
        DESCRIPTION "Solutions to the Gossip Glomers challenge by fly.io"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)

add_library(maelstrom INTERFACE)
target_include_directories(maelstrom INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# ------------ echo ------------ #
add_executable(echo src/echo/echo.cpp)
target_include_directories(echo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(echo PRIVATE fmt::fmt nlohmann_json::nlohmann_json stduuid maelstrom )


# ------------ Unique ID ------------ #
add_executable(unique_id src/unique_id/unique_id.cpp)
target_include_directories(unique_id PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(unique_id PRIVATE fmt::fmt nlohmann_json::nlohmann_json stduuid maelstrom )