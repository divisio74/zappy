cmake_minimum_required(VERSION 3.20)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS
    Graphics
    Window
    System
    Audio
    Network
)
project(zappy_gui)

file(GLOB_RECURSE GUI_SRC src/*.cpp)

add_executable(zappy_gui ${GUI_SRC})

target_compile_features(zappy_gui PRIVATE cxx_std_20)
target_include_directories(zappy_gui PRIVATE include)

target_link_libraries(zappy_gui PRIVATE
    OpenGL::GL
    GLEW::GLEW
    assimp::assimp
    glfw
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    SFML::Network
)

set_target_properties(zappy_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
