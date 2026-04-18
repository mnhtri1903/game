#include <SDL3/SDL_main.h>
#include "Game.h"
#include "EmbeddedAssets.h"   // ← NEW: extract player1.png from .exe resources

int main(int argc, char* argv[]) {

    // ── Step 1: unpack embedded assets (player1.png etc.) ────────────────────
    //  On Windows this writes assets next to the .exe once, then skips.
    //  On Linux/macOS this is a no-op.
    if (!EmbeddedAssets::extractAll()) {
        SDL_Log("[Warning] Some embedded assets could not be extracted. "
                "Make sure player1.png is present if the game crashes.");
    }

    // ── Step 2: normal game init & run ───────────────────────────────────────
    SquareJumpGame game;
    if (!game.init()) {
        SDL_Log("Failed to initialize game.");
        return 1;
    }

    game.run();
    game.shutdown();
    return 0;
}
