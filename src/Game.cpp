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

                // === SNELLE CAMERA SWITCHING MET PIJLTJESTOETSEN ===
                case SDLK_UP:
                    // Ga naar vorige camera in de lijst
                    cycleCameraPrevious();
                    office.cameraActive = true;
                    break;

                case SDLK_DOWN:
                    // Ga naar volgende camera in de lijst
                    cycleCameraNext();
                    office.cameraActive = true;
                    break;

                case SDLK_LEFT:
                    // Spring naar linkerkant kamers (Backroom -> Left Hallway -> Left Door)
                    cycleLeftSideCameras();
                    office.cameraActive = true;
                    break;

                case SDLK_RIGHT:
                    // Spring naar rechterkant kamers (Kitchen -> Right Hallway -> Right Door)
                    cycleRightSideCameras();
                    office.cameraActive = true;
                    break;

                // === SNELLE TOEGANG TOT BELANGRIJKE CAMERA'S ===
                case SDLK_Q:
                    // Q = Quick check linker deur
                    office.currentCamera = LEFT_OFFICE;
                    office.cameraActive = true;
                    break;

                case SDLK_E:
                    // E = Quick check rechter deur
                    office.currentCamera = RIGHT_OFFICE;
                    office.cameraActive = true;
                    break;

                case SDLK_TAB:
                    // TAB = Cycle door belangrijke kamers (Show Stage, Dining, beide gangen, beide deuren)
                    cycleImportantCameras();
                    office.cameraActive = true;
                    break;

                case SDLK_SPACE:
                    // SPACE = Spring terug naar Show Stage (startpunt)
                    office.currentCamera = SHOW_STAGE;
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

// === CAMERA CYCLING HELPER FUNCTIONS ===
// Ruimtelijke navigatie gebaseerd op de VISUELE POSITIE op de minimap
// UP = Ga naar kamer DIE BOVEN de huidige staat op de map
// DOWN = Ga naar kamer DIE ONDER de huidige staat op de map
// LEFT = Ga naar kamer DIE LINKS van de huidige staat op de map
// RIGHT = Ga naar kamer DIE RECHTS van de huidige staat op de map

void Game::cycleCameraNext() {
    // DOWN arrow: Ga naar de kamer ONDER de huidige op de minimap
    switch(office.currentCamera) {
        case SHOW_STAGE:
            // Show Stage (Y=80) -> Dining Hall onder (Y=155)
            office.currentCamera = DINING_HALL;
            break;
        case DINING_HALL:
            // Dining Hall (Y=155) -> blijf centraal, ga naar Right Hallway (Y=240)
            office.currentCamera = RIGHT_HALLWAY;
            break;
        case BACKROOM:
            // Backroom (Y=155) -> Left Hallway eronder (Y=240)
            office.currentCamera = LEFT_HALLWAY;
            break;
        case KITCHEN:
            // Kitchen (Y=155) -> Right Hallway eronder (Y=240)
            office.currentCamera = RIGHT_HALLWAY;
            break;
        case RESTROOM:
            // Restroom (Y=155) -> Right Hallway eronder (Y=240)
            office.currentCamera = RIGHT_HALLWAY;
            break;
        case LEFT_HALLWAY:
            // Left Hallway (Y=240) -> Left Office eronder (Y=325)
            office.currentCamera = LEFT_OFFICE;
            break;
        case RIGHT_HALLWAY:
            // Right Hallway (Y=240) -> Right Office eronder (Y=325)
            office.currentCamera = RIGHT_OFFICE;
            break;
        case LEFT_OFFICE:
        case RIGHT_OFFICE:
            // Deuren zijn onderaan, ga terug naar boven
            office.currentCamera = SHOW_STAGE;
            break;
        default:
            office.currentCamera = SHOW_STAGE;
            break;
    }
}

void Game::cycleCameraPrevious() {
    // UP arrow: Ga naar de kamer BOVEN de huidige op de minimap
    switch(office.currentCamera) {
        case SHOW_STAGE:
            // Show Stage is bovenaan, ga naar onderaan (deuren)
            office.currentCamera = LEFT_OFFICE;
            break;
        case DINING_HALL:
            // Dining Hall (Y=155) -> Show Stage erboven (Y=80)
            office.currentCamera = SHOW_STAGE;
            break;
        case BACKROOM:
            // Backroom (Y=155) -> Dining Hall erboven (ongeveer zelfde hoogte maar logisch)
            office.currentCamera = DINING_HALL;
            break;
        case KITCHEN:
            // Kitchen (Y=155) -> Dining Hall erboven
            office.currentCamera = DINING_HALL;
            break;
        case RESTROOM:
            // Restroom (Y=155) -> Kitchen of Dining Hall erboven
            office.currentCamera = KITCHEN;
            break;
        case LEFT_HALLWAY:
            // Left Hallway (Y=240) -> Backroom erboven (Y=155)
            office.currentCamera = BACKROOM;
            break;
        case RIGHT_HALLWAY:
            // Right Hallway (Y=240) -> Kitchen/Restroom erboven (Y=155)
            office.currentCamera = KITCHEN;
            break;
        case LEFT_OFFICE:
            // Left Office (Y=325) -> Left Hallway erboven (Y=240)
            office.currentCamera = LEFT_HALLWAY;
            break;
        case RIGHT_OFFICE:
            // Right Office (Y=325) -> Right Hallway erboven (Y=240)
            office.currentCamera = RIGHT_HALLWAY;
            break;
        default:
            office.currentCamera = SHOW_STAGE;
            break;
    }
}

void Game::cycleLeftSideCameras() {
    // LEFT arrow: Ga naar de kamer LINKS van de huidige op de minimap
    switch(office.currentCamera) {
        case SHOW_STAGE:
            // Show Stage (X=230) -> Backroom links (X=70)
            office.currentCamera = BACKROOM;
            break;
        case DINING_HALL:
            // Dining Hall (X=230) -> Backroom links (X=70)
            office.currentCamera = BACKROOM;
            break;
        case KITCHEN:
            // Kitchen (X=390) -> Dining Hall links (X=230)
            office.currentCamera = DINING_HALL;
            break;
        case RESTROOM:
            // Restroom (X=520) -> Kitchen links (X=390)
            office.currentCamera = KITCHEN;
            break;
        case RIGHT_HALLWAY:
            // Right Hallway (X=410) -> Left Hallway links (X=90)
            office.currentCamera = LEFT_HALLWAY;
            break;
        case RIGHT_OFFICE:
            // Right Office (X=405) -> Left Office links (X=100)
            office.currentCamera = LEFT_OFFICE;
            break;
        case BACKROOM:
        case LEFT_HALLWAY:
        case LEFT_OFFICE:
            // Al helemaal links, blijf hier of ga naar rechts
            // Ga naar de rechterkant
            office.currentCamera = DINING_HALL;
            break;
        default:
            office.currentCamera = BACKROOM;
            break;
    }
}

void Game::cycleRightSideCameras() {
    // RIGHT arrow: Ga naar de kamer RECHTS van de huidige op de minimap
    switch(office.currentCamera) {
        case SHOW_STAGE:
            // Show Stage (X=230) -> Kitchen rechts (X=390)
            office.currentCamera = KITCHEN;
            break;
        case DINING_HALL:
            // Dining Hall (X=230) -> Kitchen rechts (X=390)
            office.currentCamera = KITCHEN;
            break;
        case BACKROOM:
            // Backroom (X=70) -> Dining Hall rechts (X=230)
            office.currentCamera = DINING_HALL;
            break;
        case KITCHEN:
            // Kitchen (X=390) -> Restroom rechts (X=520)
            office.currentCamera = RESTROOM;
            break;
        case LEFT_HALLWAY:
            // Left Hallway (X=90) -> Right Hallway rechts (X=410)
            office.currentCamera = RIGHT_HALLWAY;
            break;
        case LEFT_OFFICE:
            // Left Office (X=100) -> Right Office rechts (X=405)
            office.currentCamera = RIGHT_OFFICE;
            break;
        case RESTROOM:
        case RIGHT_HALLWAY:
        case RIGHT_OFFICE:
            // Al helemaal rechts, blijf hier of ga naar links
            // Ga terug naar midden
            office.currentCamera = DINING_HALL;
            break;
        default:
            office.currentCamera = KITCHEN;
            break;
    }
}

void Game::cycleImportantCameras() {
    // TAB: cycle door alleen de belangrijkste kamers voor snelle checks
    // Show Stage -> Dining Hall -> Left Hallway -> Right Hallway -> Left Door -> Right Door
    switch(office.currentCamera) {
        case SHOW_STAGE:
            office.currentCamera = DINING_HALL;
            break;
        case DINING_HALL:
        case BACKROOM:
        case KITCHEN:
        case RESTROOM:
            // Van midden kamers ga naar linker gang
            office.currentCamera = LEFT_HALLWAY;
            break;
        case LEFT_HALLWAY:
            office.currentCamera = RIGHT_HALLWAY;
            break;
        case RIGHT_HALLWAY:
            office.currentCamera = LEFT_OFFICE;
            break;
        case LEFT_OFFICE:
            office.currentCamera = RIGHT_OFFICE;
            break;
        case RIGHT_OFFICE:
            // Loop terug naar start
            office.currentCamera = SHOW_STAGE;
            break;
        default:
            office.currentCamera = SHOW_STAGE;
            break;
    }
}
