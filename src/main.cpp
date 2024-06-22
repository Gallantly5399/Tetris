#include <iostream>

#include "Block.h"
#include "Grid.h"
#include "Game.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include "Gravity.h"
#include <filesystem>

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
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape[column].size(); row++) {
            if (shape[column][row] == 1) {
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = block.getScreenPosition(row, column, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX,
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
    for (int column = 0; column < shape.size(); column ++) {
        for (int row = 0; row < shape.size(); row ++) {
            if (shape[column][row] == 1) {
                grid.fill(startRow + row, startColumn + column, color);
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
    sf::Vertex lineTop[] = {leftTop, rightTop};

    window.draw(lineBottom, 2, sf::Lines);
    window.draw(lineLeft, 2, sf::Lines);
    window.draw(lineRight, 2, sf::Lines);
    window.draw(lineTop, 2, sf::Lines);
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
    Gravity gravity(1);
//    gravity.setNoGravity();
    unsigned long long score = 0;
    unsigned long long totalClearedLines = 0;
    unsigned long long linesToUpdate = 0;
    const int nextCount = 5;
    std::array<int, 5> scores = {0, 1, 3, 5, 8};
    Generator generator{};
    const int blockWidth = 30, stripeWidth = 1;
    Grid grid(10, 22);

    Block block = generator.nextBlock();
    sf::RenderWindow window(sf::VideoMode(1200, 675), "Tetris");
//    window.setFramerateLimit(60);
    const int startPosX = 200, startPosY = 25;
    auto [px, py] = block.getScreenPosition(0, 0, blockWidth, stripeWidth, SCREEN_WIDTH, SCREEN_HEIGHT, startPosX, startPosY);
//    std::cout << "px: "<< px << ", py: " << py << std::endl;
//    std::cout << "StartRow: " << block.getStartRow() << ", StartColumn: " << block.getStartColumn() << std::endl;
    sf::Clock clock;
    double time = 0;

    //FIXME::block movement is not smooth
//    window.setKeyRepeatEnabled(false);

    /*TODO::Every movement will update the lockDelay status
     * and the maximum movement is 10
     */
    //TODO:: add harddrop
    //TODO:: T-spin
    //TODO:: add counter-clockwise rotation
    //TODO:: block.moveDown() optimization
    sf::Clock lockDelayClock;
    double lockDelayTime = 0;
    bool isTouchedGround = false;
    int movement = 0;
    bool isLockDelay = true;
    sf::Font font;
    std::filesystem::path fontsPath = std::filesystem::path(FILE_LOCATION) / "resources" / "fonts";
    if (!font.loadFromFile((fontsPath / "scoreFont.ttf").string())) {
        std::cerr << "Failed to load font\n";
        return 1;
    }
    //text level
    sf::Text textLevel;
    textLevel.setFont(font);
    textLevel.setString("Level");
    textLevel.setCharacterSize(24);
    textLevel.setPosition(700, 25);

    sf::Text textLevelNumber;
    textLevelNumber.setFont(font);
    textLevelNumber.setCharacterSize(24);
    //1
    textLevelNumber.setPosition(730, 50);
    //10
//    textLevelNumber.setPosition(715, 50);
    sf::Text textScore;
    textScore.setFont(font);
    textScore.setString("Score");
    textScore.setCharacterSize(24);
    textScore.setPosition(700, 100);

    sf::Text textScoreNumber;
    textScoreNumber.setFont(font);
    textScoreNumber.setCharacterSize(24);
    textScoreNumber.setPosition(730, 120);

    while (window.isOpen()) {
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
                        insertBlock(grid, block);
                        block = generator.nextBlock();
                        gravity.unsetSoftDrop();
                    }
                } else {
                    lockDelayTime = 0;
                    isTouchedGround = true;
                }
            }
            else {
                insertBlock(grid, block);
                block = generator.nextBlock();
                gravity.unsetSoftDrop();
            }
        } else {
            isTouchedGround = false;
            movement = 0;
        }
        if (grid.exceed()) {
            //TODO:: game over
            window.close();
        }
        //TODO add level
        int clearedLines = grid.clearLines();
        gravity.addLines(clearedLines);
        score += scores[clearedLines] * gravity.getLevel();

        //Text Level and Score
        textLevelNumber.setString(std::to_string(gravity.getLevel()));
        if (gravity.getLevel() >= 10) textLevelNumber.setPosition(715, 50);
        else textLevelNumber.setPosition(730, 50);
        if (score >= 1000) textScoreNumber.setPosition(700, 120);
        else if (score >= 100) textScoreNumber.setPosition(710, 120);
        else if(score >= 10) textScoreNumber.setPosition(720, 120);
        else textScoreNumber.setPosition(730, 120);
        textScoreNumber.setString(std::to_string(score));

        window.clear(sf::Color::Black);
        drawBlock(block, window, blockWidth, stripeWidth, startPosX, startPosY);

        drawBlock(transparentBlock, window, blockWidth, stripeWidth, startPosX, startPosY);

        drawGrid(grid, window, blockWidth, stripeWidth, startPosX, startPosY);
        //TODO:: make magic number to constant
        //draw the main window
        //310 670
        drawMainWindowBackground(window, startPosX, startPosY, 310, startPosY + 594, SCREEN_WIDTH, SCREEN_HEIGHT);
        int nextWidth = 155, nextHeight = nextCount * 92;
        //startPos(525, 640 - nextHeight)
        drawNextWindowBackground(window, 525, 644 - nextHeight, nextWidth, nextHeight, SCREEN_WIDTH, SCREEN_HEIGHT);
        drawNextBlocks(window, generator.seeNextBlocks(nextCount), 550, 625, blockWidth, stripeWidth);
        window.draw(textLevel);
        window.draw(textLevelNumber);
        window.draw(textScore);
        window.draw(textScoreNumber);
        window.display();
    }
}

