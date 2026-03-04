#ifndef FNAF_RENDERER_H
#define FNAF_RENDERER_H

#include <SDL3/SDL.h>

#include "Office.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();
    void render(const Office& office, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy, Uint64 elapsedTime, Uint64 remainingTime);
    void clean();
    void renderJumpscare();
    void renderVictoryScreen();
    void toggleFullscreen();

    void drawCameraBar(bool cameraActive);

    void drawStatic();

    void drawWekker(Uint64 elapsedTime, Uint64 remainingTime);

    void drawCameraMap(Room activeCamera, const Animatronic &bonnie, const Animatronic &chica,
                       const Animatronic &freddy);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    
    // Base resolution for coordinate calculations
    const int BASE_WIDTH = 800;
    const int BASE_HEIGHT = 600;

    void drawPowerBar(float powerLevel);
    void drawDoorStatus(bool left, bool right);
};

#endif