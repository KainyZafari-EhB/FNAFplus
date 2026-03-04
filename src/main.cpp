#include <SDL3/SDL.h>
#include "../include/Game.h"
#include "../include/Renderer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // 1. Instanties maken
    Game game;

    // 2. Game initialiseren (venster maken, etc.)
    if (!game.init()) {
        std::cerr << "Fout bij initialiseren van Game: " << SDL_GetError() << std::endl;
        return -1;
    }

    // 3. De Game Loop
    // We gebruiken de 'running' status uit je Game klasse
    while (game.isRunning()) {

        // STAP A: INPUT (Vang toetsen op via SDL_PollEvent)
        game.check_input();

        // STAP B: UPDATE (Bereken stroom en verplaats animatronics)
        game.update();

        // STAP C: RENDER (Teken alles naar het SDL-venster)
        // We geven de 'office' door zodat de renderer weet wat hij moet tekenen
        game.render();

        // Optioneel: Kleine vertraging om je CPU niet 100% te belasten
        SDL_Delay(1);
    }

    // 4. Game Over/Win scherm tonen
    if (game.isGameLost()) {
        game.renderJumpscare();
        SDL_Delay(2000); // Laat de speler 2 seconden naar het rode scherm staren
    } else if (game.isGameWon()) {
        game.renderVictoryScreen();
        SDL_Delay(5000); // Laat de speler 3 seconden naar het overwinningsscherm kijken
    }

    // 4. Afsluiten
    game.clean();
    SDL_Quit();

    return 0;
}