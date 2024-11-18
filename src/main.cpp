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
//    UI ui{};

//    Game game{};
//    Tuner tuner{};
//    tuner.run();
//    tuner.runForPreview();
    Game game{};
    while(!game.shouldClose()) {
        game.run();
    }

}

