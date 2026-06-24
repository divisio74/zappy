cmake_minimum_required(VERSION 3.20)
project(zappy_server)

file(GLOB_RECURSE SERVER_SRC src/*.cpp)

add_executable(zappy_server ${SERVER_SRC})

target_compile_features(zappy_server PRIVATE cxx_std_20)
target_include_directories(zappy_server PRIVATE include)

set_target_properties(zappy_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
