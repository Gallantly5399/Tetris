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

const unsigned int width = 1600, height = 900;

glm::vec2 worldToScreen(unsigned int x, unsigned int y) {
    return glm::vec2((1.0f * x / width - 0.5) * 2, ((1 - 1.0f * y /height) -0.5)  * 2);
}

void output(glm::vec2 pos) {
    std::cout << pos.x << ' ' << pos.y << std::endl;
}

int main() {
    //counter-clockwise
    VertexVector vertices;
    vertices.data.resize(4000);
    IndexVector indices;
    indices.data.resize(4000);
//    std::vector<Vertex> vertices = {
////            {{0.0f, -0.2f}, {1.0f, 0.0f, 0.0f}},
////            {{0.2f, 0.2f,}, {0.0f, 1.0f, 0.0f}},
////            {{-0.2f, 0.2f}, {0.0f, 0.0f, 1.0f}},
////            {{0.2f, 0.2f}, {1.0f, 1.0f, 1.0f}},
////            {{-0.2f, 0.2f}, {1.0f, 1.0f, 1.0f}},
////            {{-0.2f, -0.2f}, {1.0f, 1.0f, 1.0f}},
////
////            Vertex{glm::vec2(-0.2f, -0.2f), glm::vec3(1.0f, 1.0f, 1.0f)},
////            Vertex{glm::vec2(-0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f)},
////            Vertex{glm::vec2(0.2f, 0.2f), glm::vec3(1.0f, 1.0f, 1.0f)}
//    };
    Block block(BlockType::O);
    block.output();
    block.rotate();
//    std::cout << std::endl;
    block.output();

    Game game(1600, 900, 400, 800);
    Render render = game.getRender();
    Grid Grid(22, 10);
    Grid.fill(0, 0);
    Grid.fill(0, 1);
    Grid.fill(0, 2);
    Grid.fill(1, 1);
    const auto& grid = Grid.getGrid();
    const unsigned int stripe = 2, blockWidth = 30;
    unsigned int startX = 200, startY = 100;
#ifndef NDEBUG
    std::cout << "grid width and height " << grid.size() << ' ' << grid[0].size() << std::endl;
#endif

    for (int i = 0;i < grid.size();i ++) {
        for (int j = 0; j < grid[i].size(); j++) {
            if (grid[i][j] == 1) {

                const unsigned int Ox = startX + (j + 1) * stripe + j * blockWidth, Oy =
                        startY + (i + 1) * stripe + i * blockWidth;
                glm::vec2 leftCorner = worldToScreen(Ox, Oy);
                glm::vec2 leftTop = worldToScreen(Ox, Oy + blockWidth);
                glm::vec2 rightCorner = worldToScreen(Ox + blockWidth, Oy);;
                glm::vec2 rightTop = worldToScreen(Ox + blockWidth, Oy + blockWidth);
                render.inputCube(vertices, indices,{Ox, Oy}, blockWidth, blockWidth);
//                vertices.push_back({leftCorner, glm::vec3(1.0f, 1.0f, 1.0f)});
//                vertices.push_back({rightCorner, glm::vec3(1.0f, 1.0f, 1.0f)});
//                vertices.push_back({leftTop, glm::vec3(1.0f, 1.0f, 1.0f)});
//
//                vertices.push_back({rightCorner, glm::vec3(1.0f, 1.0f, 1.0f)});
//                vertices.push_back({rightTop, glm::vec3(1.0f, 1.0f, 1.0f)});
//                vertices.push_back({leftTop, glm::vec3(1.0f, 1.0f, 1.0f)});
//                output(leftCorner);
//                output(leftTop);
//                output(rightCorner);
//                output(rightTop);
            }
        }
    }
//    inputData(vertices, worldToScreen(startX, startY), worldToScreen(startX + 500, startY)
//              , worldToScreen(startX, startY + 1), worldToScreen(startX + 500, startY + 1));
//    inputData(vertices, worldToScreen(startX, startY), worldToScreen(startX + 1, startY)
//            , worldToScreen(startX, startY + 500), worldToScreen(startX + 1, startY + 500));
//    inputData(vertices, worldToScreen(startX + 500, startY), worldToScreen(startX + 501, startY)
//            , worldToScreen(startX + 500, startY + 500), worldToScreen(startX + 501, startY + 500));
//    glm::vec2 mainWindowLeftCorner = worldToScreen(startX, startY);
//    glm::vec2 mainWindowLeftTop  worldToScreen(startX, startY)

    int frame = 1;
    while(!render.shouldStop()) {
#ifndef NDEBUG
        std::cout << "Current Frame " << frame << std::endl;
        frame++;
#endif
        render.render(vertices.data, indices.data);
    }
}
