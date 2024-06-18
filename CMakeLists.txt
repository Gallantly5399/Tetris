cmake_minimum_required(VERSION 3.28)
project(VulkanTetris)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)


include_directories(dependence/include)


add_executable(${PROJECT_NAME} main.cpp Window.cpp Window.h)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)
target_compile_definitions(${PROJECT_NAME} PUBLIC FILE_LOCATION=\"${PROJECT_SOURCE_DIR}\")