//
// Created by amemiya7 on 2024/6/23.
//

#include "UI.h"
#include <filesystem>
UI::UI() :
window(sf::VideoMode(1200, 675), "Tetris") {
    std::filesystem::path fontsPath = std::filesystem::path(FILE_LOCATION) / "resources" / "fonts";
    font.loadFromFile((fontsPath / "scoreFont.ttf").string());
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
}

void UI::draw() {
    window.draw(textLevel);
    window.draw(textLevelNumber);
    window.draw(textScore);
    window.draw(textScoreNumber);
    window.display();
}

void UI::clear() {
    window.clear();
}

sf::RenderWindow& UI::getWindow(){
    return window;
}
