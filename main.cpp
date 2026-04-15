#include <SDL3/SDL_main.h>
#include "Game.h"

int main(int argc, char* argv[]) {
    SquareJumpGame game;
    if (!game.init()) {
        SDL_Log("Failed to initialize game.");
        return 1;
    }
    game.run();
    game.shutdown();
    return 0;
}
