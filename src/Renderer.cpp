#include "../include/Renderer.h"
#include "../include/Office.h"
#include <iostream>
#include <map>
#include <random>

// Forward declaration for helper function
const char* getRoomName(Room room);

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

    // Stel logische presentatie in voor automatische schaling
    // Dit zorgt ervoor dat de game altijd 800x600 "denkt" te zijn,
    // maar SDL schaalt het automatisch naar de venstergrootte
    SDL_SetRenderLogicalPresentation(sdlRenderer, BASE_WIDTH, BASE_HEIGHT,
                                      SDL_LOGICAL_PRESENTATION_LETTERBOX);

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
        // We tekenen de camera feed van de geselecteerde kamer
        drawCameraView(office.currentCamera, bonnie, chica, freddy);

        // Teken camera map in de hoek (klein overzicht)
        drawCameraMap(office.currentCamera, bonnie, chica, freddy);

        // Static effect over de camera feed
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
    // Verplaatst naar rechts onder om hotkeys niet te overlappen
    // Achterkant van de balk (leeg)
    SDL_FRect bgRect = { 550, 520, 200, 30 };
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
    SDL_RenderFillRect(sdlRenderer, &bgRect);

    // De gevulde stroom (groen/rood)
    SDL_FRect powerRect = { 550, 520, (powerLevel * 2), 30 };
    if (powerLevel > 20) SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
    else SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255); // Rood bij lage stroom
    
    SDL_RenderFillRect(sdlRenderer, &powerRect);

    // Label boven de power bar
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, 555, 505, "POWER:");

    // Percentage weergave
    char powerText[20];
    snprintf(powerText, sizeof(powerText), "%.0f%%", powerLevel);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 660, 525, powerText);
}

void Renderer::drawDoorStatus(bool left, bool right) {
    // === ACHTERGROND - MUREN ===
    // Achter muur (industriële betonkleur)
    SDL_SetRenderDrawColor(sdlRenderer, 12, 10, 15, 255);
    SDL_FRect backWall = {150, 50, 500, 300};
    SDL_RenderFillRect(sdlRenderer, &backWall);

    // Zijmuren (donker beton/cement)
    SDL_SetRenderDrawColor(sdlRenderer, 15, 12, 18, 255);
    SDL_FRect leftWall = {0, 50, 150, 300};
    SDL_RenderFillRect(sdlRenderer, &leftWall);
    SDL_FRect rightWall = {650, 50, 150, 300};
    SDL_RenderFillRect(sdlRenderer, &rightWall);

    // === VLOER ===
    SDL_SetRenderDrawColor(sdlRenderer, 18, 15, 12, 255);
    SDL_FRect floor = {0, 350, 800, 250};
    SDL_RenderFillRect(sdlRenderer, &floor);

    // Vloer tegels (industriële tegels)
    SDL_SetRenderDrawColor(sdlRenderer, 15, 12, 10, 255);
    for (int i = 0; i < 800; i += 100) {
        SDL_RenderLine(sdlRenderer, i, 350, i, 600);
    }
    for (int j = 350; j < 600; j += 80) {
        SDL_RenderLine(sdlRenderer, 0, j, 800, j);
    }

    // === PLAFOND MET VENTILATIE ===
    SDL_SetRenderDrawColor(sdlRenderer, 10, 8, 12, 255);
    SDL_FRect ceiling = {0, 0, 800, 50};
    SDL_RenderFillRect(sdlRenderer, &ceiling);

    // Ventilatierooster (links)
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 5, 255);
    SDL_FRect vent1 = {50, 10, 60, 30};
    SDL_RenderFillRect(sdlRenderer, &vent1);
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
    for (int v = 15; v < 35; v += 5) {
        SDL_RenderLine(sdlRenderer, 50, v, 110, v);
    }

    // Ventilatierooster (rechts)
    SDL_FRect vent2 = {690, 10, 60, 30};
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 5, 255);
    SDL_RenderFillRect(sdlRenderer, &vent2);
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
    for (int v = 15; v < 35; v += 5) {
        SDL_RenderLine(sdlRenderer, 690, v, 750, v);
    }

    // Plafond lamp (flikkerende TL-buis sfeer)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 25, 20, 255);
    SDL_FRect lightFixture = {350, 8, 100, 15};
    SDL_RenderFillRect(sdlRenderer, &lightFixture);

    // === SECURITY BUREAU/DESK ===
    // Metalen bureau (grijs/groen industrieel)
    SDL_SetRenderDrawColor(sdlRenderer, 22, 25, 20, 255);
    SDL_FRect desk = {200, 400, 400, 150};
    SDL_RenderFillRect(sdlRenderer, &desk);

    // Bureau rand (metaal effect)
    SDL_SetRenderDrawColor(sdlRenderer, 28, 30, 25, 255);
    SDL_FRect deskEdge = {200, 400, 400, 12};
    SDL_RenderFillRect(sdlRenderer, &deskEdge);

    // Metalen poten (industrieel)
    SDL_SetRenderDrawColor(sdlRenderer, 18, 18, 18, 255);
    SDL_FRect leg1 = {220, 550, 25, 50};
    SDL_FRect leg2 = {555, 550, 25, 50};
    SDL_RenderFillRect(sdlRenderer, &leg1);
    SDL_RenderFillRect(sdlRenderer, &leg2);

    // === SECURITY MONITORS EN APPARATUUR ===
    // Monitor 1 (hoofd monitor - groter)
    SDL_SetRenderDrawColor(sdlRenderer, 8, 8, 8, 255);
    SDL_FRect monitor1Stand = {340, 480, 50, 35};
    SDL_RenderFillRect(sdlRenderer, &monitor1Stand);

    SDL_SetRenderDrawColor(sdlRenderer, 5, 6, 8, 255);
    SDL_FRect monitor1 = {310, 420, 120, 75};
    SDL_RenderFillRect(sdlRenderer, &monitor1);
    SDL_SetRenderDrawColor(sdlRenderer, 12, 12, 14, 255);
    SDL_RenderRect(sdlRenderer, &monitor1);

    // Klein groen lampje op monitor (aan)
    SDL_SetRenderDrawColor(sdlRenderer, 0, 100, 0, 255);
    SDL_FRect monitorLight = {425, 425, 3, 3};
    SDL_RenderFillRect(sdlRenderer, &monitorLight);

    // Monitor 2 (kleiner - rechts)
    SDL_FRect monitor2 = {440, 440, 80, 55};
    SDL_SetRenderDrawColor(sdlRenderer, 5, 6, 8, 255);
    SDL_RenderFillRect(sdlRenderer, &monitor2);
    SDL_SetRenderDrawColor(sdlRenderer, 12, 12, 14, 255);
    SDL_RenderRect(sdlRenderer, &monitor2);

    // Toetsenbord op bureau
    SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 15, 255);
    SDL_FRect keyboard = {450, 510, 90, 25};
    SDL_RenderFillRect(sdlRenderer, &keyboard);

    // Koffiekopje (security guard essentials!)
    SDL_SetRenderDrawColor(sdlRenderer, 80, 60, 50, 255);
    SDL_FRect coffeeCup = {550, 515, 20, 25};
    SDL_RenderFillRect(sdlRenderer, &coffeeCup);
    SDL_SetRenderDrawColor(sdlRenderer, 20, 15, 10, 255);
    SDL_FRect coffeeTop = {550, 515, 20, 5};
    SDL_RenderFillRect(sdlRenderer, &coffeeTop);

    // === FILING CABINETS (ARCHIEFKASTEN) ===
    // Linkse archiefkast (naast linker deur)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 28, 25, 255);
    SDL_FRect fileCabinet1 = {155, 200, 70, 140};
    SDL_RenderFillRect(sdlRenderer, &fileCabinet1);
    SDL_SetRenderDrawColor(sdlRenderer, 18, 20, 18, 255);
    SDL_RenderRect(sdlRenderer, &fileCabinet1);

    // Lades
    SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 15, 255);
    for (int d = 0; d < 3; d++) {
        SDL_FRect drawer = {157, 205.0f + (d * 45.0f), 66, 40};
        SDL_RenderRect(sdlRenderer, &drawer);
        // Handvat
        SDL_FRect handle = {185, 220.0f + (d * 45.0f), 10, 3};
        SDL_RenderFillRect(sdlRenderer, &handle);
    }

    // Rechtse archiefkast
    SDL_SetRenderDrawColor(sdlRenderer, 25, 28, 25, 255);
    SDL_FRect fileCabinet2 = {575, 200, 70, 140};
    SDL_RenderFillRect(sdlRenderer, &fileCabinet2);
    SDL_SetRenderDrawColor(sdlRenderer, 18, 20, 18, 255);
    SDL_RenderRect(sdlRenderer, &fileCabinet2);

    // Lades
    SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 15, 255);
    for (int d = 0; d < 3; d++) {
        SDL_FRect drawer = {577, 205.0f + (d * 45.0f), 66, 40};
        SDL_RenderRect(sdlRenderer, &drawer);
        // Handvat
        SDL_FRect handle = {605, 220.0f + (d * 45.0f), 10, 3};
        SDL_RenderFillRect(sdlRenderer, &handle);
    }

    // === RESTAURANT POSTERS (in plaats van gewone posters) ===
    // "FREDDY FAZBEAR'S PIZZA" poster (links)
    SDL_SetRenderDrawColor(sdlRenderer, 40, 20, 20, 255); // Rode rand
    SDL_FRect poster1Border = {168, 98, 64, 84};
    SDL_RenderFillRect(sdlRenderer, &poster1Border);
    SDL_SetRenderDrawColor(sdlRenderer, 25, 15, 10, 255);
    SDL_FRect poster1 = {170, 100, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &poster1);
    // Decoratieve elementen (ballon/pizza suggestie)
    SDL_SetRenderDrawColor(sdlRenderer, 80, 40, 40, 255);
    SDL_FRect pizzaIcon = {185, 115, 30, 25};
    SDL_RenderFillRect(sdlRenderer, &pizzaIcon);

    // "RULES & SAFETY" poster (rechts)
    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 20, 255); // Gele rand
    SDL_FRect poster2Border = {568, 118, 64, 84};
    SDL_RenderFillRect(sdlRenderer, &poster2Border);
    SDL_SetRenderDrawColor(sdlRenderer, 20, 18, 12, 255);
    SDL_FRect poster2 = {570, 120, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &poster2);
    // Tekst lijnen suggestie
    SDL_SetRenderDrawColor(sdlRenderer, 40, 38, 30, 255);
    for (int line = 0; line < 5; line++) {
        SDL_FRect textLine = {575, 130.0f + (line * 12.0f), 50, 2};
        SDL_RenderFillRect(sdlRenderer, &textLine);
    }

    // === DEUREN (SECURITY DOORS) ===
    // Linker deur frame (metalen frame)
    SDL_SetRenderDrawColor(sdlRenderer, 22, 22, 20, 255);
    SDL_FRect leftDoorFrame = {30, 80, 120, 270};
    SDL_RenderFillRect(sdlRenderer, &leftDoorFrame);

    // Linker deur
    SDL_FRect leftDoor = {40, 90, 100, 250};
    if (left) {
        // Dicht - metalen security deur
        SDL_SetRenderDrawColor(sdlRenderer, 35, 35, 32, 255);
        SDL_RenderFillRect(sdlRenderer, &leftDoor);

        // Metalen panelen
        SDL_SetRenderDrawColor(sdlRenderer, 28, 28, 25, 255);
        SDL_FRect panel1 = {50, 100, 80, 70};
        SDL_FRect panel2 = {50, 180, 80, 70};
        SDL_FRect panel3 = {50, 260, 80, 70};
        SDL_RenderFillRect(sdlRenderer, &panel1);
        SDL_RenderFillRect(sdlRenderer, &panel2);
        SDL_RenderFillRect(sdlRenderer, &panel3);

        // Verticale rand/scharnieren
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
        SDL_FRect hinge1 = {45, 110, 8, 20};
        SDL_FRect hinge2 = {45, 210, 8, 20};
        SDL_FRect hinge3 = {45, 300, 8, 20};
        SDL_RenderFillRect(sdlRenderer, &hinge1);
        SDL_RenderFillRect(sdlRenderer, &hinge2);
        SDL_RenderFillRect(sdlRenderer, &hinge3);

        // Deurknop (industrieel)
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 48, 255);
        SDL_FRect leftHandle = {125, 210, 10, 20};
        SDL_RenderFillRect(sdlRenderer, &leftHandle);
    } else {
        // Open - donkere gang
        SDL_SetRenderDrawColor(sdlRenderer, 3, 2, 5, 255);
        SDL_RenderFillRect(sdlRenderer, &leftDoor);
    }
    SDL_SetRenderDrawColor(sdlRenderer, 12, 10, 6, 255);
    SDL_RenderRect(sdlRenderer, &leftDoor);

    // Rechter deur frame (metalen frame)
    SDL_SetRenderDrawColor(sdlRenderer, 22, 22, 20, 255);
    SDL_FRect rightDoorFrame = {650, 80, 120, 270};
    SDL_RenderFillRect(sdlRenderer, &rightDoorFrame);

    // Rechter deur
    SDL_FRect rightDoor = {660, 90, 100, 250};
    if (right) {
        // Dicht - metalen security deur
        SDL_SetRenderDrawColor(sdlRenderer, 35, 35, 32, 255);
        SDL_RenderFillRect(sdlRenderer, &rightDoor);

        // Metalen panelen
        SDL_SetRenderDrawColor(sdlRenderer, 28, 28, 25, 255);
        SDL_FRect panel1 = {670, 100, 80, 70};
        SDL_FRect panel2 = {670, 180, 80, 70};
        SDL_FRect panel3 = {670, 260, 80, 70};
        SDL_RenderFillRect(sdlRenderer, &panel1);
        SDL_RenderFillRect(sdlRenderer, &panel2);
        SDL_RenderFillRect(sdlRenderer, &panel3);

        // Verticale rand/scharnieren
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
        SDL_FRect hinge1 = {747, 110, 8, 20};
        SDL_FRect hinge2 = {747, 210, 8, 20};
        SDL_FRect hinge3 = {747, 300, 8, 20};
        SDL_RenderFillRect(sdlRenderer, &hinge1);
        SDL_RenderFillRect(sdlRenderer, &hinge2);
        SDL_RenderFillRect(sdlRenderer, &hinge3);

        // Deurknop (industrieel)
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 48, 255);
        SDL_FRect rightHandle = {665, 210, 10, 20};
        SDL_RenderFillRect(sdlRenderer, &rightHandle);
    } else {
        // Open - donkere gang
        SDL_SetRenderDrawColor(sdlRenderer, 3, 2, 5, 255);
        SDL_RenderFillRect(sdlRenderer, &rightDoor);
    }
    SDL_SetRenderDrawColor(sdlRenderer, 12, 10, 6, 255);
    SDL_RenderRect(sdlRenderer, &rightDoor);

    // === DEUR CONTROL BUTTONS ===
    // Linker deur knop paneel
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
    SDL_FRect leftPanel = {85, 355, 30, 40};
    SDL_RenderFillRect(sdlRenderer, &leftPanel);

    // Knop indicator licht
    if (left) {
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255); // Rood = dicht
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 0, 255); // Groen = open
    }
    SDL_FRect leftLight = {95, 370, 10, 10};
    SDL_RenderFillRect(sdlRenderer, &leftLight);

    // Rechter deur knop paneel
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
    SDL_FRect rightPanel = {685, 355, 30, 40};
    SDL_RenderFillRect(sdlRenderer, &rightPanel);

    // Knop indicator licht
    if (right) {
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 0, 255);
    }
    SDL_FRect rightLight = {695, 370, 10, 10};
    SDL_RenderFillRect(sdlRenderer, &rightLight);

    // === SCHADUWEN EN VIGNETTE ===
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 100);
    SDL_FRect shadow1 = {200, 545, 400, 8};
    SDL_RenderFillRect(sdlRenderer, &shadow1);

    // Subtiele vignette
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 25);
    SDL_FRect topVignette1 = {0, 0, 800, 15};
    SDL_RenderFillRect(sdlRenderer, &topVignette1);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 15);
    SDL_FRect topVignette2 = {0, 15, 800, 15};
    SDL_RenderFillRect(sdlRenderer, &topVignette2);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 8);
    SDL_FRect topVignette3 = {0, 30, 800, 15};
    SDL_RenderFillRect(sdlRenderer, &topVignette3);

    // Zijkant schaduwen
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 20);
    SDL_FRect leftEdgeVignette = {0, 45, 25, 300};
    SDL_RenderFillRect(sdlRenderer, &leftEdgeVignette);
    SDL_FRect rightEdgeVignette = {775, 45, 25, 300};
    SDL_RenderFillRect(sdlRenderer, &rightEdgeVignette);
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
    // Kleine, subtiele hint boven in het midden (alleen als camera's UIT zijn)
    if (!cameraActive) {
        // Knippering effect om aandacht te trekken
        int blinkState = (SDL_GetTicks() / 800) % 2;

        if (blinkState) {
            // Semi-transparante achtergrond box
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 120);
            SDL_FRect hintBox = { 320, 100, 160, 25 };
            SDL_RenderFillRect(sdlRenderer, &hintBox);

            // Subtiele rand
            SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 180);
            SDL_RenderRect(sdlRenderer, &hintBox);

            // Tekst in geel/wit voor zichtbaarheid
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 100, 255);
            SDL_RenderDebugText(sdlRenderer, 330, 105, "[S] Open Cameras");
        }
    } else {
        // Als camera's OPEN zijn, toon kleine indicator
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 100);
        SDL_FRect statusBox = { 360, 100, 80, 20 };
        SDL_RenderFillRect(sdlRenderer, &statusBox);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 365, 103, "CAM ON");
    }
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

    // ===== ACHTERGROND BOX (LINKS BOVEN - UIT DE WEG VAN MINIMAP) =====
    SDL_FRect wekkerBG = {20, 10, 190, 80};
    SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 30, 220);
    SDL_RenderFillRect(sdlRenderer, &wekkerBG);

    // ===== RAND EROMHEEN =====
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderRect(sdlRenderer, &wekkerBG);
    // Extra dikke rand voor meer zichtbaarheid
    SDL_FRect innerBorder = {22, 12, 186, 76};
    SDL_RenderRect(sdlRenderer, &innerBorder);

    // ===== TITEL =====
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, 25, 15, "TIME TO SURVIVE:");

    // ===== GROTE TIJDWEERGAVE =====
    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", minutes, seconds);

    // Format grote display
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderDebugText(sdlRenderer, 50, 45, timeBuffer);

    // ===== PROGRESSBALK =====
    // Toon de voortgang visueel
    float progress = (float)remainingSeconds / 120.0f; // 120 sec = 2 min
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;

    SDL_FRect progressBG = {25, 70, 180, 12};
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 200);
    SDL_RenderFillRect(sdlRenderer, &progressBG);

    SDL_FRect progressBar = {25, 70, 180 * progress, 12};
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

// Helper functie om te checken of een animatronic in deze kamer is
bool isAnimatronicInRoom(Room room, const Animatronic& animatronic) {
    return animatronic.getCurrentRoom() == room;
}

void Renderer::drawCameraMap(Room activeCamera, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Nog kleinere minimap in de rechter bovenhoek (minimaal)
    float offsetX = 610.0f;  // Helemaal rechts
    float offsetY = 50.0f;   // Helemaal boven
    float scale = 0.18f;     // Zeer klein (18% van origineel)

    // Zeer transparante achtergrond voor minimap
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 100);
    SDL_FRect minimapBG = {offsetX - 5, offsetY - 15, 150, 125};
    SDL_RenderFillRect(sdlRenderer, &minimapBG);

    // Subtiele rand
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 60, 150);
    SDL_RenderRect(sdlRenderer, &minimapBG);

    // Klein label
    SDL_SetRenderDrawColor(sdlRenderer, 160, 160, 160, 255);
    SDL_RenderDebugText(sdlRenderer, offsetX, offsetY - 10, "MAP");

    // Definieer kamers (geschaald en verplaatst)
    std::map<Room, SDL_FRect> roomLayout = {
        {SHOW_STAGE,    {offsetX + (230 * scale), offsetY + (80 * scale), 130 * scale, 55 * scale}},
        {DINING_HALL,   {offsetX + (230 * scale), offsetY + (155 * scale), 130 * scale, 55 * scale}},
        {BACKROOM,      {offsetX + (70 * scale), offsetY + (155 * scale), 110 * scale, 55 * scale}},
        {KITCHEN,       {offsetX + (390 * scale), offsetY + (155 * scale), 110 * scale, 55 * scale}},
        {RESTROOM,      {offsetX + (520 * scale), offsetY + (155 * scale), 110 * scale, 55 * scale}},
        {LEFT_HALLWAY,  {offsetX + (90 * scale), offsetY + (240 * scale), 110 * scale, 55 * scale}},
        {RIGHT_HALLWAY, {offsetX + (410 * scale), offsetY + (240 * scale), 110 * scale, 55 * scale}},
        {LEFT_OFFICE,   {offsetX + (100 * scale), offsetY + (325 * scale), 105 * scale, 60 * scale}},
        {RIGHT_OFFICE,  {offsetX + (405 * scale), offsetY + (325 * scale), 105 * scale, 60 * scale}}
    };

    // Teken verbindingslijnen (dunner voor minimap)
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 60, 255);
    SDL_RenderLine(sdlRenderer, offsetX + (295 * scale), offsetY + (135 * scale), offsetX + (295 * scale), offsetY + (155 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (230 * scale), offsetY + (182 * scale), offsetX + (180 * scale), offsetY + (182 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (360 * scale), offsetY + (182 * scale), offsetX + (445 * scale), offsetY + (182 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (360 * scale), offsetY + (182 * scale), offsetX + (575 * scale), offsetY + (182 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (125 * scale), offsetY + (210 * scale), offsetX + (145 * scale), offsetY + (240 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (445 * scale), offsetY + (210 * scale), offsetX + (465 * scale), offsetY + (240 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (145 * scale), offsetY + (295 * scale), offsetX + (152 * scale), offsetY + (325 * scale));
    SDL_RenderLine(sdlRenderer, offsetX + (465 * scale), offsetY + (295 * scale), offsetX + (458 * scale), offsetY + (325 * scale));

    // Office box (niet geschaald in minimap)
    SDL_FRect officeSpace = {offsetX + (255 * scale), offsetY + (425 * scale), 100 * scale, 70 * scale};
    SDL_SetRenderDrawColor(sdlRenderer, 60, 80, 120, 255);
    SDL_RenderFillRect(sdlRenderer, &officeSpace);
    SDL_SetRenderDrawColor(sdlRenderer, 100, 140, 200, 255);
    SDL_RenderRect(sdlRenderer, &officeSpace);

    // Teken kamers met nummer labels
    for (auto const& [room, rect] : roomLayout) {
        // Bepaal kleur en actieve status
        if (room == activeCamera) {
            SDL_SetRenderDrawColor(sdlRenderer, 0, 200, 0, 255); // Actieve camera (groen)
        } else if (room == LEFT_OFFICE || room == RIGHT_OFFICE) {
            SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255); // Deuren (rood)
        } else {
            SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 60, 255); // Inactief
        }
        SDL_RenderFillRect(sdlRenderer, &rect);
        SDL_SetRenderDrawColor(sdlRenderer, 120, 120, 120, 255);
        SDL_RenderRect(sdlRenderer, &rect);

        // Teken hotkey nummer voor elke kamer (groot en duidelijk)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        const char* keyLabel = "";
        switch(room) {
            case SHOW_STAGE: keyLabel = "1"; break;
            case DINING_HALL: keyLabel = "2"; break;
            case BACKROOM: keyLabel = "3"; break;
            case KITCHEN: keyLabel = "4"; break;
            case RESTROOM: keyLabel = "5"; break;
            case LEFT_HALLWAY: keyLabel = "6"; break;
            case RIGHT_HALLWAY: keyLabel = "7"; break;
            case LEFT_OFFICE: keyLabel = "8"; break;
            case RIGHT_OFFICE: keyLabel = "9"; break;
            default: keyLabel = "?"; break;
        }

        // Teken nummer in het midden van de kamer
        float labelX = rect.x + (rect.w / 2) - 3;
        float labelY = rect.y + (rect.h / 2) - 4;
        SDL_RenderDebugText(sdlRenderer, labelX, labelY, keyLabel);

        // Teken animatronics (zeer kleine dots in de hoeken)
        if (isAnimatronicInRoom(room, bonnie)) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
            SDL_FRect dot = {rect.x + 1, rect.y + 1, 3, 3};
            SDL_RenderFillRect(sdlRenderer, &dot);
        }
        if (isAnimatronicInRoom(room, chica)) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_FRect dot = {rect.x + rect.w - 4, rect.y + 1, 3, 3};
            SDL_RenderFillRect(sdlRenderer, &dot);
        }
        if (isAnimatronicInRoom(room, freddy)) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255);
            SDL_FRect dot = {rect.x + 1, rect.y + rect.h - 4, 3, 3};
            SDL_RenderFillRect(sdlRenderer, &dot);
        }
    }
}

void Renderer::drawCameraView(Room camera, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Zwarte achtergrond voor camera feed
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
    SDL_RenderClear(sdlRenderer);

    // Camera frame/rand
    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 35, 255);
    SDL_FRect cameraFrame = {20, 20, 760, 560};
    SDL_RenderRect(sdlRenderer, &cameraFrame);

    // Teken verschillende kamer views
    switch(camera) {
        case SHOW_STAGE:
            drawShowStageView(bonnie, chica, freddy);
            break;
        case DINING_HALL:
            drawDiningHallView(bonnie, chica, freddy);
            break;
        case BACKROOM:
            drawBackroomView(bonnie, chica, freddy);
            break;
        case KITCHEN:
            drawKitchenView(bonnie, chica, freddy);
            break;
        case RESTROOM:
            drawRestroomView(bonnie, chica, freddy);
            break;
        case LEFT_HALLWAY:
            drawLeftHallwayView(bonnie, chica, freddy);
            break;
        case RIGHT_HALLWAY:
            drawRightHallwayView(bonnie, chica, freddy);
            break;
        case LEFT_OFFICE:
            drawLeftDoorView(bonnie, chica, freddy);
            break;
        case RIGHT_OFFICE:
            drawRightDoorView(bonnie, chica, freddy);
            break;
        default:
            // Fallback: toon "NO SIGNAL"
            SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
            SDL_RenderDebugText(sdlRenderer, 350, 280, "NO SIGNAL");
            break;
    }

    // Camera label bovenaan
    SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255); // Groen CRT effect
    const char* cameraName = getRoomName(camera);
    char cameraLabel[50];
    snprintf(cameraLabel, sizeof(cameraLabel), "CAM: %s", cameraName);
    SDL_RenderDebugText(sdlRenderer, 30, 30, cameraLabel);

    // Timestamp (voor sfeer)
    char timestamp[20];
    Uint64 currentTime = SDL_GetTicks();
    int hours = 12 + ((currentTime / 10000) % 6); // Simuleer 12AM - 6AM
    int minutes = (currentTime / 1000) % 60;
    snprintf(timestamp, sizeof(timestamp), "%02d:%02d AM", hours, minutes);
    SDL_RenderDebugText(sdlRenderer, 680, 30, timestamp);

    // REC indicator (knipperend)
    if ((SDL_GetTicks() / 500) % 2) {
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect recDot = {30, 50, 10, 10};
        SDL_RenderFillRect(sdlRenderer, &recDot);
        SDL_RenderDebugText(sdlRenderer, 45, 48, "REC");
    }

    // Verbeterde hotkey instructies (compacter en duidelijker)
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 200);
    SDL_FRect instructionsBG = {20, 490, 300, 95};
    SDL_RenderFillRect(sdlRenderer, &instructionsBG);
    SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
    SDL_RenderRect(sdlRenderer, &instructionsBG);

    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 498, "CAMERA HOTKEYS:");

    SDL_SetRenderDrawColor(sdlRenderer, 180, 180, 180, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 515, "[1] Stage    [2] Dining   [3] Back");
    SDL_RenderDebugText(sdlRenderer, 30, 530, "[4] Kitchen  [5] Restroom [6] L.Hall");
    SDL_RenderDebugText(sdlRenderer, 30, 545, "[7] R.Hall   [8] L.Door!  [9] R.Door!");

    SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 565, "[S] Close Cameras");

    // Legenda voor animatronics (onder de minimap, rechts)
    float legendX = 600.0f;
    float legendY = 180.0f;

    // Achtergrond voor legenda
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 120);
    SDL_FRect legendBG = {legendX - 5, legendY - 5, 155, 35};
    SDL_RenderFillRect(sdlRenderer, &legendBG);
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 60, 150);
    SDL_RenderRect(sdlRenderer, &legendBG);

    // Bonnie indicator
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
    SDL_FRect bonnieIndicator = {legendX, legendY, 6, 6};
    SDL_RenderFillRect(sdlRenderer, &bonnieIndicator);
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, legendX + 10, legendY - 2, "Bonnie");

    // Chica indicator
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
    SDL_FRect chicaIndicator = {legendX + 70, legendY, 6, 6};
    SDL_RenderFillRect(sdlRenderer, &chicaIndicator);
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, legendX + 80, legendY - 2, "Chica");

    // Freddy indicator
    SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255);
    SDL_FRect freddyIndicator = {legendX, legendY + 15, 6, 6};
    SDL_RenderFillRect(sdlRenderer, &freddyIndicator);
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, legendX + 10, legendY + 13, "Freddy");
}

void Renderer::drawShowStageView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Achtergrond - donker podium
    SDL_SetRenderDrawColor(sdlRenderer, 20, 15, 25, 255);
    SDL_FRect stage = {100, 150, 600, 350};
    SDL_RenderFillRect(sdlRenderer, &stage);

    // Podium vloer
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    SDL_FRect stageFloor = {100, 400, 600, 100};
    SDL_RenderFillRect(sdlRenderer, &stageFloor);

    // Gordijnen (paars)
    SDL_SetRenderDrawColor(sdlRenderer, 60, 20, 80, 255);
    SDL_FRect curtainLeft = {100, 150, 50, 350};
    SDL_FRect curtainRight = {650, 150, 50, 350};
    SDL_RenderFillRect(sdlRenderer, &curtainLeft);
    SDL_RenderFillRect(sdlRenderer, &curtainRight);

    // Spotlight cirkels
    SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 60, 100);
    SDL_FRect spotlight1 = {200, 300, 100, 150};
    SDL_FRect spotlight2 = {350, 300, 100, 150};
    SDL_FRect spotlight3 = {500, 300, 100, 150};
    SDL_RenderFillRect(sdlRenderer, &spotlight1);
    SDL_RenderFillRect(sdlRenderer, &spotlight2);
    SDL_RenderFillRect(sdlRenderer, &spotlight3);

    // Teken animatronics als ze hier zijn
    if (isAnimatronicInRoom(SHOW_STAGE, bonnie)) {
        // Bonnie (links - paars)
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 150, 255);
        SDL_FRect bonnieBody = {220, 320, 60, 120};
        SDL_RenderFillRect(sdlRenderer, &bonnieBody);
        SDL_FRect bonnieHead = {230, 290, 40, 40};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);
        // Oren
        SDL_FRect bonnieEar1 = {225, 270, 15, 25};
        SDL_FRect bonnieEar2 = {260, 270, 15, 25};
        SDL_RenderFillRect(sdlRenderer, &bonnieEar1);
        SDL_RenderFillRect(sdlRenderer, &bonnieEar2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 220, 450, "BONNIE");
    } else {
        // Lege spotlight
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 215, 450, "EMPTY");
    }

    if (isAnimatronicInRoom(SHOW_STAGE, freddy)) {
        // Freddy (midden - bruin/oranje)
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 40, 255);
        SDL_FRect freddyBody = {370, 320, 60, 120};
        SDL_RenderFillRect(sdlRenderer, &freddyBody);
        SDL_FRect freddyHead = {380, 290, 40, 40};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);
        // Hoed
        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect freddyHat = {375, 270, 50, 25};
        SDL_RenderFillRect(sdlRenderer, &freddyHat);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 370, 450, "FREDDY");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 365, 450, "EMPTY");
    }

    if (isAnimatronicInRoom(SHOW_STAGE, chica)) {
        // Chica (rechts - geel)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 50, 255);
        SDL_FRect chicaBody = {520, 320, 60, 120};
        SDL_RenderFillRect(sdlRenderer, &chicaBody);
        SDL_FRect chicaHead = {530, 290, 40, 40};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);
        // Bek (oranje)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect chicaBeak = {540, 310, 20, 15};
        SDL_RenderFillRect(sdlRenderer, &chicaBeak);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 520, 450, "CHICA");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 515, 450, "EMPTY");
    }
}

void Renderer::drawDiningHallView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Dining hall achtergrond
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 15, 255);
    SDL_FRect background = {50, 100, 700, 400};
    SDL_RenderFillRect(sdlRenderer, &background);

    // Tafels en stoelen (simpel)
    SDL_SetRenderDrawColor(sdlRenderer, 40, 30, 20, 255);
    for (int i = 0; i < 3; i++) {
        SDL_FRect table = {100.0f + (i * 200.0f), 250, 120, 80};
        SDL_RenderFillRect(sdlRenderer, &table);
    }

    // Vloer tegels
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    for (int i = 50; i < 750; i += 100) {
        SDL_RenderLine(sdlRenderer, i, 350, i, 500);
    }

    // Animatronic detectie
    bool anyoneHere = false;

    if (isAnimatronicInRoom(DINING_HALL, bonnie)) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 150, 255);
        SDL_FRect bonnieFigure = {150, 200, 80, 150};
        SDL_RenderFillRect(sdlRenderer, &bonnieFigure);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 155, 360, "BONNIE DETECTED");
        anyoneHere = true;
    }

    if (isAnimatronicInRoom(DINING_HALL, chica)) {
        SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 50, 255);
        SDL_FRect chicaFigure = {360, 200, 80, 150};
        SDL_RenderFillRect(sdlRenderer, &chicaFigure);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 365, 360, "CHICA DETECTED");
        anyoneHere = true;
    }

    if (isAnimatronicInRoom(DINING_HALL, freddy)) {
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 40, 255);
        SDL_FRect freddyFigure = {570, 200, 80, 150};
        SDL_RenderFillRect(sdlRenderer, &freddyFigure);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 575, 360, "FREDDY DETECTED");
        anyoneHere = true;
    }

    if (!anyoneHere) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 320, 280, "ROOM CLEAR");
    }
}

void Renderer::drawBackroomView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Donkere opslag ruimte
    SDL_SetRenderDrawColor(sdlRenderer, 15, 12, 10, 255);
    SDL_FRect background = {50, 100, 700, 400};
    SDL_RenderFillRect(sdlRenderer, &background);

    // Dozen en spullen
    SDL_SetRenderDrawColor(sdlRenderer, 60, 50, 40, 255);
    SDL_FRect box1 = {100, 300, 80, 80};
    SDL_FRect box2 = {220, 320, 60, 60};
    SDL_FRect box3 = {600, 280, 100, 100};
    SDL_RenderFillRect(sdlRenderer, &box1);
    SDL_RenderFillRect(sdlRenderer, &box2);
    SDL_RenderFillRect(sdlRenderer, &box3);

    // Check voor Bonnie (zijn favorite plek!)
    if (isAnimatronicInRoom(BACKROOM, bonnie)) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 150, 255);
        SDL_FRect bonnieShape = {380, 200, 90, 180};
        SDL_RenderFillRect(sdlRenderer, &bonnieShape);

        // Gloeiende ogen effect
        SDL_SetRenderDrawColor(sdlRenderer, 255, 50, 50, 255);
        SDL_FRect eye1 = {395, 220, 15, 15};
        SDL_FRect eye2 = {440, 220, 15, 15};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 330, 400, "!!! BONNIE !!!");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 340, 280, "EMPTY");
    }
}

void Renderer::drawKitchenView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Keuken achtergrond
    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 25, 255);
    SDL_FRect background = {50, 100, 700, 400};
    SDL_RenderFillRect(sdlRenderer, &background);

    // Aanrecht/counter
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 55, 255);
    SDL_FRect counter = {100, 350, 600, 80};
    SDL_RenderFillRect(sdlRenderer, &counter);

    // Fornuis
    SDL_SetRenderDrawColor(sdlRenderer, 40, 40, 40, 255);
    SDL_FRect stove = {150, 280, 120, 70};
    SDL_RenderFillRect(sdlRenderer, &stove);

    // Kasten
    SDL_SetRenderDrawColor(sdlRenderer, 50, 45, 35, 255);
    SDL_FRect cabinet1 = {400, 200, 100, 150};
    SDL_FRect cabinet2 = {550, 200, 100, 150};
    SDL_RenderFillRect(sdlRenderer, &cabinet1);
    SDL_RenderFillRect(sdlRenderer, &cabinet2);

    // Chica's favoriete plek
    if (isAnimatronicInRoom(KITCHEN, chica)) {
        SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 50, 255);
        SDL_FRect chicaShape = {350, 200, 80, 150};
        SDL_RenderFillRect(sdlRenderer, &chicaShape);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 310, 370, "!!! CHICA IN KITCHEN !!!");

        // Pizza icon bij Chica
        SDL_SetRenderDrawColor(sdlRenderer, 255, 150, 50, 255);
        SDL_FRect pizza = {380, 360, 30, 30};
        SDL_RenderFillRect(sdlRenderer, &pizza);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 340, 280, "EMPTY");
    }
}

void Renderer::drawRestroomView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Toilet/restroom achtergrond
    SDL_SetRenderDrawColor(sdlRenderer, 20, 25, 25, 255);
    SDL_FRect background = {50, 100, 700, 400};
    SDL_RenderFillRect(sdlRenderer, &background);

    // Tegels (wit-achtig)
    SDL_SetRenderDrawColor(sdlRenderer, 40, 45, 45, 255);
    for (int i = 50; i < 750; i += 80) {
        for (int j = 100; j < 500; j += 80) {
            SDL_FRect tile = {static_cast<float>(i), static_cast<float>(j), 75, 75};
            SDL_RenderRect(sdlRenderer, &tile);
        }
    }

    // Toiletten
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 195, 255);
    SDL_FRect toilet1 = {150, 300, 60, 80};
    SDL_FRect toilet2 = {350, 300, 60, 80};
    SDL_FRect toilet3 = {550, 300, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &toilet1);
    SDL_RenderFillRect(sdlRenderer, &toilet2);
    SDL_RenderFillRect(sdlRenderer, &toilet3);

    // Freddy komt vaak hier
    if (isAnimatronicInRoom(RESTROOM, freddy)) {
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 40, 255);
        SDL_FRect freddyShape = {400, 180, 80, 150};
        SDL_RenderFillRect(sdlRenderer, &freddyShape);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 330, 350, "!!! FREDDY DETECTED !!!");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 340, 280, "CLEAR");
    }
}

void Renderer::drawLeftHallwayView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Lange donkere gang naar links
    SDL_SetRenderDrawColor(sdlRenderer, 10, 8, 12, 255);
    SDL_RenderClear(sdlRenderer);

    // Muren (perspectief)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 18, 255);
    // Linker muur
    SDL_RenderLine(sdlRenderer, 50, 100, 200, 300);
    SDL_RenderLine(sdlRenderer, 200, 300, 200, 500);
    SDL_RenderLine(sdlRenderer, 200, 500, 50, 580);

    // Rechter muur
    SDL_RenderLine(sdlRenderer, 750, 100, 600, 300);
    SDL_RenderLine(sdlRenderer, 600, 300, 600, 500);
    SDL_RenderLine(sdlRenderer, 600, 500, 750, 580);

    // Vloer lijnen
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    for (int i = 0; i < 5; i++) {
        SDL_RenderLine(sdlRenderer, 200, 350 + (i * 30), 600, 350 + (i * 30));
    }

    // Dim licht effect aan het eind van de gang
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 40, 100);
    SDL_FRect dimLight = {300, 350, 200, 150};
    SDL_RenderFillRect(sdlRenderer, &dimLight);

    // Bonnie check (hij gebruikt linker gang!)
    if (isAnimatronicInRoom(LEFT_HALLWAY, bonnie)) {
        // Bonnie silhouet in de gang (dreigend)
        SDL_SetRenderDrawColor(sdlRenderer, 80, 40, 120, 255);
        SDL_FRect bonnieSilhouette = {350, 280, 100, 200};
        SDL_RenderFillRect(sdlRenderer, &bonnieSilhouette);

        // Gloeiende ogen
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {370, 300, 20, 20};
        SDL_FRect eye2 = {410, 300, 20, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 250, 500, "!!! BONNIE APPROACHING !!!");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 320, 400, "HALLWAY CLEAR");
    }
}

void Renderer::drawRightHallwayView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Lange donkere gang naar rechts (mirror van links)
    SDL_SetRenderDrawColor(sdlRenderer, 10, 8, 12, 255);
    SDL_RenderClear(sdlRenderer);

    // Muren (perspectief - gespiegeld)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 18, 255);
    SDL_RenderLine(sdlRenderer, 750, 100, 600, 300);
    SDL_RenderLine(sdlRenderer, 600, 300, 600, 500);
    SDL_RenderLine(sdlRenderer, 600, 500, 750, 580);

    SDL_RenderLine(sdlRenderer, 50, 100, 200, 300);
    SDL_RenderLine(sdlRenderer, 200, 300, 200, 500);
    SDL_RenderLine(sdlRenderer, 200, 500, 50, 580);

    // Vloer
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    for (int i = 0; i < 5; i++) {
        SDL_RenderLine(sdlRenderer, 200, 350 + (i * 30), 600, 350 + (i * 30));
    }

    // Dim licht
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 40, 100);
    SDL_FRect dimLight = {300, 350, 200, 150};
    SDL_RenderFillRect(sdlRenderer, &dimLight);

    // Check voor Chica en Freddy
    bool anyoneThere = false;

    if (isAnimatronicInRoom(RIGHT_HALLWAY, chica)) {
        SDL_SetRenderDrawColor(sdlRenderer, 180, 160, 40, 255);
        SDL_FRect chicaSilhouette = {330, 280, 100, 200};
        SDL_RenderFillRect(sdlRenderer, &chicaSilhouette);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 260, 480, "CHICA");
        anyoneThere = true;
    }

    if (isAnimatronicInRoom(RIGHT_HALLWAY, freddy)) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 60, 30, 255);
        SDL_FRect freddySilhouette = {380, 250, 100, 220};
        SDL_RenderFillRect(sdlRenderer, &freddySilhouette);

        // Gloeiende ogen
        SDL_SetRenderDrawColor(sdlRenderer, 255, 200, 0, 255);
        SDL_FRect eye1 = {400, 280, 20, 20};
        SDL_FRect eye2 = {440, 280, 20, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 260, 500, "FREDDY");
        anyoneThere = true;
    }

    if (anyoneThere) {
        SDL_SetRenderDrawColor(sdlRenderer, 255, 50, 50, 255);
        SDL_RenderDebugText(sdlRenderer, 250, 520, "!!! APPROACHING !!!");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 320, 400, "HALLWAY CLEAR");
    }
}

void Renderer::drawLeftDoorView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Zeer close-up van linker deur
    SDL_SetRenderDrawColor(sdlRenderer, 8, 8, 10, 255);
    SDL_RenderClear(sdlRenderer);

    // Deur frame
    SDL_SetRenderDrawColor(sdlRenderer, 40, 35, 30, 255);
    SDL_FRect doorFrame = {200, 50, 400, 500};
    SDL_RenderFillRect(sdlRenderer, &doorFrame);

    // Als Bonnie hier is = GEVAAR!
    if (isAnimatronicInRoom(LEFT_OFFICE, bonnie)) {
        // Bonnie's gezicht HEEL DICHTBIJ (eng!)
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 150, 255);
        SDL_FRect bonnieFace = {250, 150, 300, 350};
        SDL_RenderFillRect(sdlRenderer, &bonnieFace);

        // Grote gloeiende ogen (zeer dreigend)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {300, 250, 60, 60};
        SDL_FRect eye2 = {440, 250, 60, 60};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupillen
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {320, 270, 20, 20};
        SDL_FRect pupil2 = {460, 270, 20, 20};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Waarschuwing
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 200, 520, "!!! BONNIE AT LEFT DOOR !!!");
        SDL_RenderDebugText(sdlRenderer, 240, 540, "CLOSE THE DOOR NOW!");
    } else {
        // Lege deuropening
        SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
        SDL_FRect opening = {250, 100, 300, 400};
        SDL_RenderFillRect(sdlRenderer, &opening);

        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 320, 300, "DOOR CLEAR");
    }
}

void Renderer::drawRightDoorView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Zeer close-up van rechter deur
    SDL_SetRenderDrawColor(sdlRenderer, 8, 8, 10, 255);
    SDL_RenderClear(sdlRenderer);

    // Deur frame
    SDL_SetRenderDrawColor(sdlRenderer, 40, 35, 30, 255);
    SDL_FRect doorFrame = {200, 50, 400, 500};
    SDL_RenderFillRect(sdlRenderer, &doorFrame);

    bool someoneAtDoor = false;

    // Chica check
    if (isAnimatronicInRoom(RIGHT_OFFICE, chica)) {
        SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 50, 255);
        SDL_FRect chicaFace = {250, 150, 300, 350};
        SDL_RenderFillRect(sdlRenderer, &chicaFace);

        // Grote ogen
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_FRect eye1 = {300, 250, 60, 60};
        SDL_FRect eye2 = {440, 250, 60, 60};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Bek
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beak = {370, 350, 60, 40};
        SDL_RenderFillRect(sdlRenderer, &beak);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 200, 510, "!!! CHICA AT RIGHT DOOR !!!");
        someoneAtDoor = true;
    }

    // Freddy check
    if (isAnimatronicInRoom(RIGHT_OFFICE, freddy)) {
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 40, 255);
        SDL_FRect freddyFace = {250, 120, 300, 380};
        SDL_RenderFillRect(sdlRenderer, &freddyFace);

        // Hoed (boven het gezicht)
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hat = {240, 80, 320, 50};
        SDL_RenderFillRect(sdlRenderer, &hat);

        // Gloeiende ogen
        SDL_SetRenderDrawColor(sdlRenderer, 255, 200, 0, 255);
        SDL_FRect eye1 = {300, 220, 60, 60};
        SDL_FRect eye2 = {440, 220, 60, 60};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 200, 530, "!!! FREDDY AT RIGHT DOOR !!!");
        someoneAtDoor = true;
    }

    if (!someoneAtDoor) {
        SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
        SDL_FRect opening = {250, 100, 300, 400};
        SDL_RenderFillRect(sdlRenderer, &opening);

        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 320, 300, "DOOR CLEAR");
    }

    if (someoneAtDoor) {
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 240, 550, "CLOSE THE DOOR NOW!");
    }
}

