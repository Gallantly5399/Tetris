//
// Created by amemiya7 on 2024/6/18.
//

#include "Game.h"

void Game::insertBlock() {
    const auto& shape = block.getShape();
    const auto& color = block.getColor();
    int startRow = block.getStartRow();
    int startColumn = block.getStartColumn();
    for (int column = 0; column < shape.size(); column ++) {
        for (int row = 0; row < shape.size(); row ++) {
            if (shape[column][row] == 1) {
                grid.fill(startRow + row, startColumn + column, color);
            }
        }
    }

}

void Game::processKeyEvents() {
    sf::Event event;
    sf::RenderWindow& window = ui.getWindow();
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
            } else if (event.key.scancode == sf::Keyboard::Scan::H) {
                hold();
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.scancode == sf::Keyboard::Scan::S) {
                gravity.unsetSoftDrop();
            }
        }
    }
}

void Game::tick() {
    sf::RenderWindow& window = ui.getWindow();
    //key events
    isHardDrop = false;
    processKeyEvents();
    //TODO:: refactor block score api
    double passedTime = clock.restart().asSeconds();
    time += passedTime;
    if (time >= gravity.getFallTime()) {
        block.moveDown(grid);
        time = 0;
    }

    Block transparentBlock = block.getTransparentBlock();
    while(transparentBlock.moveDown(grid));
    if (isHardDrop) while(block.moveDown(grid));
    if (block.touch(grid)) {
        if (isLockDelay && !isHardDrop) {
            if (isTouchedGround) {
                lockDelayTime += passedTime;
                if (lockDelayTime >= 0.5 || movement >= 10) {
                    insertBlock();
                    isHold = false;
                    block = generator.nextBlock();
                    gravity.unsetSoftDrop();
                    lockDelayTime = movement = 0;
                }
            } else {
                lockDelayTime = 0;
                isTouchedGround = true;
            }
        }
        else {
            insertBlock();
            isHold = false;
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

Block Game::getHoldBlock() const {
    return holdBlock;
}

void Game::hold() {
    if (isHold) return;
    isHold = true;
    if (holdBlock.empty()) {
        holdBlock = Block(block.getType());
        block = generator.nextBlock();
        return;
    }
    BlockType temType = holdBlock.getType();
    holdBlock = Block(block.getType());
    block = Block(temType);
}
