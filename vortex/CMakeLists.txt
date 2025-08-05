add_library(vortex
    src/node.cpp
    src/seq_kv_node.cpp
)

target_include_directories(vortex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Catch2 REQUIRED)
find_package(concurrencpp REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(vortex PUBLIC concurrencpp::concurrencpp nlohmann_json::nlohmann_json)

add_executable(tests tests/test_node.cpp)
target_link_libraries(tests PRIVATE vortex Catch2::Catch2WithMain)
