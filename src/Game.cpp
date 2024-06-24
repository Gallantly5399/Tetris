//
// Created by amemiya7 on 2024/6/18.
//

#include "Game.h"
#ifndef NDEBUG
#define Debug(x) (std::cerr << #x << ": " << (x) << std::endl)
#else
#define Debug(x)
#endif

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

void Game::processEvents() {
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
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                auto [gridColumn, gridRow] = mousePositionToGridPosition(mousePosition.x, mousePosition.y);
                grid.fill(gridRow,  gridColumn, sf::Color{0x333333ff});
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                auto [gridColumn, gridRow] = mousePositionToGridPosition(mousePosition.x, mousePosition.y);
                grid.clear(gridRow, gridColumn);
            }
        }
    }
}

void Game::tick() {
    sf::RenderWindow& window = ui.getWindow();
    //key events
    isHardDrop = false;
    processEvents();
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
                    ScoreType scoreType = addScore();
                    isHold = false;
                    block = generator.nextBlock();
                    gravity.unsetSoftDrop();
                    lockDelayTime = movement = 0;
                }
            } else {
                lockDelayTime = 0;
                isTouchedGround = true;
            }
        } else {
            insertBlock();
            ScoreType scoreType = addScore();
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

std::pair<int, int> Game::mousePositionToGridPosition(float x, float y) {
    return {(int)(x - 200) / 31, (int)(675 - y - 25) / 31,  };
}

ScoreType Game::addScore() {
    bool isSrs = block.getSrs();
    Grid tempGrid = grid;
    int lines = tempGrid.clearLines();
    ScoreType scoreType = ScoreType::None;
    //T-spin
    if (TSpin()) {
        //T-spin mini
        if (isSrs) {
            if (lines == 0) scoreType = ScoreType::TSpinMiniNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinMiniSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinMiniDouble;
        } else {
            //T-spin
            if (lines == 0) scoreType = ScoreType::TSpinNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
        }
    } else {
        if (lines == 1) scoreType = ScoreType::Single;
        else if (lines == 2) scoreType = ScoreType::Double;
        else if (lines == 3) scoreType = ScoreType::Triple;
        else if (lines == 4) scoreType = ScoreType::Tetris;
    }
    if (isDifficultScore(scoreType)) {
        if (backToBack) score += 1.5 * static_cast<int>(scoreType);
        else score += static_cast<int>(scoreType);
        backToBack = true;
        comboCount ++;
    } else if (scoreType == ScoreType::Single || scoreType == ScoreType::Double || scoreType == ScoreType::Triple) {
        score += static_cast<int>(scoreType);
        backToBack = false;
        comboCount ++;
        backToBack = false;
    } else {
        score += static_cast<int>(scoreType);
        comboCount = 0;
    }
    if (comboCount >= 2) {
        score += (comboCount - 1) * static_cast<int>(ScoreType::Combo);
    }
    grid.clearLines();
    return scoreType;
}

bool Game::TSpin() const {
    if (block.getType() != BlockType::T || block.getLastMovement() != Movement::Rotate) return false;
    const auto& shape = block.getShape();
    auto [startRow, startColumn] = block.getPosition();
    int count = grid.isOccupied(startRow, startColumn) + grid.isOccupied(startRow + 2, startColumn) +
                grid.isOccupied(startRow, startColumn + 2) + grid.isOccupied(startRow + 2, startColumn + 2);
    return count >= 3;
}

int Game::getScore() {
    return score;
}
