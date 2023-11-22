cmake_minimum_required(VERSION 3.22)

project(gossip_glomers 
            VERSION 1.0
            DESCRIPTION "Solutions to the Gossip Glomers challenge by fly.io"
            LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(fmt)
find_package(nlohmann_json REQUIRED)

# ----------- ECHO --------------- #

include_directories(echo/include)

add_executable(echo 
    echo/EchoServer.cpp
    echo/echo.cpp
)
target_link_libraries(echo PRIVATE fmt::fmt nlohmann_json::nlohmann_json)

