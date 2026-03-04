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
    // Definieer de positie en grootte van elke kamer op basis van AFSTAND TOT OFFICE
    // Layout: Office onderaan, dichtstbijzijnde kamers direct erboven, verste kamers bovenaan
    std::map<Room, SDL_FRect> roomLayout = {
        // VERST VAN OFFICE (bovenaan) - Startpunten
        {SHOW_STAGE,    {230, 80, 130, 55}},      // Bovenaan centraal - START

        // GEMIDDELDE AFSTAND (midden) - Tweede ring
        {DINING_HALL,   {230, 155, 130, 55}},     // Direct onder Show Stage
        {BACKROOM,      {70, 155, 110, 55}},      // Links van Dining Hall
        {KITCHEN,       {390, 155, 110, 55}},     // Rechts van Dining Hall
        {RESTROOM,      {520, 155, 110, 55}},     // Ver rechts van Dining Hall

        // DICHTBIJ OFFICE (derde ring) - Gangen
        {LEFT_HALLWAY,  {90, 240, 110, 55}},      // Links gang - dichtbij
        {RIGHT_HALLWAY, {410, 240, 110, 55}},     // Rechts gang - dichtbij

        // VOOR DE DEUR (onderste ring) - Gevaarzone!
        {LEFT_OFFICE,   {100, 325, 105, 60}},     // Linker deur - ZEER DICHTBIJ
        {RIGHT_OFFICE,  {405, 325, 105, 60}}      // Rechter deur - ZEER DICHTBIJ
    };

    // === TEKEN VERBINDINGSLIJNEN EERST (toon de routes) ===
    SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 80, 255);

    // Van Show Stage naar Dining Hall (verticaal naar beneden)
    SDL_RenderLine(sdlRenderer, 295, 135, 295, 155);

    // Van Dining Hall naar de drie zijkamers
    SDL_RenderLine(sdlRenderer, 230, 182, 180, 182);  // Naar Backroom
    SDL_RenderLine(sdlRenderer, 360, 182, 445, 182);  // Naar Kitchen
    SDL_RenderLine(sdlRenderer, 360, 182, 575, 182);  // Naar Restroom

    // Van Backroom naar Left Hallway (verticaal)
    SDL_RenderLine(sdlRenderer, 125, 210, 145, 240);

    // Van Kitchen naar Right Hallway (verticaal)
    SDL_RenderLine(sdlRenderer, 445, 210, 465, 240);

    // Van Restroom naar Right Hallway (diagonaal)
    SDL_RenderLine(sdlRenderer, 575, 210, 520, 240);

    // Van Left Hallway naar Left Office (verticaal)
    SDL_RenderLine(sdlRenderer, 145, 295, 152, 325);

    // Van Right Hallway naar Right Office (verticaal)
    SDL_RenderLine(sdlRenderer, 465, 295, 458, 325);

    // Van beide deuren naar het centrale Office
    SDL_SetRenderDrawColor(sdlRenderer, 200, 50, 50, 255); // Rood voor gevaar
    SDL_RenderLine(sdlRenderer, 205, 385, 255, 425);  // Left door naar office
    SDL_RenderLine(sdlRenderer, 405, 385, 355, 425);  // Right door naar office

    // === TEKEN JOUW OFFICE ONDERAAN (DOEL VAN ANIMATRONICS) ===
    SDL_FRect officeSpace = {255, 425, 100, 70}; // Onderaan centraal
    SDL_SetRenderDrawColor(sdlRenderer, 60, 80, 120, 255); // Blauwe tint
    SDL_RenderFillRect(sdlRenderer, &officeSpace);

    // Dikke randen voor nadruk (3 lagen)
    SDL_SetRenderDrawColor(sdlRenderer, 100, 140, 200, 255);
    for (int i = 0; i < 3; i++) {
        SDL_FRect outline = {253.0f - i, 423.0f - i, 104.0f + (i*2), 74.0f + (i*2)};
        SDL_RenderRect(sdlRenderer, &outline);
    }

    // Labels
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 270, 440, "YOUR");
    SDL_RenderDebugText(sdlRenderer, 265, 460, "OFFICE");

    // "YOU ARE HERE" marker
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
    SDL_FRect playerMarker = {298, 475, 14, 14};
    SDL_RenderFillRect(sdlRenderer, &playerMarker);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderDebugText(sdlRenderer, 302, 477, "X");

    // === TEKEN ALLE KAMERS ===
    for (auto const& [room, rect] : roomLayout) {
        // Kleur bepalen op basis van type/status
        if (room == activeCamera) {
            SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 0, 255); // Actieve camera (Heldergroen)
        } else if (room == LEFT_OFFICE || room == RIGHT_OFFICE) {
            SDL_SetRenderDrawColor(sdlRenderer, 180, 30, 30, 255); // Deurposities (Donkerrood!)
        } else if (room == LEFT_HALLWAY || room == RIGHT_HALLWAY) {
            SDL_SetRenderDrawColor(sdlRenderer, 80, 60, 30, 255); // Gangen (Oranje-bruin)
        } else if (room == SHOW_STAGE) {
            SDL_SetRenderDrawColor(sdlRenderer, 60, 50, 80, 255); // Show Stage (Paars tint)
        } else {
            SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255); // Normale kamers (Grijs)
        }
        SDL_RenderFillRect(sdlRenderer, &rect);

        // Rand
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderRect(sdlRenderer, &rect);

        // Kamer naam met custom formatting
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);

        if (room == LEFT_OFFICE) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 12, "LEFT");
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 28, "DOOR");
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, rect.x + 8, rect.y + 5, "!!!");
        } else if (room == RIGHT_OFFICE) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 15, rect.y + 12, "RIGHT");
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 28, "DOOR");
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, rect.x + 8, rect.y + 5, "!!!");
        } else if (room == SHOW_STAGE) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 25, rect.y + 12, "SHOW");
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 28, "STAGE");
        } else if (room == DINING_HALL) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 12, "DINING");
            SDL_RenderDebugText(sdlRenderer, rect.x + 25, rect.y + 28, "HALL");
        } else if (room == LEFT_HALLWAY) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 25, rect.y + 12, "LEFT");
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 28, "HALL");
        } else if (room == RIGHT_HALLWAY) {
            SDL_RenderDebugText(sdlRenderer, rect.x + 20, rect.y + 12, "RIGHT");
            SDL_RenderDebugText(sdlRenderer, rect.x + 25, rect.y + 28, "HALL");
        } else {
            const char* roomName = getRoomName(room);
            SDL_RenderDebugText(sdlRenderer, rect.x + 15, rect.y + 20, roomName);
        }

        // === TEKEN ANIMATRONICS MET LABELS ===
        SDL_FRect botRect;
        botRect.w = 16; botRect.h = 16;

        // Bonnie (Magenta/Paars)
        if (bonnie.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
            botRect.x = rect.x + (rect.w / 2) - 8;
            botRect.y = rect.y + (rect.h / 2) - 8;
            SDL_RenderFillRect(sdlRenderer, &botRect);
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, botRect.x + 4, botRect.y + 3, "B");
        }

        // Chica (Geel)
        if (chica.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            botRect.x = rect.x + (rect.w / 2) + 10;
            botRect.y = rect.y + (rect.h / 2) + 10;
            SDL_RenderFillRect(sdlRenderer, &botRect);
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_RenderDebugText(sdlRenderer, botRect.x + 4, botRect.y + 3, "C");
        }

        // Freddy (Oranje)
        if (freddy.getCurrentRoom() == room) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255);
            botRect.x = rect.x + (rect.w / 2) - 20;
            botRect.y = rect.y + (rect.h / 2) - 20;
            SDL_RenderFillRect(sdlRenderer, &botRect);
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, botRect.x + 4, botRect.y + 3, "F");
        }
    }

    // === WAARSCHUWINGSSYSTEEM ===
    bool dangerAtDoor = false;
    std::string warningText = "";

    if (bonnie.getCurrentRoom() == LEFT_OFFICE) {
        dangerAtDoor = true;
        warningText += "BONNIE AT LEFT DOOR! ";
    }
    if (chica.getCurrentRoom() == RIGHT_OFFICE) {
        dangerAtDoor = true;
        warningText += "CHICA AT RIGHT DOOR! ";
    }
    if (freddy.getCurrentRoom() == RIGHT_OFFICE) {
        dangerAtDoor = true;
        warningText += "FREDDY AT RIGHT DOOR! ";
    }

    if (dangerAtDoor) {
        // Knipperende rode waarschuwing
        int blinkState = (SDL_GetTicks() / 250) % 2;
        if (blinkState) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 200);
            SDL_FRect alertBox = {40, 515, 520, 35};
            SDL_RenderFillRect(sdlRenderer, &alertBox);

            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, 50, 522, "!!! DANGER !!!");
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, 50, 535, warningText.c_str());
        }
    }

    // === LEGENDA (rechtsboven) ===
    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 40, 220);
    SDL_FRect legendBG = {615, 85, 175, 110};
    SDL_RenderFillRect(sdlRenderer, &legendBG);
    SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
    SDL_RenderRect(sdlRenderer, &legendBG);

    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 625, 92, "ANIMATRONICS:");

    // Bonnie
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
    SDL_FRect bonnieBox = {625, 112, 14, 14};
    SDL_RenderFillRect(sdlRenderer, &bonnieBox);
    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 645, 110, "= Bonnie");

    // Chica
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
    SDL_FRect chicaBox = {625, 137, 14, 14};
    SDL_RenderFillRect(sdlRenderer, &chicaBox);
    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 645, 135, "= Chica");

    // Freddy
    SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 0, 255);
    SDL_FRect freddyBox = {625, 162, 14, 14};
    SDL_RenderFillRect(sdlRenderer, &freddyBox);
    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 645, 160, "= Freddy");

    // Distance indicator
    SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
    SDL_RenderDebugText(sdlRenderer, 620, 210, "Top = Far");
    SDL_RenderDebugText(sdlRenderer, 620, 225, "Bottom = Near");
}