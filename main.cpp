#include "vulkan/vulkan.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <iostream>

#include "Render.h"
#include "Block.h"
#include "Grid.h"
#include "Game.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

const unsigned int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 675;

glm::vec2 worldToScreen(unsigned int x, unsigned int y) {
    return glm::vec2((1.0f * x / SCREEN_WIDTH - 0.5) * 2, ((1 - 1.0f * y /SCREEN_HEIGHT) -0.5)  * 2);
}

void output(glm::vec2 pos) {
    std::cout << pos.x << ' ' << pos.y << std::endl;
}

int main() {
    //counter-clockwise
//    VertexVector vertices;
//    vertices.data.resize(4000);
//    IndexVector indices;
//    indices.data.resize(4000);
//    Block block(BlockType::O);
//    block.output();
//    block.rotate();
////    std::cout << std::endl;
//    block.output();

//    Game game(SCREEN_WIDTH, SCREEN_HEIGHT, 10, 22);
//    auto& render = game.getRender();
    Render render = Render(SCREEN_WIDTH,  SCREEN_HEIGHT);
//#ifndef NDEBUG
//    std::cout << "grid width and height " << grid.size() << ' ' << grid[0].size() << std::endl;
//#endif

    int frame = 1;
    while(!render.shouldStop()) {
//#ifndef NDEBUG
//        std::cout << "Current Frame " << frame << std::endl;
//        frame++;
//#endif
//        game.tick();
    }
}
