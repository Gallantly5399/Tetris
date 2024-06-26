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
    const auto &shape = block.getShape();
    const auto &color = block.getColor();
    int startRow = block.getStartRow();
    int startColumn = block.getStartColumn();
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape.size(); row++) {
            if (shape[column][row] == 1) {
                grid.fill(startRow + row, startColumn + column, color);
            }
        }
    }

}

void Game::processEvents() {
    sf::Event event;
    sf::RenderWindow &window = ui.getWindow();
    while (window.pollEvent(event)) {
//            std::cout << gravity.getFallTime() << std::endl;
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.scancode == sf::Keyboard::Scan::I) {
                isAiActive = !isAiActive;
                firstBlock = !firstBlock;
            } else if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                window.close();
            } else if (event.key.scancode == sf::Keyboard::Scan::W) {
                bool state = block.rotate(grid);
                if (isTouchedGround && state) movement++, lockDelayTime = 0;
                //rotate the block
            } else if (event.key.scancode == sf::Keyboard::Scan::A) {
                bool state = block.moveLeft(grid);
                if (isTouchedGround && state) movement++, lockDelayTime = 0;
            } else if (event.key.scancode == sf::Keyboard::Scan::D) {
                bool state = block.moveRight(grid);
                if (isTouchedGround && state) movement++, lockDelayTime = 0;
            } else if (event.key.scancode == sf::Keyboard::Scan::S) {
                gravity.setSoftDrop();
            } else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                isHardDrop = true;
            } else if (event.key.scancode == sf::Keyboard::Scan::Q) {
                block.rotateCounterClockwise(grid);
            } else if (event.key.scancode == sf::Keyboard::Scan::H) {
                hold();
            } else if (event.key.scancode == sf::Keyboard::Scan::P) {
                restart();
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.scancode == sf::Keyboard::Scan::S) {
                gravity.unsetSoftDrop();
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                auto [gridColumn, gridRow] = mousePositionToGridPosition(mousePosition.x, mousePosition.y);
                grid.fill(gridRow, gridColumn, sf::Color{0x333333ff});
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                auto [gridColumn, gridRow] = mousePositionToGridPosition(mousePosition.x, mousePosition.y);
                grid.clear(gridRow, gridColumn);
            }
        }
    }
}

#include <iostream>

void Game::tick() {
    //key events
    isHardDrop = false;
    processEvents();
    if (!isRunning) return;
    double passedTime = clock.restart().asSeconds();
    time += passedTime;
    if (time >= gravity.getFallTime()) {
        int count = (int)std::floor(time / gravity.getFallTime());
        for (int i = 0;i < count;i ++) {
            block.moveDown(grid);
        }
        time -= count * gravity.getFallTime();
    }

    Block transparentBlock = block.getTransparentBlock();
    while (transparentBlock.moveDown(grid));
    if (firstBlock && isAiActive) {
        firstBlock = false;
        std::vector<Block> workingPieces = {block, generator.seeNextBlocks(1)[0]};
        ai.add(workingPieces, grid);
    }
    long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    long long durationTime = currentTime - aiLastMoveTime;
    while (isAiActive && durationTime > ai.limitation() && !aiMovement.empty()) {
        int count;
        if (ai.limitation() == 0) count = std::numeric_limits<int>::max();
        else count = durationTime / ai.limitation();
        while(!aiMovement.empty() && count > 0) {
            const Movement &currentMovement = aiMovement.read();
            if (currentMovement == Movement::Rotate) {
//            std::cout << "Rotate" << std::endl;
                block.rotate(grid);
            } else if (currentMovement == Movement::Left) {
//            std::cout << "Left" << std::endl;
                block.moveLeft(grid);
            } else if (currentMovement == Movement::Right) {
//            std::cout << "Right" << std::endl;
                block.moveRight(grid);
            } else if (currentMovement == Movement::Down) {
//            std::cout << "Down" << std::endl;
                while (block.moveDown(grid));
                isHardDrop = true;
                count --;
            }
        }
        aiLastMoveTime = currentTime;
    }

    if (isHardDrop) { while (block.moveDown(grid)); }

    if (block.touch(grid)) {
        if (isLockDelay && !isHardDrop) {
            if (isTouchedGround) {
                lockDelayTime += passedTime;
                if (lockDelayTime >= 0.5 || movement >= 10) {
                    insertBlock();
                    lastScoreType = addScore();
                    isHold = false;
                    if (grid.exceed()) {
                        stop();
                    } else {
                        block = generator.nextBlock();
                        if (isAiActive) {
                            //TODO::AI
                            std::vector<Block> workingPieces = {block, generator.seeNextBlocks(1)[0]};
                            ai.add(workingPieces, grid);
                        };
                    }
                    gravity.unsetSoftDrop();
                    lockDelayTime = movement = 0;

                }
            } else {
                lockDelayTime = 0;
                isTouchedGround = true;
            }
        } else {
            insertBlock();
            lastScoreType = addScore();
            isHold = false;
            gravity.unsetSoftDrop();
            if (grid.exceed()) {
                stop();
            } else {
                block = generator.nextBlock();

                if (isAiActive) {
                    //TODO::AI
                    std::vector<Block> workingPieces = {block, generator.seeNextBlocks(1)[0]};
                    ai.add(workingPieces, grid);
                }
            }
        }
    } else {
        isTouchedGround = false;
        movement = 0;
    }


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
    return {(int) (x - 200) / 31, (int) (675 - y - 25) / 31,};
}

ScoreType Game::addScore() {
    bool isSrs = block.getSrs();
    Grid tempGrid = grid;
    int lines = tempGrid.clearLines();
    ScoreType scoreType = ScoreType::None;
    //T-spin
    if (TSpin()) {
        //T-spin mini
        if (isSrs || block.checkMiniTSpin(grid)) {
            if (lines == 0) scoreType = ScoreType::TSpinMiniNoLines;
            else if (lines == 1) scoreType = ScoreType::TSpinMiniSingle;
            else if (lines == 2) scoreType = ScoreType::TSpinMiniDouble;
            else if (lines == 3) scoreType = ScoreType::TSpinTriple;
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
        if (backToBack) score += 1.5 * scoreTypeToInt(scoreType);
        else score += scoreTypeToInt(scoreType);
        backToBack = true;
        comboCount++;
    } else if (scoreType == ScoreType::Single || scoreType == ScoreType::Double || scoreType == ScoreType::Triple) {
        score += scoreTypeToInt(scoreType);
        backToBack = false;
        comboCount++;
        backToBack = false;
    } else {
        score += scoreTypeToInt(scoreType);
        comboCount = 0;
    }
    if (comboCount >= 2) {
        score += (comboCount - 1) * scoreTypeToInt(ScoreType::Combo);
    }
    gravity.addLines(lines);
    grid.clearLines();
    return scoreType;
}

bool Game::TSpin() const {
    if (block.getType() != BlockType::T || block.getLastMovement() != Movement::Rotate) return false;
    const auto &shape = block.getShape();
    auto [startRow, startColumn] = block.getPosition();
    int count = grid.isOccupied(startRow, startColumn) + grid.isOccupied(startRow + 2, startColumn) +
                grid.isOccupied(startRow, startColumn + 2) + grid.isOccupied(startRow + 2, startColumn + 2);
    return count >= 3;
}

Game::Game() : block(BlockType::O), grid(10, 22), ui(), generator(), gravity(),
               ai(0.42863, 0.46455, 0.744134, 0.216181) {
    block = generator.nextBlock();
    aiLastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    clock.restart();
    aiThread = std::thread(&AI::best, &ai, std::ref(aiMovement));
    aiMovement.data.resize(1000);
}

void Game::stop() {
    isRunning = false;
}

bool Game::shouldStop() {
    return !isRunning;
}

bool Game::shouldClose() {
    return !ui.getWindow().isOpen();
}

//FIXME::restart error
void Game::restart() {
    isRunning = true;
    score = 0;
    holdBlock = BlockType::None;
    grid.clear();
    gravity.clear();
    generator.clear();
    block = generator.nextBlock();
}

void Game::draw() {
    ui.clear();
    if (!shouldStop()) {
        ui.drawBlock(block, 200, 25);
        Block transparentBlock = block.getTransparentBlock();
        while (transparentBlock.moveDown(grid));
        ui.drawBlock(transparentBlock, 200, 25);
    }
    ui.drawLevel(gravity.getLevel());
    ui.drawScore(score);
    ui.drawLines(gravity.getLines());
    ui.drawGrid(grid);
    ui.drawHoldBlock(holdBlock);
    ui.drawNextBlocks(generator.seeNextBlocks(ui.getNextCount()));
    ui.drawBackground();
    ui.drawScoreType(lastScoreType, comboCount, backToBack);
    ui.display();
}

void Game::run() {
    while (!shouldClose()) {
        tick();
        draw();
    }
}

void Game::close() {
    ui.getWindow().close();
}

bool Game::isDifficultScore(const ScoreType &scoreType) const {
    if (scoreType == ScoreType::Tetris || scoreType == ScoreType::TSpinMiniSingle ||
        scoreType == ScoreType::TSpinMiniDouble ||
        scoreType == ScoreType::TSpinSingle || scoreType == ScoreType::TSpinDouble ||
        scoreType == ScoreType::TSpinTriple) {
        return true;
    }
    return false;
}

int Game::scoreTypeToInt(ScoreType scoreType) {
    if (scoreType == ScoreType::None) return 0;
    else if (scoreType == ScoreType::Single) return 100;
    else if (scoreType == ScoreType::Double) return 300;
    else if (scoreType == ScoreType::Triple) return 500;
    else if (scoreType == ScoreType::Tetris) return 800;
    else if (scoreType == ScoreType::Combo) return 50;
    else if (scoreType == ScoreType::TSpinMiniNoLines) return 100;
    else if (scoreType == ScoreType::TSpinMiniSingle) return 200;
    else if (scoreType == ScoreType::TSpinMiniDouble) return 400;
    else if (scoreType == ScoreType::TSpinNoLines) return 400;
    else if (scoreType == ScoreType::TSpinSingle) return 800;
    else if (scoreType == ScoreType::TSpinDouble) return 1200;
    else if (scoreType == ScoreType::TSpinTriple) return 1600;
    else if (scoreType == ScoreType::SinglePerfectClear) return 800;
    else if (scoreType == ScoreType::DoublePerfectClear) return 1200;
    else if (scoreType == ScoreType::TriplePerfectClear) return 1600;
    else if (scoreType == ScoreType::TetrisPerfectClear) return 2000;
    else if (scoreType == ScoreType::BackToBackTetrisPerfectClear) return 3200;
    else return 0;
}

std::queue<Movement> Game::simulateMovement(const Block &aiBlock) {
    std::queue<Movement> movements;

    Block temBlock = this->block;
    while (temBlock.getRotation() != aiBlock.getRotation()) {
        movements.push(Movement::Rotate);
        temBlock.rotate(grid);
    }
    auto [aiRow, aiColumn] = aiBlock.getPosition();
    auto [blockRow, blockColumn] = temBlock.getPosition();
    if (aiColumn > blockColumn) {
        for (int i = 0; i < aiColumn - blockColumn; i++) movements.push(Movement::Right);
    } else if (aiColumn < blockColumn) {
        for (int i = 0; i < blockColumn - aiColumn; i++) movements.push(Movement::Left);
    }


    movements.push(Movement::Down);
    return movements;
}

Game::~Game() {
    ai.stop();
    if (aiThread.joinable()) aiThread.join();
    ui.getWindow().close();
}
