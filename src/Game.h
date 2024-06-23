//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Block.h"
#include "Grid.h"
#include "UI.h"
#include <chrono>
#include "Gravity.h"
#include "Generator.h"

enum class scoreType {
    single = 100,
    doubleLine = 300,
    tripleLine = 500,
    tetris = 800,
    softDrop = 1,
    hardDrop = 2,

};
class Game {
public:
    //every tick render mainWindow and nextWindow and the moving block
    //if timer > fall time then block move down
    double frameTime = 1.0 / 60 * 1000;
    bool firstDraw = false;
    void tick() {
        sf::RenderWindow& window = ui.getWindow();
        //key events
        bool isHardDrop = false;
        sf::Event event;
        while (window.pollEvent(event)) {
//            std::cout << gravity.getFallTime() << std::endl;
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                    window.close();
                } else if (event.key.scancode == sf::Keyboard::Scan::W) {
                    bool state = block.rotate(grid);
                    if (isTouchedGround && state) movement ++, lockDelayTime = 0;
                    //rotate the block
                } else if (event.key.scancode == sf::Keyboard::Scan::A) {
                    bool state = block.moveLeft(grid);
                    if (isTouchedGround && state) movement ++, lockDelayTime = 0;
                } else if (event.key.scancode == sf::Keyboard::Scan::D) {
                    bool state = block.moveRight(grid);
                    if (isTouchedGround && state) movement ++, lockDelayTime = 0;
                } else if (event.key.scancode == sf::Keyboard::Scan::S) {
                    gravity.setSoftDrop();
                } else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                    isHardDrop = true;
                } else if (event.key.scancode == sf::Keyboard::Scan::Q) {
                    block.rotateCounterClockwise(grid);
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.scancode == sf::Keyboard::Scan::S) {
                    gravity.unsetSoftDrop();
                }
            }
        }

        //TODO:: refactor block score api
        double passedTime = clock.restart().asSeconds();
        time += passedTime;
        if (time >= gravity.getFallTime()) {
            block.moveDown(grid);
            time = 0;
        }

        Block transparentBlock = block.getTransparentBlock();
        while(transparentBlock.moveDown(grid));
        //TODO:: add lock delay 0.5s and maximum movement of 10
        if (isHardDrop) while(block.moveDown(grid));
        if (block.touch(grid)) {
            if (isLockDelay && !isHardDrop) {
                if (isTouchedGround) {
                    lockDelayTime += passedTime;
                    if (lockDelayTime >= 0.5 || movement >= 10) {
                        insertBlock();
                        block = generator.nextBlock();
                        gravity.unsetSoftDrop();
                    }
                } else {
                    lockDelayTime = 0;
                    isTouchedGround = true;
                }
            }
            else {
                insertBlock();
                block = generator.nextBlock();
                gravity.unsetSoftDrop();
            }
        } else {
            isTouchedGround = false;
            movement = 0;
        }
        if (grid.exceed()) {
            //TODO:: game over
            stop();
            window.close();
        }
    }
    Game() : block(BlockType::O), grid(10, 22), ui(), generator(), gravity() {
        block = generator.nextBlock();
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
    void insertBlock();
    const unsigned int Height = 10, Width = 20;
    const unsigned int NextWidth = 3, NextHeight = 6, NextCount = 1;
    sf::RenderWindow& getWindow() {
        return ui.getWindow();
    }
    Gravity& getGravity() {
        return gravity;
    }
    Grid& getGrid() {
        return grid;
    }
    Block& getBlock() {
        return block;
    }

    Generator& getGenerator() {
        return generator;
    }
    void stop() {
        isRunning = false;
    }
    bool shouldStop() {
        return !isRunning;
    }

    bool shouldClose() {
        return !ui.getWindow().isOpen();
    }
private:
    bool isTouchedGround = false;
    int movement = 0;
    bool isRunning = true;
    //input the cube data into the render
    enum class BlockMovement {
        left,
        right,
        down,
        rotate
    };
    static BlockMovement blockMovement;
    unsigned int mainWindowWidth, mainWindowHeight;
    unsigned int nextWindowWidth, nextWindowHeight;
//    const unsigned int mainWindowWidth = 1200, mainWindowHeight = 675;
//    const unsigned int nextWindowWidth = 400 , nextWindowHeight = 800;
    bool isLockDelay = true;
    double lockDelayTime = 0;
    double time = 0;

    sf::Clock clock;
    Block block;
    Grid grid;
    UI ui;
    Generator generator;
    Gravity gravity;
    //map two dimenstions to one dimension
    int index(int x, int y) {
        return x + y * Width;
    }

};
