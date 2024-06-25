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
#include "Tuner.h"
int main() {
//    Tuner tuner{};
//    tuner.run();
    Game game{};
    while(!game.shouldClose()) {
        game.run();
    }
    game.close();
}

