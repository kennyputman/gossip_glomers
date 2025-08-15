add_executable(single_node
    single_node.cpp
)

add_executable(multi_node
    multi_node.cpp
)

target_link_libraries(single_node PRIVATE vortex)
target_link_libraries(multi_node PRIVATE vortex)