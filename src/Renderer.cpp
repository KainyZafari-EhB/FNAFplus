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
        SDL_FRect drawer = {157, 205 + (d * 45), 66, 40};
        SDL_RenderRect(sdlRenderer, &drawer);
        // Handvat
        SDL_FRect handle = {185, 220 + (d * 45), 10, 3};
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
        SDL_FRect drawer = {577, 205 + (d * 45), 66, 40};
        SDL_RenderRect(sdlRenderer, &drawer);
        // Handvat
        SDL_FRect handle = {605, 220 + (d * 45), 10, 3};
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
        SDL_FRect textLine = {575, 130 + (line * 12), 50, 2};
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