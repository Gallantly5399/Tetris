#include <iostream>

#include "Block.h"
#include "Grid.h"
#include "Game.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include "Gravity.h"

//SFML coordinate system
// -----------x
// |
// |
// |
// |
// y

//My coordinate system
// y
// |
// |
// |
// |
// -----------x

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

const unsigned int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 675;

//glm::vec2 worldToScreen(unsigned int x, unsigned int y) {
//    return glm::vec2((1.0f * x / SCREEN_WIDTH - 0.5) * 2, ((1 - 1.0f * y /SCREEN_HEIGHT) -0.5)  * 2);
//}

void output(glm::vec2 pos) {
    std::cout << pos.x << ' ' << pos.y << std::endl;
}

void drawGrid(const Grid& grid, sf::RenderWindow &window, int blockWidth, int stripeWidth, int startPosX, int startPosY) {
    for (int column = 0; column < grid.getWidth(); column++) {
        for (int row = 0; row < grid.getHeight() - 1; row++) {
            if (grid.isOccupied(row, column)) {
//                std::cout << "Draw row:" << row << " and column:" << column << "\n";
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = grid.getScreenPosition(row, column, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX,
                                                           startPosY);
                rectangle.setPosition(posX, posY);
//                std::cout << "posX: " << posX << ", posY: " << posY << std::endl;
                rectangle.setFillColor(grid.getColor(row, column));
                window.draw(rectangle);
            }
        }
    }
}

void drawBlock(const Block& block, sf::RenderWindow& window, int blockWidth, int stripeWidth, int startPosX, int startPosY) {
    const auto& shape = block.getShape();
    const auto& color = block.getColor();
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            if (shape[i][j] == 1) {
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = block.getScreenPosition(i, j, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX,
                                                           startPosY);
                rectangle.setPosition(posX, posY);
                rectangle.setFillColor(color);
                window.draw(rectangle);
            }
        }
    }
}

//generate the next block
class Generator {
public:
    Generator() {
        std::random_device rd;
        gen = std::mt19937(rd());
        std::shuffle(nextBlocks.begin(), nextBlocks.begin() + 7, gen);
        std::shuffle(nextBlocks.begin() + 7, nextBlocks.begin() + 14, gen);
    }
    Block nextBlock() {
#ifndef NDEBUG
        std::cout << "Current Next Block Index: " << index << std::endl;
#endif
        if (index == 7) {
            std::shuffle(nextBlocks.begin(), nextBlocks.begin() + 7, gen);
        }
        if (index == nextBlocks.size()) {
            index = 0;
            std::shuffle(nextBlocks.begin() + 7, nextBlocks.begin() + 14, gen);
        }
        return Block(static_cast<BlockType>(nextBlocks[index++ % 14]));
    }
    std::vector<Block> seeNextBlocks(int count) {
        std::vector<Block> blocks;
        for (int i = 0; i < count; i++) {
            blocks.push_back(Block(static_cast<BlockType>(nextBlocks[(index + i) % 14])));
        }
        return blocks;
    }
    int operator()() {
        return gen();
    }
private:
    std::mt19937 gen;
    int index = 0;
    std::array<int, 14> nextBlocks = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6};
};

//insert the block into the grid
void insertBlock(Grid& grid, const Block& block) {
    const auto& shape = block.getShape();
    const auto& color = block.getColor();
    int startRow = block.getStartRow();
    int startColumn = block.getStartColumn();
    for (int i = 0; i < shape.size(); i++) {
        for (int j = 0; j < shape[i].size(); j++) {
            if (shape[i][j] == 1) {
                grid.fill(startRow + i, startColumn + j, color);
            }
        }
    }
}


void drawMainWindowBackground(sf::RenderWindow& window, int startPosX, int startPosY, int width, int height, int windowWidth, int windowHeight, bool reverseY = true) {
    //draw 3 lines
    sf::Vector2f leftBottom(startPosX, startPosY);
    sf::Vector2f leftTop(startPosX, startPosY + height);
    sf::Vector2f rightBottom(startPosX + width, startPosY);
    sf::Vector2f rightTop(startPosX + width, startPosY + height);
    if (reverseY) {
        leftBottom.y = windowHeight - leftBottom.y;
        leftTop.y = windowHeight - leftTop.y;
        rightBottom.y = windowHeight - rightBottom.y;
        rightTop.y = windowHeight - rightTop.y;
    }
    sf::Vertex lineBottom[] = {leftBottom, rightBottom};
    sf::Vertex lineLeft[] = {leftBottom, leftTop};
    sf::Vertex lineRight[] = {rightBottom, rightTop};

    window.draw(lineBottom, 2, sf::Lines);
    window.draw(lineLeft, 2, sf::Lines);
    window.draw(lineRight, 2, sf::Lines);
}

void drawNextWindowBackground(sf::RenderWindow& window, int startPosX, int startPosY, int width, int height, int windowWidth, int windowHeight, bool reverseY = true) {
    sf::Vector2f leftBottom(startPosX, startPosY);
    sf::Vector2f leftTop(startPosX, startPosY + height);
    sf::Vector2f rightBottom(startPosX + width, startPosY);
    sf::Vector2f rightTop(startPosX + width, startPosY + height);
    if (reverseY) {
        leftBottom.y = windowHeight - leftBottom.y;
        leftTop.y = windowHeight - leftTop.y;
        rightBottom.y = windowHeight - rightBottom.y;
        rightTop.y = windowHeight - rightTop.y;
    }
    sf::Vertex lineBottom[] = {leftBottom, rightBottom};
    sf::Vertex lineLeft[] = {leftBottom, leftTop};
    sf::Vertex lineRight[] = {rightBottom, rightTop};
    sf::Vertex lineTop[] = {leftTop, rightTop};
    window.draw(lineBottom, 2, sf::Lines);
    window.draw(lineLeft, 2, sf::Lines);
    window.draw(lineRight, 2, sf::Lines);
    window.draw(lineTop, 2, sf::Lines);
}

void drawNextBlocks(sf::RenderWindow& window, const std::vector<Block>& nextBlocks, int startPosX, int startPosY, int blockWidth, int stripeWidth) {
    int posX = 550, posY = 625 - 91;
    for (int i = 0; i < nextBlocks.size(); i++) {
        int currentPosX = 550, currentPosY = posY - i * 92;
        Block temBlock = nextBlocks[i];
        temBlock.setStartRow(0);
        temBlock.setStartColumn(0);

        if (temBlock.getType() == BlockType::I) {
           currentPosX -= 45;
        } else if (temBlock.getType() == BlockType::O) {
            currentPosX -= 15;
        }

        drawBlock(temBlock, window, blockWidth, stripeWidth, currentPosX, currentPosY);
    }
}
int main() {
    Gravity gravity(10);
    unsigned long long score = 0;
    unsigned long long level = 1;
    unsigned long long totalClearedLines = 0;
    unsigned long long linesToUpdate = 0;
    const int nextCount = 5;
    std::array<int, 5> scores = {0, 100, 300, 500, 800};
    Generator generator{};
    const int blockWidth = 30, stripeWidth = 1;
    Grid grid(10, 22);

    Block block = generator.nextBlock();
    sf::RenderWindow window(sf::VideoMode(1200, 675), "Tetris");
    window.setFramerateLimit(60);
    const int startPosX = 200, startPosY = 25;
    auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
    std::cout << "px: "<< px << ", py: " << py << std::endl;
    std::cout << "StartRow: " << block.getStartRow() << ", StartColumn: " << block.getStartColumn() << std::endl;
    sf::Clock clock;
    double time = 0;

    //FIXME::block movement is not smooth
//    window.setKeyRepeatEnabled(false);

    /*TODO::Every movement will update the lockDelay status
     * and the maximum movement is 10
     */
    sf::Clock lockDelayClock;
    double lockDelayTime = 0;
    bool isTouchedGround = false;
    bool isLockDelay = true;
    while (window.isOpen()) {
//        std::cout << gravity.getFallTime() << std::endl;
        double passedTime = clock.restart().asSeconds();
        time += passedTime;
        if (time >= gravity.getFallTime()) {
            std::cout << "Time1: " << time << std::endl;
            block.moveDown(grid);
            time = 0;
        }
        //timer
        //key events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.scancode == sf::Keyboard::Scan::Escape) {
#ifndef NDEBUG
                    std::cout << "pressed escape\n";
#endif
                    window.close();
                } else if (event.key.scancode == sf::Keyboard::Scan::W) {
#ifndef NDEBUG
                    std::cout << "pressed W\n";
#endif
                    bool state = block.rotate(grid);
#ifndef NDEBUG
                    if (state) {
                        std::cout << "rotated\n";
                        std::cout << "StartRow: " << block.getStartRow() << ", StartColumn: " << block.getStartColumn() << std::endl;
                        auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
                        std::cout << "Screen PosX:" << px << ", Screen PosY:" << py << std::endl;
                    }
#endif
                    //rotate the block
                } else if (event.key.scancode == sf::Keyboard::Scan::A) {
#ifndef NDEBUG
                    std::cout << "pressed A\n";
#endif
//                    bool state = block.moveLeft(grid);
//                    if (state) {
//                        std::cout << "move to left\n";
//                    }
                    //move the block to the left
                } else if (event.key.scancode == sf::Keyboard::Scan::D) {
#ifndef NDEBUG
                    std::cout << "pressed D\n";
#endif
//                    block.moveRight(grid);
//                    bool state = block.moveRight(grid);
//                    if (state) {
//                        std::cout << "move to Right\n";
//                    }
                    //move the block to the right
                } else if (event.key.scancode == sf::Keyboard::Scan::S) {
                    gravity.setSoftDrop();
                    //move the block down
                    //add the gravity level
                }
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.scancode == sf::Keyboard::Scan::S) {
                    gravity.reset();
                }
            }
        }
        //TODO:: add lock delay 0.5s and maximum movement of 10
        if (block.touch(grid)) {
            if (isLockDelay) {
                if (isTouchedGround) {
                    lockDelayTime += passedTime;
#ifndef NDEBUG
                    std::cout << "Lock Delay Time:" << time << std::endl;
#endif
                    if (lockDelayTime >= 0.5) {
#ifndef NDEBUG
                        std::cout << "Lock Delay Time:" << time << std::endl;
#endif
                        insertBlock(grid, block);
                        block = generator.nextBlock();
                        gravity.reset();
                    }
                } else {
                    lockDelayTime = 0;
                    isTouchedGround = true;
                }
            }
            else {
                insertBlock(grid, block);
                block = generator.nextBlock();
                gravity.reset();
            }
        } else {
#ifndef NDEBUG
            std::cout << "Is Not Touching the Groud" << std::endl;
#endif
            isTouchedGround = false;
        }
        if (grid.exceed()) {
            //TODO:: game over
            window.close();
        }

        //TODO add level
        int clearedLines = grid.clearLines();
        totalClearedLines += clearedLines;
        linesToUpdate += clearedLines;
        score += scores[clearedLines] * level;
        if (linesToUpdate >= 10 * level) {
            linesToUpdate -= 10 * level;
            gravity.levelUp();
            level++;
        }

        window.clear(sf::Color::Black);
        drawBlock(block, window, blockWidth, stripeWidth, startPosX, startPosY);
        drawGrid(grid, window, blockWidth, stripeWidth, startPosX, startPosY);
        //TODO:: make magic number to constant
        drawMainWindowBackground(window, startPosX, startPosY, 310, 670, SCREEN_WIDTH, SCREEN_HEIGHT);
        int nextWidth = 155, nextHeight = nextCount * 92;
        drawNextWindowBackground(window, 525, 640 - nextHeight, nextWidth, nextHeight, SCREEN_WIDTH, SCREEN_HEIGHT);
        drawNextBlocks(window, generator.seeNextBlocks(nextCount), 550, 625, blockWidth, stripeWidth);
#ifndef NDEBUG
//        auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
//        std::cout << "px: "<< px << ", py: " << py << std::endl;
//        std::cout << "StartX: " << block.getStartX() << ", StartY: " << block.getStartY() << std::endl;
#endif

//        for (int i = 0; i < grid.getWidth(); i++) {
//            for (int j = 0; j < grid.getHeight(); j++) {
//                if (grid.isOccupied(i, j)) {
//                    sf::RectangleShape rectangle(sf::Vector2f(30, 30));
//                    auto [posX, posY] = grid.getScreenPosition(i, j, 30, 1, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX,
//                                                               startPosY);
//                    rectangle.setPosition(posX, posY);
//                    rectangle.setFillColor(grid.getColor(i, j));
//                    window.draw(rectangle);
//                }
//            }
//        }
        window.display();
    }
}

