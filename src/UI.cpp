//
// Created by amemiya7 on 2024/6/23.
//

#include "UI.h"
#include <filesystem>
#include <string>

UI::UI() :
window(sf::VideoMode(1200, 675), "Tetris") {
    std::filesystem::path fontsPath = std::filesystem::path(FILE_LOCATION) / "resources" / "fonts";
    font.loadFromFile((fontsPath / "scoreFont.ttf").string());
    textLevel.setFont(font);
    textLevel.setString("Level: 0");
    textLevel.setCharacterSize(24);
    textLevel.setPosition(700, 25);

    textScore.setFont(font);
    textScore.setString("Score: 0");
    textScore.setCharacterSize(24);
    textScore.setPosition(700, 75);

    textLines.setFont(font);
    textLines.setString("Lines: 0");
    textLines.setCharacterSize(24);
    textLines.setPosition(700, 125);

    textScoreType.setFont(font);
    textScoreType.setCharacterSize(20);
    textScoreType.setPosition(700, 275);
}


void UI::clear() {
    window.clear();
}

sf::RenderWindow& UI::getWindow(){
    return window;
}

void UI::drawLevel(int level) {
    textLevel.setString("Level: " + std::to_string(level));
    window.draw(textLevel);
}

void UI::drawScore(int score) {
    textScore.setString("Score: " + std::to_string(score));
    window.draw(textScore);
}

void UI::drawLines(int lines) {
    textLines.setString("Lines: " + std::to_string(lines));
    window.draw(textLines);
}

void UI::display() {
    window.display();
}

void UI::drawNextBlocks(const std::vector<Block> &nextBlocks) {
    int posX = NextBlockStartX, posY = NextBlockStartY;
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

        drawBlock(temBlock, currentPosX, currentPosY);
    }

}

void UI::drawHoldBlock(const Block &block) {
    int currentPosX = HoldBlockStartX, currentPosY = HoldBlockStartY;
    Block temBlock = block;
    temBlock.setStartRow(0);
    temBlock.setStartColumn(0);

    if (temBlock.getType() == BlockType::I) {
        currentPosX -= 45;
    } else if (temBlock.getType() == BlockType::O) {
        currentPosX -= 15;
    }

    drawBlock(temBlock, currentPosX, currentPosY);
}

void UI::drawWindowBackground(int startPosX, int startPosY, int width, int height, bool reverseY) {
    sf::Vector2f leftBottom(startPosX, startPosY);
    sf::Vector2f leftTop(startPosX, startPosY + height);
    sf::Vector2f rightBottom(startPosX + width, startPosY);
    sf::Vector2f rightTop(startPosX + width, startPosY + height);
    if (reverseY) {
        leftBottom.y = SCREEN_HEIGHT - leftBottom.y;
        leftTop.y = SCREEN_HEIGHT - leftTop.y;
        rightBottom.y = SCREEN_HEIGHT - rightBottom.y;
        rightTop.y = SCREEN_HEIGHT - rightTop.y;
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

void UI::drawBlock(const Block &block, int startPosX, int startPosY) {
    if (block.empty()) return;
    const auto &shape = block.getShape();
    const auto &color = block.getColor();
    for (int column = 0; column < shape.size(); column++) {
        for (int row = 0; row < shape[column].size(); row++) {
            if (shape[column][row] == 1) {
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = block.getScreenPosition(row, column, BlockWidth, StripeWidth, SCREEN_WIDTH,
                                                            SCREEN_HEIGHT, startPosX,
                                                            startPosY);
                rectangle.setPosition(posX, posY);
                rectangle.setFillColor(color);
                window.draw(rectangle);
            }
        }
    }
}

void UI::drawGrid(const Grid &grid) {
    for (int column = 0; column < grid.getWidth(); column++) {
        for (int row = 0; row < grid.getHeight() - 1; row++) {
            if (grid.isOccupied(row, column)) {
//                std::cout << "Draw row:" << row << " and column:" << column << "\n";
                sf::RectangleShape rectangle(sf::Vector2f(30, 30));
                auto [posX, posY] = grid.getScreenPosition(row, column, BlockWidth, StripeWidth, SCREEN_WIDTH,
                                                           SCREEN_HEIGHT, GridStartX,
                                                           GridStartY);
                rectangle.setPosition(posX, posY);
//                std::cout << "posX: " << posX << ", posY: " << posY << std::endl;
                rectangle.setFillColor(grid.getColor(row, column));
                window.draw(rectangle);
            }
        }
    }
}

void UI::drawBackground() {
    //main window
    drawWindowBackground(MainWindowStartX, MainWindowStartY, 310, MainWindowStartY + 594);
    //next window
    drawWindowBackground(NextWindowStartX, NextWindowStartY, NextWidth, NextHeight);
    //hold window
    drawWindowBackground(HoldWindowStartX, HoldWindowStartY, HoldWidth, HoldHeight);
}

int UI::getNextCount() const {
    return NextCount;
}

std::string to_string(ScoreType scoreType) {
    if (scoreType == ScoreType::Single) return "Single";
    else if (scoreType == ScoreType::Double) return "Double";
    else if (scoreType == ScoreType::Triple) return "Triple";
    else if (scoreType == ScoreType::Tetris) return "Tetris";
    else if (scoreType == ScoreType::TSpinMiniNoLines) return "TSpinMiniNoLines";
    else if (scoreType == ScoreType::TSpinMiniSingle) return "TSpinMiniSingle";
    else if (scoreType == ScoreType::TSpinMiniDouble) return "TSpinMiniDouble";
    else if (scoreType == ScoreType::TSpinNoLines) return "TSpinNoLines";
    else if (scoreType == ScoreType::TSpinSingle) return "TSpinSingle";
    else if (scoreType == ScoreType::TSpinDouble) return "TSpinDouble";
    else if (scoreType == ScoreType::TSpinTriple) return "TSpinTriple";
    else if (scoreType == ScoreType::SinglePerfectClear) return "SinglePerfectClear";
    else if (scoreType == ScoreType::DoublePerfectClear) return "DoublePerfectClear";
    else if (scoreType == ScoreType::TriplePerfectClear) return "TriplePerfectClear";
    else if (scoreType == ScoreType::TetrisPerfectClear) return "TetrisPerfectClear";
    else if (scoreType == ScoreType::BackToBackTetrisPerfectClear) return "BackToBackTetrisPerfectClear";
    else return "";
}
void UI::drawScoreType(ScoreType scoreType, int comboCount, bool backToBack) {
    std::string text = to_string(scoreType);
    if (comboCount > 0) text += "\nCombo x" + std::to_string(comboCount);
    if (backToBack) text += "\nBackToBack";
    textScoreType.setString(text);
    window.draw(textScoreType);
}
