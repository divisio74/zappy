cmake_minimum_required(VERSION 3.20)
project(zappy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options utiles
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add_subdirectory(gui)
add_subdirectory(server)
# add_subdirectory(ai)
