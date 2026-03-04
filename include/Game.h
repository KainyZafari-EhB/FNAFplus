//
// Created by kainy on 2/14/2026.
//

#ifndef FNAF_GAME_H
#define FNAF_GAME_H

#include <SDL3/SDL.h>
#include "Office.h"

class Renderer;  // Forward declaration to avoid circular dependency


class Game {
    // Input: Checkt via SDL_PollEvent of je op 'A' (Linkerdeur) of 'S' (Camera) drukt.
public:
    Game();
    bool init();
    bool isRunning() const { return running; }
    const Office& getOffice() const { return office; }
    void clean();

    void check_input();

    // Update: Verlaagt de stroom elke seconde en vraagt de Animatronics of ze willen verplaatsen.

    void update();

    float berekenVerbruik();

    // Render: Vertelt de Renderer dat het tijd is om het scherm te verversen.

    void render();
    void renderJumpscare();
    void renderVictoryScreen();

    // Wekker getter methods
    bool isGameWon() const { return gameWon; }
    bool isGameLost() const { return gameLost; }
    Uint64 getElapsedTime() const { return SDL_GetTicks() - gameStartTime; }
    Uint64 getRemainingTime() const {
        Uint64 elapsed = getElapsedTime();
        return (elapsed >= NIGHT_DURATION) ? 0 : (NIGHT_DURATION - elapsed);
    }

private:
    Office office;
    SDL_Event event;
    Animatronic bonnie;
    Animatronic chica;
    Animatronic freddy;
    Renderer* renderer = nullptr;
    bool running = true;

    Uint64 laatsteCheckTijd = 0;      // Voor de vloeiende power-delta

    // Camera cycling helper functions
    void cycleCameraNext();
    void cycleCameraPrevious();
    void cycleLeftSideCameras();
    void cycleRightSideCameras();
    void cycleImportantCameras();
    Uint64 laatsteMovementCheck = 0;  // Voor de 5-seconden sprongen
    const Uint64 MOVEMENT_INTERVAL = 4970;

    // Wekker/Timer voor winconditie
    Uint64 gameStartTime = 0;         // Wanneer het spel is gestart
    const Uint64 NIGHT_DURATION = 180000; // 3 minuten = 180000 ms
    bool gameWon = false;
    bool gameLost = false;
};


#endif //FNAF_GAME_H