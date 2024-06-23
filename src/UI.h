//
// Created by amemiya7 on 2024/6/23.
//

#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Block.h"

class UI {
public:
    explicit UI();
    UI(const UI& ui) = delete;
    UI& operator=(const UI& ui) = delete;
    UI(UI&& ui) = delete;
    UI& operator=(UI&& ui) = delete;

    void draw();
    void clear();
    sf::RenderWindow& getWindow();
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text textLevel;
    sf::Text textLevelNumber;
    sf::Text textScore;
    sf::Text textScoreNumber;
    sf::Text textLines;
    sf::Text textLinesNumber;
};
