project(zappy_server)

file(GLOB_RECURSE SERVER_SRC src/*.cpp)

add_executable(zappy_server ${SERVER_SRC})

target_include_directories(zappy_server PRIVATE include)