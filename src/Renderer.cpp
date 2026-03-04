#include "../include/Renderer.h"
#include <iostream>
#include <map>
#include <random>

Renderer::Renderer() {
}

Renderer::~Renderer() {
}

bool Renderer::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    window = SDL_CreateWindow("FNAF C++ Edition", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!window) return false;

    sdlRenderer = SDL_CreateRenderer(window, NULL);
    if (!sdlRenderer) return false;

    return true;

}

void Renderer::render(const Office& office, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy, Uint64 elapsedTime, Uint64 remainingTime) {
    // STAP 1: Reset de kwast en maak het scherm ALTIJD eerst schoon
    // We kiezen een basiskleur (donkerblauw/zwart)
    SDL_SetRenderDrawColor(sdlRenderer, 10, 10, 15, 255);
    SDL_RenderClear(sdlRenderer);

    // STAP 2: Teken de inhoud (Kantoor OF Camera's)
    if (!office.cameraActive) {
        // We tekenen het kantoor
        drawDoorStatus(office.leftDoorClosed, office.rightDoorClosed);
    } else {
        // We tekenen de camera-omgeving
        // Eerst een blauwe achtergrond voor de monitor
        SDL_SetRenderDrawColor(sdlRenderer, 0, 20, 40, 255);
        SDL_RenderFillRect(sdlRenderer, NULL); // Vult het hele scherm

        drawCameraMap(office.currentCamera, bonnie, chica, freddy);
        drawStatic();
    }

    // STAP 3: Teken de UI (Altijd bovenop)
    drawPowerBar(office.powerLevel);
    drawCameraBar(office.cameraActive);
    drawWekker(elapsedTime, remainingTime);

    // STAP 4: Presenteer het resultaat aan de videokaart
    // Dit moet ALTIJD als allerlaatste gebeuren!
    SDL_RenderPresent(sdlRenderer);
}

void Renderer::clean() {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_Quit();
}

void Renderer::drawPowerBar(float powerLevel) {
    // Achterkant van de balk (leeg)
    SDL_FRect bgRect = { 50, 520, 200, 30 };
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
    SDL_RenderFillRect(sdlRenderer, &bgRect);

    // De gevulde stroom (groen/rood)
    SDL_FRect powerRect = { 50, 520, (powerLevel * 2), 30 };
    if (powerLevel > 20) SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
    else SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255); // Rood bij lage stroom
    
    SDL_RenderFillRect(sdlRenderer, &powerRect);
}

void Renderer::drawDoorStatus(bool left, bool right) {
    // === ACHTERGROND - MUREN ===
    // Achter muur (veel donkerder voor enge sfeer)
    SDL_SetRenderDrawColor(sdlRenderer, 20, 18, 25, 255);
    SDL_FRect backWall = {150, 50, 500, 300};
    SDL_RenderFillRect(sdlRenderer, &backWall);

    // Zijmuren (bijna zwart)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 22, 30, 255);
    SDL_FRect leftWall = {0, 50, 150, 300};
    SDL_RenderFillRect(sdlRenderer, &leftWall);
    SDL_FRect rightWall = {650, 50, 150, 300};
    SDL_RenderFillRect(sdlRenderer, &rightWall);

    // === VLOER ===
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 22, 255);
    SDL_FRect floor = {0, 350, 800, 250};
    SDL_RenderFillRect(sdlRenderer, &floor);

    // Vloer tegels (voor detail - subtiel)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 18, 255);
    for (int i = 0; i < 800; i += 100) {
        SDL_RenderLine(sdlRenderer, i, 350, i, 600);
    }
    for (int j = 350; j < 600; j += 80) {
        SDL_RenderLine(sdlRenderer, 0, j, 800, j);
    }

    // === PLAFOND ===
    SDL_SetRenderDrawColor(sdlRenderer, 18, 15, 20, 255);
    SDL_FRect ceiling = {0, 0, 800, 50};
    SDL_RenderFillRect(sdlRenderer, &ceiling);

    // Plafond ventilator (donker metaal)
    SDL_SetRenderDrawColor(sdlRenderer, 40, 40, 42, 255);
    SDL_FRect fan = {380, 10, 40, 30};
    SDL_RenderFillRect(sdlRenderer, &fan);
    SDL_FRect fanBlade1 = {350, 20, 100, 10};
    SDL_FRect fanBlade2 = {390, 5, 20, 40};
    SDL_RenderFillRect(sdlRenderer, &fanBlade1);
    SDL_RenderFillRect(sdlRenderer, &fanBlade2);

    // === BUREAU/DESK ===
    // Tafelblad (oud, donker hout)
    SDL_SetRenderDrawColor(sdlRenderer, 40, 30, 20, 255);
    SDL_FRect desk = {200, 400, 400, 150};
    SDL_RenderFillRect(sdlRenderer, &desk);

    // Tafel rand (3D effect - donkerder)
    SDL_SetRenderDrawColor(sdlRenderer, 50, 40, 25, 255);
    SDL_FRect deskEdge = {200, 400, 400, 15};
    SDL_RenderFillRect(sdlRenderer, &deskEdge);

    // Tafel poten (bijna zwart)
    SDL_SetRenderDrawColor(sdlRenderer, 35, 25, 15, 255);
    SDL_FRect leg1 = {220, 550, 20, 50};
    SDL_FRect leg2 = {560, 550, 20, 50};
    SDL_RenderFillRect(sdlRenderer, &leg1);
    SDL_RenderFillRect(sdlRenderer, &leg2);

    // === MONITOR OP BUREAU ===
    // Monitor voet (bijna zwart)
    SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 15, 255);
    SDL_FRect monitorStand = {370, 480, 60, 40};
    SDL_RenderFillRect(sdlRenderer, &monitorStand);

    // Monitor scherm (uit/zwart met subtiele gloed)
    SDL_SetRenderDrawColor(sdlRenderer, 8, 10, 12, 255);
    SDL_FRect monitor = {330, 420, 140, 80};
    SDL_RenderFillRect(sdlRenderer, &monitor);

    // Monitor rand
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 22, 255);
    SDL_RenderRect(sdlRenderer, &monitor);

    // === POSTERS OP MUUR ===
    // Poster 1 (links - vervaagd en donker)
    SDL_SetRenderDrawColor(sdlRenderer, 45, 35, 30, 255);
    SDL_FRect poster1 = {170, 100, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &poster1);
    SDL_SetRenderDrawColor(sdlRenderer, 60, 45, 35, 255);
    SDL_RenderRect(sdlRenderer, &poster1);

    // Poster 2 (rechts)
    SDL_FRect poster2 = {570, 120, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &poster2);
    SDL_SetRenderDrawColor(sdlRenderer, 60, 45, 35, 255);
    SDL_RenderRect(sdlRenderer, &poster2);

    // === DEUREN (NU MET MEER DETAIL) ===
    // Linker deur frame (donker hout)
    SDL_SetRenderDrawColor(sdlRenderer, 35, 28, 22, 255);
    SDL_FRect leftDoorFrame = {30, 80, 120, 270};
    SDL_RenderFillRect(sdlRenderer, &leftDoorFrame);

    // Linker deur
    SDL_FRect leftDoor = {40, 90, 100, 250};
    if (left) {
        // Dicht - deur zichtbaar (oud donker hout)
        SDL_SetRenderDrawColor(sdlRenderer, 60, 50, 40, 255);
        SDL_RenderFillRect(sdlRenderer, &leftDoor);

        // Deur panelen (details - nog donkerder)
        SDL_SetRenderDrawColor(sdlRenderer, 50, 40, 30, 255);
        SDL_FRect panel1 = {50, 100, 80, 50};
        SDL_FRect panel2 = {50, 160, 80, 50};
        SDL_FRect panel3 = {50, 220, 80, 50};
        SDL_RenderFillRect(sdlRenderer, &panel1);
        SDL_RenderFillRect(sdlRenderer, &panel2);
        SDL_RenderFillRect(sdlRenderer, &panel3);

        // Deurknop (gedimd messing)
        SDL_SetRenderDrawColor(sdlRenderer, 90, 80, 50, 255);
        SDL_FRect leftHandle = {130, 210, 8, 15};
        SDL_RenderFillRect(sdlRenderer, &leftHandle);
    } else {
        // Open - donkere, angstaanjagende opening
        SDL_SetRenderDrawColor(sdlRenderer, 5, 3, 8, 255);
        SDL_RenderFillRect(sdlRenderer, &leftDoor);
    }
    SDL_SetRenderDrawColor(sdlRenderer, 20, 15, 10, 255);
    SDL_RenderRect(sdlRenderer, &leftDoor);

    // Rechter deur frame (donker hout)
    SDL_SetRenderDrawColor(sdlRenderer, 35, 28, 22, 255);
    SDL_FRect rightDoorFrame = {650, 80, 120, 270};
    SDL_RenderFillRect(sdlRenderer, &rightDoorFrame);

    // Rechter deur
    SDL_FRect rightDoor = {660, 90, 100, 250};
    if (right) {
        // Dicht - deur zichtbaar (oud donker hout)
        SDL_SetRenderDrawColor(sdlRenderer, 60, 50, 40, 255);
        SDL_RenderFillRect(sdlRenderer, &rightDoor);

        // Deur panelen (nog donkerder)
        SDL_SetRenderDrawColor(sdlRenderer, 50, 40, 30, 255);
        SDL_FRect panel1 = {670, 100, 80, 50};
        SDL_FRect panel2 = {670, 160, 80, 50};
        SDL_FRect panel3 = {670, 220, 80, 50};
        SDL_RenderFillRect(sdlRenderer, &panel1);
        SDL_RenderFillRect(sdlRenderer, &panel2);
        SDL_RenderFillRect(sdlRenderer, &panel3);

        // Deurknop (gedimd messing)
        SDL_SetRenderDrawColor(sdlRenderer, 90, 80, 50, 255);
        SDL_FRect rightHandle = {662, 210, 8, 15};
        SDL_RenderFillRect(sdlRenderer, &rightHandle);
    } else {
        // Open - donkere, angstaanjagende opening
        SDL_SetRenderDrawColor(sdlRenderer, 5, 3, 8, 255);
        SDL_RenderFillRect(sdlRenderer, &rightDoor);
    }
    SDL_SetRenderDrawColor(sdlRenderer, 20, 15, 10, 255);
    SDL_RenderRect(sdlRenderer, &rightDoor);

    // === LAMPJES/VERLICHTING INDICATORS ===
    // Linker deur licht indicator
    if (left) {
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255); // Rood = dicht
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255); // Groen = open
    }
    SDL_FRect leftLight = {90, 360, 15, 15};
    SDL_RenderFillRect(sdlRenderer, &leftLight);

    // Rechter deur licht indicator
    if (right) {
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
    }
    SDL_FRect rightLight = {695, 360, 15, 15};
    SDL_RenderFillRect(sdlRenderer, &rightLight);

    // === SCHADUWEN (voor diepte en enge sfeer) ===
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 120);
    SDL_FRect shadow1 = {200, 545, 400, 8}; // Bureau schaduw (groter en donkerder)
    SDL_RenderFillRect(sdlRenderer, &shadow1);

    // Extra schaduwen in de hoeken voor meer angst
    SDL_FRect shadowCornerLeft = {0, 0, 100, 100};
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 60);
    SDL_RenderFillRect(sdlRenderer, &shadowCornerLeft);

    SDL_FRect shadowCornerRight = {700, 0, 100, 100};
    SDL_RenderFillRect(sdlRenderer, &shadowCornerRight);
}

void Renderer::renderJumpscare() {
    // Maak het hele scherm knalrood
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
    SDL_Delay(500); // Wacht een halve seconde voor het effect
}
void Renderer::renderVictoryScreen() {
    // Groene achtergrond voor overwinning
    SDL_SetRenderDrawColor(sdlRenderer, 0, 100, 0, 255);
    SDL_RenderClear(sdlRenderer);

    // Grote gouden banner in het midden
    SDL_FRect bannerBG = {50, 150, 700, 300};
    SDL_SetRenderDrawColor(sdlRenderer, 255, 215, 0, 255); // Goud
    SDL_RenderFillRect(sdlRenderer, &bannerBG);

    // Rand om de banner
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255); // Wit
    SDL_RenderRect(sdlRenderer, &bannerBG);
    SDL_FRect innerBorder = {55, 155, 690, 290};
    SDL_RenderRect(sdlRenderer, &innerBorder);

    // Titel tekst
    SDL_SetRenderDrawColor(sdlRenderer, 0, 50, 0, 255); // Donkergroen voor tekst
    SDL_RenderDebugText(sdlRenderer, 250, 200, "=== VICTORY! ===");

    // Hoofdtekst
    SDL_RenderDebugText(sdlRenderer, 180, 250, "YOU SURVIVED THE NIGHT!");

    // Subtekst
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
    SDL_RenderDebugText(sdlRenderer, 200, 300, "Congratulations!");
    SDL_RenderDebugText(sdlRenderer, 150, 350, "The animatronics couldn't get you!");

    // Voeg wat decoratieve sterren/vierkanten toe
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255); // Geel
    SDL_FRect star1 = {100, 180, 20, 20};
    SDL_FRect star2 = {680, 180, 20, 20};
    SDL_FRect star3 = {100, 400, 20, 20};
    SDL_FRect star4 = {680, 400, 20, 20};
    SDL_RenderFillRect(sdlRenderer, &star1);
    SDL_RenderFillRect(sdlRenderer, &star2);
    SDL_RenderFillRect(sdlRenderer, &star3);
    SDL_RenderFillRect(sdlRenderer, &star4);

    SDL_RenderPresent(sdlRenderer);
}

void Renderer::toggleFullscreen() {
    bool isFullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
    SDL_SetWindowFullscreen(window, !isFullscreen);
}

void Renderer::drawCameraBar(bool cameraActive) {
    // 1. Bepaal de positie: onderaan het scherm (bijv. 800x600 venster)
    // De balk is 600 pixels breed en 40 pixels hoog
    SDL_FRect camBar = { 100, 550, 600, 40 };

    // 2. Kies de kleur
    if (cameraActive) {
        // Fel groen of blauw als de camera open is
        SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 150);
    } else {
        // Grijs/wit als de camera dicht is
        SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 100); // Lichtgrijs
    }

    // 3. Teken de balk
    SDL_RenderFillRect(sdlRenderer, &camBar);

    // 4. Teken een randje eromheen voor een 'UI' gevoel
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 180); // Puur wit
    SDL_RenderDebugText(sdlRenderer, 350, 560, " [ S for camera's ] "); // Simpele SDL3 debug tekst
}

void Renderer::drawStatic() {
    // Gebruik een random engine
    static std::default_random_engine engine(static_cast<unsigned int>(time(0)));
    // Willekeurige coördinaten over het hele scherm
    std::uniform_int_distribution<int> xPos(0, 800); // Venster breedte
    std::uniform_int_distribution<int> yPos(0, 600); // Venster hoogte
    // Willekeurige grijstinten
    std::uniform_int_distribution<int> greyValue(100, 200);

    // Teken ongeveer 500 willekeurige puntjes
    for (int i = 0; i < 500; ++i) {
        int x = xPos(engine);
        int y = yPos(engine);
        int grey = greyValue(engine);
        SDL_SetRenderDrawColor(sdlRenderer, grey, grey, grey, 255);
        SDL_RenderPoint(sdlRenderer, x, y); // Teken een enkel puntje
    }
}

void Renderer::drawWekker(Uint64 elapsedTime, Uint64 remainingTime) {
    // Converteer milliseconden naar minuten en seconden
    int remainingSeconds = (remainingTime / 1000);
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;

    // Bepaal kleur op basis van resterende tijd
    Uint8 r, g, b;
    if (remainingSeconds > 120) {
        // Meer dan 2 minuten: groen
        r = 0; g = 255; b = 0;
    } else if (remainingSeconds > 60) {
        // 1-2 minuten: geel
        r = 255; g = 255; b = 0;
    } else if (remainingSeconds > 30) {
        // 30-60 seconden: oranje
        r = 255; g = 165; b = 0;
    } else if (remainingSeconds > 10) {
        // 10-30 seconden: rood
        r = 255; g = 0; b = 0;
    } else if (remainingSeconds > 0) {
        // Minder dan 10 seconden: knipperend rood
        int blinkState = (SDL_GetTicks() / 200) % 2; // Knippering effect
        r = 255; g = (blinkState ? 100 : 0); b = (blinkState ? 100 : 0);
    } else {
        // Tijd voorbij: felrood
        r = 255; g = 0; b = 0;
    }

    // ===== ACHTERGROND BOX =====
    SDL_FRect wekkerBG = {600, 10, 190, 80};
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 30, 220);
    SDL_RenderFillRect(sdlRenderer, &wekkerBG);

    // ===== RAND EROMHEEN =====
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderRect(sdlRenderer, &wekkerBG);
    // Extra dikke rand voor meer zichtbaarheid
    SDL_FRect innerBorder = {602, 12, 186, 76};
    SDL_RenderRect(sdlRenderer, &innerBorder);

    // ===== TITEL =====
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, 605, 15, "TIME TO SURVIVE:");

    // ===== GROTE TIJDWEERGAVE =====
    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", minutes, seconds);

    // Format grote display
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderDebugText(sdlRenderer, 630, 45, timeBuffer);

    // ===== PROGRESSBALK =====
    // Toon de voortgang visueel
    float progress = (float)remainingSeconds / 120.0f; // 120 sec = 2 min
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;

    SDL_FRect progressBG = {605, 70, 180, 12};
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 200);
    SDL_RenderFillRect(sdlRenderer, &progressBG);

    SDL_FRect progressBar = {605, 70, 180 * progress, 12};
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderFillRect(sdlRenderer, &progressBar);
}

// Helper functie om Room enum naar string te converteren
const char* getRoomName(Room room) {
    switch(room) {
        case SHOW_STAGE: return "Show Stage";
        case DINING_HALL: return "Dining Hall";
        case BACKROOM: return "Backroom";
        case KITCHEN: return "Kitchen";
        case RESTROOM: return "Restroom";
        case LEFT_HALLWAY: return "Left Hall";
        case RIGHT_HALLWAY: return "Right Hall";
        case LEFT_OFFICE: return "Left Office";
        case RIGHT_OFFICE: return "Right Office";
        case JUMPSCARE: return "JUMPSCARE";
        default: return "Unknown";
    }
}

void Renderer::drawCameraMap(Room activeCamera, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Definieer de positie en grootte van elke kamer op de map
    // (Dit zijn X, Y, Breedte, Hoogte op je 800x600 scherm)
    std::map<Room, SDL_FRect> roomLayout = {
        {SHOW_STAGE,    {100, 100, 150, 80}},
        {DINING_HALL,   {280, 100, 150, 80}},
        {BACKROOM,      {100, 200, 150, 80}},
        {KITCHEN,       {280, 200, 150, 80}},
        {RESTROOM,      {460, 200, 150, 80}},
        {LEFT_HALLWAY,  {100, 300, 150, 80}},
        {RIGHT_HALLWAY, {460, 300, 150, 80}},
        {LEFT_OFFICE,   {100, 400, 150, 80}}, // Kantoor is niet op map, maar voor debug
        {RIGHT_OFFICE,  {460, 400, 150, 80}}  // Kantoor is niet op map, maar voor debug
    };

    for (auto const& [room, rect] : roomLayout) {
        // 1. Teken de kamer zelf
        if (room == activeCamera) {
            SDL_SetRenderDrawColor(sdlRenderer, 0, 150, 0, 255); // Actieve camera (Groen)
        } else {
            SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255); // Niet-actieve kamer (Donkergrijs)
        }
        SDL_RenderFillRect(sdlRenderer, &rect);

        // 2. Teken een rand
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255); // Lichtgrijze rand
        SDL_RenderRect(sdlRenderer, &rect);

        // 3. Teken de kamer naam
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255); // Wit voor tekst
        const char* roomName = getRoomName(room);
        SDL_RenderDebugText(sdlRenderer, rect.x + 5, rect.y + 5, roomName);

        // 4. Teken de animatronics
        SDL_FRect botRect;
        botRect.w = 10; botRect.h = 10; // Klein vierkantje voor de robot

        // Bonnie's positie
        if (bonnie.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255); // Magenta voor Bonnie
            botRect.x = rect.x + (rect.w / 2) - 5; // Midden van de kamer
            botRect.y = rect.y + (rect.h / 2) - 5;
            SDL_RenderFillRect(sdlRenderer, &botRect);
        }
        // Chica's positie
        if (chica.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255); // Geel voor Chica
            botRect.x = rect.x + (rect.w / 2) + 5; // Iets verschoven
            botRect.y = rect.y + (rect.h / 2) + 5;
            SDL_RenderFillRect(sdlRenderer, &botRect);
        }
        // Freddy's positie
        if (freddy.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255); // Oranje voor Freddy
            botRect.x = rect.x + (rect.w / 2) - 15; // Nog meer verschoven
            botRect.y = rect.y + (rect.h / 2) - 15;
            SDL_RenderFillRect(sdlRenderer, &botRect);
        }
    }
}