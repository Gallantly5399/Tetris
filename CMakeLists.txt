cmake_minimum_required(VERSION 3.20)
project(Tetris)

set(CMAKE_CXX_STANDARD 20)
if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()



find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
include_directories(${SFML_INCLUDE_DIRS} dependencies/tomlplusplus/include dependencies/)

add_executable(${PROJECT_NAME} src/main.cpp src/Block.cpp src/Grid.cpp src/Gravity.cpp src/UI.cpp src/UI.h src/Generator.cpp src/Generator.h src/Game.cpp src/Tuner.h src/Ai.h src/Utility.h src/Utility.cpp)
target_link_libraries(${PROJECT_NAME} sfml-system sfml-window sfml-graphics spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
target_compile_definitions(${PROJECT_NAME} PUBLIC FILE_LOCATION=\"${PROJECT_SOURCE_DIR}\")