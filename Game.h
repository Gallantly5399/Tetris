//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Render.h"
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
            render.clear();
            renderBlock();
            renderGrid();
            render.render();
        }

        auto currentTime = std::chrono::steady_clock::now();
        double durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count();
        if (durationTime >= frameTime) {
            render.clear();
            renderBlock();
            renderGrid();
            render.render();
            lastUpdateTime = std::chrono::steady_clock::now();
        }
//        bool moveState = false;
//        if (blockMovement == BlockMovement::left) {
//            moveState = block.moveLeft(grid);
//        } else if (blockMovement == BlockMovement::right) {
//            moveState = block.moveRight(grid);
//        } else if (blockMovement == BlockMovement::down) {
//            moveState = block.moveDown(grid);
//        } else if (blockMovement == BlockMovement::rotate) {
//            moveState = block.rotate(grid);
//        }
//        render.inputCube(block.getPosition(), render.blockWidth, blockLength);
//        blockMovement = BlockMovement::down;
//        if (block.isCollide(grid)) {
//            block = Block(BlockType::O);
//            grid.fill(block);
//        }
//        grid.clear();
//        grid.fill(block);
//        grid.clearLine();
//        render.clear();
//        render.draw(grid);
//        render.draw(block);
//        render.drawNext(block);
//        render.draw();
    }
    void insertGridBlock(const Block& block) {
        const auto& shape = block.getShape();
        const auto& color = block.getColor();
        grid.insertBlock(shape, color, block.getPosition().startRow, block.getPosition().startColumn);
    }
    Game(unsigned int windowWidth, unsigned int windowHeight,int gameWidth, int gameHeight):
    render(windowWidth, windowHeight, nullptr), block(BlockType::O), grid(gameWidth, gameHeight) {
//        render.resizeData(4000);
    }
    const unsigned int Height = 10, Width = 20;
    const unsigned int NextWidth = 3, NextHeight = 6, NextCount = 1;
    Render& getRender() {
        return render;
    }
private:
    //input the cube data into the render
    void inputCube(int row, int column, float width, float height, glm::vec3 color) {
        render.inputCube(render.blockToWorld(row, column), width, height, color);
    }
    void renderBlock() {
        const auto& shape = block.getShape();
        const auto& color = block.getColor();
        const auto& position = block.getPosition();
        for (int i = 0;i < shape.size(); i ++) {
            for (int j = 0;j < shape[i].size(); j ++) {
                if (shape[i][j] == 1) {
                    inputCube(position.startRow + i, position.startColumn + j, render.blockWidth, render.blockWidth, color);
                }
            }
        }
    }
    void renderGrid() {
        const auto& tempGrid = grid.getGrid();
        for (int i = 0;i < tempGrid.size(); i ++) {
            for (int j = 0;j < tempGrid[i].size(); j ++) {
                if (tempGrid[i][j] == 1) {
                    inputCube(i, j, render.blockWidth, render.blockWidth, grid.getColor(i, j));
                }
            }
        }
    }
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
    Render render;
    Block block;
    Grid grid;
    //map two dimenstions to one dimension
    int index(int x, int y) {
        return x + y * Width;
    }

};
