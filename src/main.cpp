#include <iostream>

#include "Game.h"

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
int main() {
    Game game{};
    while(!game.shouldClose()) {
        game.run();
    }
}

