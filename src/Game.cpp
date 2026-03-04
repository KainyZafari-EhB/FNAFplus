//
// Created by kainy on 2/14/2026.
//

#include "../include/Game.h"

#include <iostream>

#include "../include/Animatronic.h"
#include "../include/Office.h"
#include "../include/Renderer.h"
#include "SDL3/SDL_timer.h"

Game::Game() : bonnie("Bonnie", 10), chica("Chica", 10), freddy("Freddy", 10) {
    this->running = true;
    renderer = new Renderer();
}

bool Game::init() {
    gameStartTime = SDL_GetTicks();
    return renderer->init();
}

void Game::check_input() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            this->running = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            switch (event.key.key) {
                //schakelt linkse deur open/closed
                case SDLK_A:
                    office.leftDoorClosed = !office.leftDoorClosed;
                    break;
                //schakelt rechtse deur open/closed
                case SDLK_D:
                    office.rightDoorClosed = !office.rightDoorClosed;
                    break;
                case SDLK_S:
                    // Schakel camera's in/uit
                    office.cameraActive = !office.cameraActive;
                    break;
                case SDLK_F11:
                    // Toggle fullscreen
                    renderer->toggleFullscreen();
                    break;
                //sluit de applicatie als je "ESC" indrukt
                case SDLK_ESCAPE:
                    this->running = false;
                    break;

                // === CAMERA SWITCHING (1-9) ===
                case SDLK_1:
                    office.currentCamera = SHOW_STAGE;
                    office.cameraActive = true; // Activeer camera's automatisch
                    break;
                case SDLK_2:
                    office.currentCamera = DINING_HALL;
                    office.cameraActive = true;
                    break;
                case SDLK_3:
                    office.currentCamera = BACKROOM;
                    office.cameraActive = true;
                    break;
                case SDLK_4:
                    office.currentCamera = KITCHEN;
                    office.cameraActive = true;
                    break;
                case SDLK_5:
                    office.currentCamera = RESTROOM;
                    office.cameraActive = true;
                    break;
                case SDLK_6:
                    office.currentCamera = LEFT_HALLWAY;
                    office.cameraActive = true;
                    break;
                case SDLK_7:
                    office.currentCamera = RIGHT_HALLWAY;
                    office.cameraActive = true;
                    break;
                case SDLK_8:
                    office.currentCamera = LEFT_OFFICE;
                    office.cameraActive = true;
                    break;
                case SDLK_9:
                    office.currentCamera = RIGHT_OFFICE;
                    office.cameraActive = true;
                    break;
            }
        }
    }
}

void Game::update() {
    Uint64 currentTime = SDL_GetTicks();

    // Als dit de allereerste frame is, zet de tijd even goed
    if (laatsteCheckTijd == 0) {
        laatsteCheckTijd = currentTime;
        laatsteMovementCheck = currentTime;
        return;
    }

    // --- DEEL 1: STROOM VERBRUIK (Elke frame) ---
    // DeltaTime berekenen: Hoeveel ms zijn er voorbij sinds de VORIGE frame?
    float deltaTime = (currentTime - laatsteCheckTijd) / 1000.0f;
    laatsteCheckTijd = currentTime; // Update deze pas NA de berekening

    if (office.powerLevel > 0) {
        float verbruikPerSeconde = berekenVerbruik();
        office.powerLevel -= (verbruikPerSeconde * deltaTime);
    } else {
        office.powerLevel = 0;
        office.stroomUitgevallen();
    }

    // --- DEEL 2: ANIMATRONIC BEWEGING (Elke ~5 seconden) ---
    // Vergelijk de huidige tijd met het moment dat ze voor het laatst bewogen
    if (currentTime > laatsteMovementCheck + MOVEMENT_INTERVAL) {
        bonnie.move(office);
        chica.move(office);
        freddy.move(office);

        // Reset naar de huidige tijd voor de volgende 5 seconden
        laatsteMovementCheck = currentTime;

        std::cout << "[DEBUG] Stroom: " << (int)office.powerLevel << "%" << std::endl;

        if (bonnie.getCurrentRoom() == JUMPSCARE || chica.getCurrentRoom() == JUMPSCARE || freddy.getCurrentRoom() == JUMPSCARE) {
            std::cout << "GAME OVER!" << std::endl;
            gameLost = true;
            this->running = false;
        }
    }

    //DEEL 3: WEKKER (WIN-CONDITIE)
    // Check of de nacht voorbij is (6 minuten zonder Game Over)
    if (getElapsedTime() >= NIGHT_DURATION && !gameLost) {
        std::cout << "========================================" << std::endl;
        std::cout << "JE HEBT GEWONNEN! De nacht is voorbij!" << std::endl;
        std::cout << "========================================" << std::endl;
        gameWon = true;
        this->running = false;
    }

}

float Game::berekenVerbruik() {
    int actieveApparaten = 1; // Je begint altijd met 1 (de ventilator/lichten)

    if (office.leftDoorClosed) actieveApparaten++;
    if (office.rightDoorClosed) actieveApparaten++;
    if (office.cameraActive)    actieveApparaten++;

    // Totaal verbruik per seconde
    return actieveApparaten * office.usagePerDevice;
}

void Game::render() {
    renderer->render(office, bonnie, chica, freddy, getElapsedTime(), getRemainingTime());
}

void Game::renderJumpscare() {
    renderer->renderJumpscare();
}

void Game::renderVictoryScreen() {
    renderer->renderVictoryScreen();
}

void Game::clean() {
    if (renderer) {
        renderer->clean();
        delete renderer;
        renderer = nullptr;
    }
}

