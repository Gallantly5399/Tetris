//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Block.h"
#include "Grid.h"
#include <chrono>

class Game {
public:
    //every tick render mainWindow and nextWindow and the moving block
    //if timer > fall time then block move down
    double frameTime = 1.0 / 60 * 1000;
    bool firstDraw = false;
    void tick() {
        static auto lastUpdateTime = std::chrono::steady_clock::now();
        if (!firstDraw) {
            firstDraw = true;
            lastUpdateTime = std::chrono::steady_clock::now();
        }

        auto currentTime = std::chrono::steady_clock::now();
        double durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - lastUpdateTime).count();
        if (durationTime >= frameTime) {
            lastUpdateTime = std::chrono::steady_clock::now();
        }
    }
    void insertGridBlock(const Block& block) {
        const auto& shape = block.getShape();
        const auto& color = block.getColor();
        grid.insertBlock(shape, color, block.getPosition().startRow, block.getPosition().startColumn);
    }
    Game(unsigned int windowWidth, unsigned int windowHeight,int gameWidth, int gameHeight):
    block(BlockType::O), grid(gameWidth, gameHeight) {
//        render.resizeData(4000);
    }
    const unsigned int Height = 10, Width = 20;
    const unsigned int NextWidth = 3, NextHeight = 6, NextCount = 1;
private:
    //input the cube data into the render
    enum class BlockMovement {
        left,
        right,
        down,
        rotate
    };
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static BlockMovement blockMovement;
    unsigned int mainWindowWidth, mainWindowHeight;
    unsigned int nextWindowWidth, nextWindowHeight;
//    const unsigned int mainWindowWidth = 1200, mainWindowHeight = 675;
//    const unsigned int nextWindowWidth = 400 , nextWindowHeight = 800;
    Block block;
    Grid grid;
    //map two dimenstions to one dimension
    int index(int x, int y) {
        return x + y * Width;
    }

};
