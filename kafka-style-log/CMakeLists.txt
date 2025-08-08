add_executable(kafka
    main.cpp
)

target_link_libraries(kafka PRIVATE vortex)