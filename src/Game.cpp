//
// Created by amemiya7 on 2024/6/18.
//

#include "Game.h"
#include <toml++/toml.hpp>
#include <cassert>

const int AI_SEE_NEXT = 9;
const uint32_t comboScores[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5};

void Game::sendBlocks() {
    if (isAiActive && block.isNotSent) {
        //TODO::AI
        std::vector<Block> workingPieces{block};
        for (const auto &i: generator.seeNextBlocks(AI_SEE_NEXT)) {
            workingPieces.push_back(i);
        }
        ai.add(workingPieces, grid);
        block.isNotSent = false;
        aiLastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    };
}

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

//TODO::add smooth movement
//FIXME::soft drop error
//TODO::modify ai limitation to PPS instead of MPS
void Game::processEvents() {
    sf::Event event{};
    sf::RenderWindow &window = ui.getWindow();
    if (isLeftMove) leftMoveCount++;
    else leftMoveCount = 0;
    if (isRightMove) rightMoveCount++;
    else rightMoveCount = 0;

    if (!aiMovement.empty()) {
        movements.push(aiMovement.read());
    }
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.scancode == sf::Keyboard::Scan::I) {
                isAiActive = !isAiActive;
                firstBlock = !firstBlock;
            }
            if (event.key.scancode == sf::Keyboard::Scan::Escape) {
                window.close();
            }
            if (isAiActive) return;
            if (event.key.scancode == sf::Keyboard::Scan::U) {
            }
            if (event.key.scancode == sf::Keyboard::Scan::W) { //right rotate
                movements.push(Movement::Rotate);
//                bool state = utility::rotate(grid, block);
//
//                if (isTouchedGround && state) movement++, lockDelayTime = 0;
                //rotate the block
            } else if (event.key.scancode == sf::Keyboard::Scan::A) {
                if (!isLeftMove) movements.push(Movement::Left);
                isLeftMove = true;
                isRightMove = false;
//                HORIZONTAL_MOVEMENT = -1;
//                bool state = utility::moveLeft(grid, block);
//                if (isTouchedGround && state) movement++, lockDelayTime = 0;
            } else if (event.key.scancode == sf::Keyboard::Scan::D) {
                if (!isRightMove) movements.push(Movement::Right);
                isRightMove = true;
                isLeftMove = false;
//                HORIZONTAL_MOVEMENT = 1;
//                bool state = utility::moveRight(grid,block);
//                if (isTouchedGround && state) movement++, lockDelayTime = 0;
            } else if (event.key.scancode == sf::Keyboard::Scan::S) {
                movements.push(Movement::softDrop);
//                gravity.setSoftDrop();
            } else if (event.key.scancode == sf::Keyboard::Scan::Space) {
                movements.push(Movement::HardDrop);
//                isHardDrop = true;
            } else if (event.key.scancode == sf::Keyboard::Scan::Q) {
                movements.push(Movement::RotateCounterClockwise);
//                utility::rotateCounterClockwise(grid, block);
            } else if (event.key.scancode == sf::Keyboard::Scan::H) {
                movements.push(Movement::Hold);
//                hold();
            } else if (event.key.scancode == sf::Keyboard::Scan::P) {
                restart();
            } else if (event.key.scancode == sf::Keyboard::Scan::M) {
                block = generator.nextBlock();
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (isAiActive) return;
            if (event.key.scancode == sf::Keyboard::Scan::S) {
                gravity.unsetSoftDrop();
            } else if (event.key.scancode == sf::Keyboard::Scan::A) {
                isLeftMove = false;
            } else if (event.key.scancode == sf::Keyboard::Scan::D) {
                isRightMove = false;
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (isAiActive) return;
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
    if (!isAiActive && leftMoveCount >= 5 && isLeftMove) {
        movements.push(Movement::Left);
    }
    if (!isAiActive && rightMoveCount >= 5 && isRightMove) {
        movements.push(Movement::Right);
    }
//    if (isAiActive) {
    if (!movements.empty()) {
        auto movement = movements.front();
        movements.pop();
//    }
//    for (auto movement: movements) {
        if (movement == Movement::Hold) {
            grid.hold(block);
            if (block.empty()) {
                block = generator.nextBlock();
//            continue;
            }
        } else if (movement == Movement::HardDrop) {
            isHardDrop = true;
        } else if (movement == Movement::softDrop) {
            gravity.setSoftDrop();
        } else if (movement == Movement::softDropEnd) {
            gravity.unsetSoftDrop();
        } else {
            bool state = utility::move(grid, block, movement);
            if (isTouchedGround && state) lockDelayTime = 0;
        }
    }
//    }
}

void Game::tick() {
    logicFrameCount++;
    isHardDrop = false;
    processEvents();
    if (!isRunning) return;
    double passedTime = clock.restart().asSeconds();
    time += passedTime;
    while (time >= gravity.getFallTime()) {
        utility::moveDown(grid, block);
        if (gravity.softDrop()) {
            time = 0;
        } else {
            time -= gravity.getFallTime();
        }
    }

    Block transparentBlock = block.getTransparentBlock();
    while (utility::moveDown(grid, transparentBlock));
    sendBlocks();
    //TODO::integrate AI movements into Events

    if (isHardDrop) { while (utility::moveDown(grid, block)); }

    if (utility::touch(grid, block)) {
        sendMessage(Message::BlockTouchGround);
        if (isLockDelay && !isHardDrop) {
            if (isTouchedGround) {
                lockDelayTime += passedTime;
                if (lockDelayTime >= 0.5 || movementCount >= 10) {
                    checkQueue.push(block);
                    utility::insertBlock(grid, block);
                    lastScoreType = addScore();
//                    std::cout << utility::getScoreType(grid, block) << '\n';
                    grid.holdable = true;
                    if (grid.exceed()) {
                        stop();
                    } else {
                        block = generator.nextBlock();
                    }
                    gravity.unsetSoftDrop();
                    lockDelayTime = movementCount = 0;

                }
            } else {
                lockDelayTime = 0;
                isTouchedGround = true;
            }
        } else {
            utility::insertBlock(grid, block);
            checkQueue.push(block);
            lastScoreType = addScore();
            grid.holdable = true;
            gravity.unsetSoftDrop();
            if (grid.exceed()) {
                stop();
            } else {
                block = generator.nextBlock();
            }
        }
    } else {
        isTouchedGround = false;
        movementCount = 0;
    }


}

void Game::hold() {
    if (!grid.holdable) return;
    grid.holdable = false;
    if (grid.holdBlock.empty()) {
        grid.holdBlock = Block(block.getType());
        block = generator.nextBlock();
        return;
    }
    BlockType temType = grid.holdBlock.getType();
    grid.holdBlock = Block(block.getType());
    block = Block(temType);

}

std::pair<int, int> Game::mousePositionToGridPosition(float x, float y) {
    return {(int) (x - 200) / 31, (int) (675 - y - 25) / 31,};
}

ScoreType Game::addScore() {
    ScoreType scoreType = utility::getScoreType(grid, block);
    gravity.addLines(grid.lines());
    score += utility::getScore(grid, block);
    comboCount = grid.comboCount;
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
//386 pc
               ai(626, 700, 736, 148, 455, 338, 200, 452, 100000, 379, 10000, 8000, 100, 725, 1, 1000, 5000, 1000, 5000,
                  1, 1, 1, 1) {
    block = generator.nextBlock();
    aiLastMoveTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    clock.restart();
    aiMovement.data.resize(1000);
    aiMessage.data.resize(1000);
    //TODO::add thread pool
    parseConfig();
    aiThread = std::thread([&] {
        ai.best(aiMovement, aiMessage, checkQueue, bestBlock);
    });


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

void Game::restart() {
    isRunning = true;
    score = 0;
    grid.clear();
    gravity.clear();
    generator.clear();
    block = generator.nextBlock();
    firstBlock = false;
    backToBack = false;
    comboCount = 0;
    clock.restart();
    aiMovement.clear();
    aiMessage.clear();
    while (!movements.empty()) movements.pop();
}

void Game::draw() {
    renderFrameCount++;
    ui.clear();
    if (!shouldStop()) {
        ui.drawBlock(block, 200, 25);
//        if (!bestBlock.empty()) ui.drawBlock(bestBlock, 200, 25);
        Block transparentBlock = block.getTransparentBlock();
        while (utility::moveDown(grid, transparentBlock));
        ui.drawBlock(transparentBlock, 200, 25);
    }
    ui.drawLevel(gravity.getLevel());
    ui.drawScore(score);
    ui.drawLines(gravity.getLines());
    ui.drawGrid(grid);
    ui.drawHoldBlock(grid.holdBlock);
    ui.drawNextBlocks(generator.seeNextBlocks(ui.getNextCount()));
    ui.drawBackground();
    ui.drawScoreType(lastScoreType, comboCount, backToBack);
    ui.display();
}

void Game::run() {
    auto lastLogicTime = std::chrono::high_resolution_clock::now();
    auto lastRenderTime = lastLogicTime;
    auto currentLogicTime = lastLogicTime;
    auto currentRenderTime = lastLogicTime;
    double logicDuration = 0, renderDuration = 0;
    while (!shouldClose()) {
        currentLogicTime = std::chrono::high_resolution_clock::now();
        currentRenderTime = currentLogicTime;
        logicDuration += std::chrono::duration<double, std::milli>(currentLogicTime - lastLogicTime).count();
        renderDuration += std::chrono::duration<double, std::milli>(currentRenderTime - lastRenderTime).count();
        lastLogicTime = currentLogicTime;
        lastRenderTime = currentRenderTime;

        while (logicDuration >= 1000.0 / MAX_LOGIC_FRAMES) {
            tick();
            logicDuration -= 1000.0 / MAX_LOGIC_FRAMES;
        }
        while (renderDuration >= 1000.0 / MAX_RENDER_FRAMES) {
            draw();
            renderDuration -= 1000.0 / MAX_RENDER_FRAMES;
        }
        if (logicDuration > 1.0 && renderDuration > 1.0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


bool Game::isDifficultScore(const ScoreType &scoreType) {
    //TODO::read from config file
    if (scoreType == ScoreType::Tetris || scoreType == ScoreType::TSpinMiniSingle ||
        scoreType == ScoreType::TSpinMiniDouble ||
        scoreType == ScoreType::TSpinSingle || scoreType == ScoreType::TSpinDouble ||
        scoreType == ScoreType::TSpinTriple || scoreType == ScoreType::BackToBackTetrisPerfectClear ||
        scoreType == ScoreType::DoublePerfectClear || scoreType == ScoreType::TriplePerfectClear ||
        scoreType == ScoreType::TetrisPerfectClear || scoreType == ScoreType::SinglePerfectClear) {
        return true;
    }
    return false;
}


Game::~Game() {
    ai.stop();
    if (aiThread.joinable()) aiThread.join();
    ui.getWindow().close();
}

void Game::parseConfig() {
    auto config = toml::parse_file((projectPath / "config.toml").c_str());
    MAX_LOGIC_FRAMES = config["game"]["MAX_LOGIC_FRAMES"].value_or<uint32_t>(20);
    MAX_RENDER_FRAMES = config["game"]["MAX_RENDER_FRAMES"].value_or<uint32_t>(60);
    MAX_AI_MOVEMENTS_PER_SECOND = config["ai"]["MAX_AI_MOVEMENTS_PER_SECOND"].value_or<uint32_t>(100);
    NO_GRAVITY = config["game"]["NO_GRAVITY"].value_or<bool>(false);
    uint32_t PPS = config["ai"]["PPS"].value_or<uint32_t>(1);
    ai.PPS = PPS;
    if (NO_GRAVITY) gravity.setNoGravity();
}

void Game::sendMessage(Message message) {
    aiMessage.write(message);
}