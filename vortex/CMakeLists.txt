add_library(vortex
    src/node.cpp
)

target_include_directories(vortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Catch2 REQUIRED)

add_executable(tests tests/test_node.cpp)
target_link_libraries(tests PRIVATE vortex Catch2::Catch2WithMain)
