cmake_minimum_required(VERSION 3.28)
project(VulkanTetris)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SFML COMPONENTS system window graphics network audio REQUIRED)
include_directories(${SFML_INCLUDE_DIRS})

find_library(SFML_AUDIO PATHS ${PROJECT_SOURCE_DIR}/dependence/include/SFML/lib sfml-audio-s)
message(${SFML_AUDIO})
add_executable(${PROJECT_NAME} main.cpp Game.cpp Game.h Block.cpp Block.h Grid.cpp Grid.h Gravity.cpp Gravity.h)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
target_compile_definitions(${PROJECT_NAME} PUBLIC FILE_LOCATION=\"${PROJECT_SOURCE_DIR}\")