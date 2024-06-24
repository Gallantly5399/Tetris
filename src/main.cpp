#include <iostream>

#include "Block.h"
#include "Grid.h"
#include "Game.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <random>
#include "Gravity.h"
#include <filesystem>
#include "Generator.h"

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

const unsigned int SCREEN_WIDTH = 1200, SCREEN_HEIGHT = 675;

//glm::vec2 worldToScreen(unsigned int x, unsigned int y) {
//    return glm::vec2((1.0f * x / SCREEN_WIDTH - 0.5) * 2, ((1 - 1.0f * y /SCREEN_HEIGHT) -0.5)  * 2);
//}


void
drawGrid(const Grid &grid, sf::RenderWindow &window, int blockWidth, int stripeWidth, int startPosX, int startPosY) {
    for (int column = 0; column < grid.getWidth(); column++) {
        for (int row = 0; row < grid.getHeight() - 1; row++) {
            if (grid.isOccupied(row, column)) {
//                std::cout << "Draw row:" << row << " and column:" << column << "\n";
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = grid.getScreenPosition(row, column, blockWidth, stripeWidth, SCREEN_WIDTH,
                                                           SCREEN_HEIGHT, startPosX,
                                                           startPosY);
                rectangle.setPosition(posX, posY);
//                std::cout << "posX: " << posX << ", posY: " << posY << std::endl;
                rectangle.setFillColor(grid.getColor(row, column));
                window.draw(rectangle);
            }
        }
    }
}

void
drawBlock(const Block &block, sf::RenderWindow &window, int blockWidth, int stripeWidth, int startPosX, int startPosY) {
    if (block.empty()) return;
    const auto &shape = block.getShape();
    const auto &color = block.getColor();
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape[column].size(); row++) {
            if (shape[column][row] == 1) {
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = block.getScreenPosition(row, column, blockWidth, stripeWidth, SCREEN_WIDTH,
                                                            SCREEN_HEIGHT, startPosX,
                                                            startPosY);
                rectangle.setPosition(posX, posY);
                rectangle.setFillColor(color);
                window.draw(rectangle);
            }
        }
    }
}

//insert the block into the grid
void insertBlock(Grid &grid, const Block &block) {
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


void
drawWindowBackground(sf::RenderWindow &window, int startPosX, int startPosY, int width, int height, int windowWidth,
                     int windowHeight, bool reverseY = true) {
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

void drawHoldBlock(sf::RenderWindow &window, const Block &block, int startPosX, int startPosY, int blockWidth,
                   int stripeWidth) {
    int currentPosX = startPosX, currentPosY = startPosY;
    Block temBlock = block;
    temBlock.setStartRow(0);
    temBlock.setStartColumn(0);

    if (temBlock.getType() == BlockType::I) {
        currentPosX -= 45;
    } else if (temBlock.getType() == BlockType::O) {
        currentPosX -= 15;
    }

    drawBlock(temBlock, window, blockWidth, stripeWidth, currentPosX, currentPosY);
}

void drawNextBlocks(sf::RenderWindow &window, const std::vector<Block> &nextBlocks, int startPosX, int startPosY,
                    int blockWidth, int stripeWidth) {
    int posX = 550, posY = 625 - 91;
    for (int i = 0; i < nextBlocks.size(); i++) {
        int currentPosX = posX, currentPosY = posY - i * 92;
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
    //TODO::add restart
//    Gravity gravity(1);
//    gravity.setNoGravity();
    unsigned long long score = 0;
    unsigned long long totalClearedLines = 0;
    unsigned long long linesToUpdate = 0;
    const int nextCount = 5;
//    Generator generator{};
    const int blockWidth = 30, stripeWidth = 1;
//    Grid grid(10, 22);

//    Block block = generator.nextBlock();

//    window.setFramerateLimit(60);
    const int startPosX = 200, startPosY = 25;
//    sf::Clock clock;
//    double time = 0;

    //FIXME::block movement is not smooth
//    window.setKeyRepeatEnabled(false);

    //TODO:: T-spin
    //TODO:: block.moveDown() optimization
//    sf::Clock lockDelayClock;
//    double lockDelayTime = 0;
//    bool isTouchedGround = false;
//    int movement = 0;
//    bool isLockDelay = true;
    sf::Font font;
    std::filesystem::path fontsPath = std::filesystem::path(FILE_LOCATION) / "resources" / "fonts";
    if (!font.loadFromFile((fontsPath / "scoreFont.ttf").string())) {
        std::cerr << "Failed to load font\n";
        return 1;
    }
    //text level
    sf::Text textLevel;
    textLevel.setFont(font);
    textLevel.setString("Level: 0");
    textLevel.setCharacterSize(24);
    textLevel.setPosition(700, 25);
//
//    sf::Text textLevelNumber;
//    textLevelNumber.setFont(font);
//    textLevelNumber.setCharacterSize(24);
//    textLevelNumber.setPosition(730, 50);
    sf::Text textScore;
    textScore.setFont(font);
    textScore.setString("Score: 0");
    textScore.setCharacterSize(24);
    textScore.setPosition(700, 75);

    sf::Text textLines;
    textLines.setFont(font);
    textLines.setString("Lines: 0");
    textLines.setCharacterSize(24);
    textLines.setPosition(700, 125);
//    sf::Text textScoreNumber;
//    textScoreNumber.setFont(font);
//    textScoreNumber.setCharacterSize(24);
//    textScoreNumber.setPosition(730, 120);
    Game game{};
    sf::RenderWindow &window = game.getWindow();
    while (!game.shouldClose()) {
        game.tick();
        textLevel.setString("Level: " + std::to_string(game.getGravity().getLevel()));
        textScore.setString("Score: " + std::to_string(game.getScore()));
        textLines.setString("Lines: " + std::to_string(game.getGravity().getLines()));
        window.clear(sf::Color::Black);
        if (!game.shouldStop()) {
            drawBlock(game.getBlock(), window, blockWidth, stripeWidth, startPosX, startPosY);
            Block transparentBlock = game.getBlock().getTransparentBlock();
            while (transparentBlock.moveDown(game.getGrid()));
            drawBlock(transparentBlock, window, blockWidth, stripeWidth, startPosX, startPosY);
        }
        drawGrid(game.getGrid(), window, blockWidth, stripeWidth, startPosX, startPosY);
        //TODO:: make magic number to constant
        //draw the main window
        //310 670
        //main window
        drawWindowBackground(window, startPosX, startPosY, 310, startPosY + 594, SCREEN_WIDTH, SCREEN_HEIGHT);
        int nextWidth = 155, nextHeight = nextCount * 92;
        //startPos(525, 640 - nextHeight)
        //next window
        drawWindowBackground(window, 525, 644 - nextHeight, nextWidth, nextHeight, SCREEN_WIDTH, SCREEN_HEIGHT);
        drawNextBlocks(window, game.getGenerator().seeNextBlocks(nextCount), 550, 625, blockWidth, stripeWidth);

        //hold window
        drawWindowBackground(window, 35, startPosY + 594 - 92 + 25, nextWidth, 92, SCREEN_WIDTH, SCREEN_HEIGHT);
        drawHoldBlock(window, game.getHoldBlock(), 60, startPosY + 594 - 92, blockWidth, stripeWidth);
        window.draw(textLevel);
        window.draw(textScore);
        window.draw(textLines);
        window.display();
    }
}

