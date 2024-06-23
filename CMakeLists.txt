cmake_minimum_required(VERSION 3.20)
project(Tetris)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
include_directories(${SFML_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} src/main.cpp src/Block.cpp src/Grid.cpp src/Gravity.cpp src/UI.cpp src/UI.h src/Generator.cpp src/Generator.h src/Game.cpp)
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics)
target_compile_definitions(${PROJECT_NAME} PUBLIC FILE_LOCATION=\"${PROJECT_SOURCE_DIR}\")