#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <iostream>

#include "Block.h"
#include "Grid.h"
#include "Game.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <random>

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

class Generator {
public:
    Generator() {
        std::random_device rd;
        gen = std::mt19937(rd());
        std::shuffle(nextBlocks.begin(), nextBlocks.end(), gen);
    }
    Block nextBlock() {
        std::cout << index << std::endl;
        if (index == nextBlocks.size()) {
            index = 0;
            std::shuffle(nextBlocks.begin(), nextBlocks.end(), gen);
        }
        return Block(static_cast<BlockType>(nextBlocks[index++ % 7]));
    }
    int operator()() {
        return gen();
    }
private:
    std::mt19937 gen;
    int index = 0;
    std::array<int, 7> nextBlocks = {0, 1, 2, 3, 4, 5, 6};
};

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

class Gravity {
public:

private:
    int level = 1;
    int fallTime = 0;
};
int main() {
    unsigned long long score = 0;
    std::array<int, 5> scores = {0, 100, 300, 500, 800};
    Generator generator{};
    const int blockWidth = 30, stripeWidth = 1;
    Block block = generator.nextBlock();
    Grid grid(10, 22);
//    grid.fill(0, 0, sf::Color::Blue);
//    grid.fill(0, 1, sf::Color::Blue);
//    grid.fill(0, 2, sf::Color::Blue);
//    grid.fill(1, 1, sf::Color::Blue);

    sf::RenderWindow window(sf::VideoMode(1200, 675), "Tetris");
//    window.setFramerateLimit(60);
    const int startPosX = 200, startPosY = 25;
    auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
    std::cout << "px: "<< px << ", py: " << py << std::endl;
    std::cout << "StartRow: " << block.getStartRow() << ", StartColumn: " << block.getStartColumn() << std::endl;
    window.setFramerateLimit(60);
    while (window.isOpen()) {
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
                    bool state = block.moveLeft(grid);
                    if (state) {
                        std::cout << "move to left\n";
                    }
                    //move the block to the left
                } else if (event.key.scancode == sf::Keyboard::Scan::D) {
#ifndef NDEBUG
                    std::cout << "pressed D\n";
#endif
                    bool state = block.moveRight(grid);
                    if (state) {
                        std::cout << "move to Right\n";
                    }
                    //move the block to the right
                } else if (event.key.scancode == sf::Keyboard::Scan::S) {
#ifndef NDEBUG
                    std::cout << "pressed S\n";
#endif
                    bool state = block.moveDown(grid);
                    if (state) {
                        std::cout << "move Down\n";
                        std::cout << "StartRow: " << block.getStartRow() << ", StartColumn: " << block.getStartColumn() << std::endl;
                        auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
                        std::cout << "Screen PosX:" << px << ", Screen PosY:" << py << std::endl;
                    }
                    //move the block down
                    //add the gravity level
                }
            }
        }

        //TODO:: add time delay 0.5s or 30frames
        if (block.touch(grid)) {
            insertBlock(grid, block);
            block = generator.nextBlock();
        }
        if (grid.exceed()) {
            //TODO:: game over
            window.close();
        }
        //TODO add level
        score += scores[grid.clearLines()];
        window.clear(sf::Color::Black);
        drawBlock(block, window, blockWidth, stripeWidth, startPosX, startPosY);
        drawGrid(grid, window, blockWidth, stripeWidth, startPosX, startPosY);
        drawMainWindowBackground(window, startPosX, startPosY, 310, 670, SCREEN_WIDTH, SCREEN_HEIGHT);
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

