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
#include "Tuner.h"
int main() {
//    Tuner tuner{};
//    tuner.run();
    Game game{};
    while(!game.shouldClose()) {
        game.run();
    }
}

