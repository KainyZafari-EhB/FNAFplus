#include "../include/Renderer.h"
#include "../include/Office.h"
#include <iostream>
#include <map>
#include <random>

// Forward declaration for helper function
const char* getRoomName(Room room);
bool isAnimatronicInRoom(Room room, const Animatronic& animatronic);

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
        drawDoorStatus(office.leftDoorClosed, office.rightDoorClosed, bonnie, chica, freddy);
    } else {
        // We tekenen de camera feed van de geselecteerde kamer
        drawCameraView(office.currentCamera, bonnie, chica, freddy);

        // Teken camera map in de hoek (klein overzicht)
        drawCameraMap(office.currentCamera, bonnie, chica, freddy);

        // Static effect over de camera feed
        drawStatic();
    }

    // Lightweight mood pass: stronger when camera is active / power is low.
    drawLowPowerTint(office.powerLevel);
    drawVignette(office.cameraActive ? 26 : 18);

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

void Renderer::drawUiPanel(const SDL_FRect& rect, Uint8 alpha, bool warning) {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(sdlRenderer, &rect);
    if (warning) {
        SDL_SetRenderDrawColor(sdlRenderer, 180, 40, 40, 220);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 90, 90, 90, 200);
    }
    SDL_RenderRect(sdlRenderer, &rect);
}

void Renderer::drawVignette(Uint8 alpha) {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, alpha);
    SDL_FRect top = {0, 0, 800, 35};
    SDL_FRect bottom = {0, 565, 800, 35};
    SDL_FRect left = {0, 0, 30, 600};
    SDL_FRect right = {770, 0, 30, 600};
    SDL_RenderFillRect(sdlRenderer, &top);
    SDL_RenderFillRect(sdlRenderer, &bottom);
    SDL_RenderFillRect(sdlRenderer, &left);
    SDL_RenderFillRect(sdlRenderer, &right);
}

void Renderer::drawLowPowerTint(float powerLevel) {
    // Stronger red tint as power decreases.
    if (powerLevel >= 45.0f) {
        return;
    }
    float danger = (45.0f - powerLevel) / 45.0f;
    if (danger < 0.0f) danger = 0.0f;
    if (danger > 1.0f) danger = 1.0f;
    Uint8 alpha = static_cast<Uint8>(danger * 90.0f);
    SDL_SetRenderDrawColor(sdlRenderer, 120, 0, 0, alpha);
    SDL_FRect overlay = {0, 0, 800, 600};
    SDL_RenderFillRect(sdlRenderer, &overlay);
}

void Renderer::drawPowerBar(float powerLevel) {
    // Verplaatst naar rechts onder om hotkeys niet te overlappen
    // Achterkant van de balk (leeg)
    SDL_FRect bgRect = { 550, 520, 200, 30 };
    drawUiPanel(bgRect, 175, powerLevel <= 20.0f);

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

void Renderer::drawDoorStatus(bool left, bool right, const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
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

        // === ANIMATRONIC IN LEFT DOORWAY ===
        if (isAnimatronicInRoom(LEFT_OFFICE, bonnie)) {
            // BONNIE lurking in left doorway (SUPER SCARY!)
            float bX = 55.0f;
            float bY = 120.0f;

            // Silhouette in darkness
            SDL_SetRenderDrawColor(sdlRenderer, 90, 50, 140, 255);
            SDL_FRect bonnieBody = {bX, bY + 50, 65, 130};
            SDL_RenderFillRect(sdlRenderer, &bonnieBody);

            SDL_SetRenderDrawColor(sdlRenderer, 100, 60, 150, 255);
            SDL_FRect bonnieHead = {bX + 10, bY, 45, 60};
            SDL_RenderFillRect(sdlRenderer, &bonnieHead);

            // TALL EARS (very recognizable silhouette)
            SDL_SetRenderDrawColor(sdlRenderer, 80, 40, 130, 255);
            SDL_FRect ear1 = {bX + 13, bY - 35, 14, 45};
            SDL_FRect ear2 = {bX + 38, bY - 35, 14, 45};
            SDL_RenderFillRect(sdlRenderer, &ear1);
            SDL_RenderFillRect(sdlRenderer, &ear2);

            // GLOWING RED EYES piercing through darkness
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
            SDL_FRect eye1 = {bX + 20, bY + 20, 10, 15};
            SDL_FRect eye2 = {bX + 35, bY + 20, 10, 15};
            SDL_RenderFillRect(sdlRenderer, &eye1);
            SDL_RenderFillRect(sdlRenderer, &eye2);

            // Eye glow effect
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 80);
            SDL_FRect glow1 = {bX + 17, bY + 17, 16, 21};
            SDL_FRect glow2 = {bX + 32, bY + 17, 16, 21};
            SDL_RenderFillRect(sdlRenderer, &glow1);
            SDL_RenderFillRect(sdlRenderer, &glow2);

            // Pupils staring at you
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_FRect pupil1 = {bX + 23, bY + 24, 4, 7};
            SDL_FRect pupil2 = {bX + 38, bY + 24, 4, 7};
            SDL_RenderFillRect(sdlRenderer, &pupil1);
            SDL_RenderFillRect(sdlRenderer, &pupil2);

            // Guitar silhouette
            SDL_SetRenderDrawColor(sdlRenderer, 150, 30, 30, 255);
            SDL_FRect guitar = {bX - 5, bY + 120, 20, 50};
            SDL_RenderFillRect(sdlRenderer, &guitar);
        }
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

        // === ANIMATRONICS IN RIGHT DOORWAY ===
        // Check for Chica first
        if (isAnimatronicInRoom(RIGHT_OFFICE, chica)) {
            // CHICA lurking in right doorway
            float cX = 680.0f;
            float cY = 120.0f;

            // Silhouette
            SDL_SetRenderDrawColor(sdlRenderer, 180, 160, 60, 255);
            SDL_FRect chicaBody = {cX, cY + 50, 65, 130};
            SDL_RenderFillRect(sdlRenderer, &chicaBody);

            SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 70, 255);
            SDL_FRect chicaHead = {cX + 10, cY, 45, 60};
            SDL_RenderFillRect(sdlRenderer, &chicaHead);

            // BEAK (very recognizable)
            SDL_SetRenderDrawColor(sdlRenderer, 220, 120, 0, 255);
            SDL_FRect beakTop = {cX + 25, cY + 22, 20, 12};
            SDL_FRect beakBottom = {cX + 25, cY + 36, 20, 12};
            SDL_RenderFillRect(sdlRenderer, &beakTop);
            SDL_RenderFillRect(sdlRenderer, &beakBottom);

            // MAGENTA EYES
            SDL_SetRenderDrawColor(sdlRenderer, 180, 0, 180, 255);
            SDL_FRect eye1 = {cX + 18, cY + 18, 10, 15};
            SDL_FRect eye2 = {cX + 37, cY + 18, 10, 15};
            SDL_RenderFillRect(sdlRenderer, &eye1);
            SDL_RenderFillRect(sdlRenderer, &eye2);

            // Pupils staring
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_FRect pupil1 = {cX + 21, cY + 22, 4, 7};
            SDL_FRect pupil2 = {cX + 40, cY + 22, 4, 7};
            SDL_RenderFillRect(sdlRenderer, &pupil1);
            SDL_RenderFillRect(sdlRenderer, &pupil2);

            // Bib
            SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
            SDL_FRect bib = {cX + 22, cY + 50, 22, 18};
            SDL_RenderFillRect(sdlRenderer, &bib);

            // Cupcake in hand
            SDL_SetRenderDrawColor(sdlRenderer, 220, 80, 120, 255);
            SDL_FRect cupcake = {cX + 60, cY + 110, 18, 22};
            SDL_RenderFillRect(sdlRenderer, &cupcake);
        } else if (isAnimatronicInRoom(RIGHT_OFFICE, freddy)) {
            // FREDDY lurking in right doorway
            float fX = 680.0f;
            float fY = 120.0f;

            // Silhouette
            SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 40, 255);
            SDL_FRect freddyBody = {fX, fY + 50, 65, 130};
            SDL_RenderFillRect(sdlRenderer, &freddyBody);

            SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 50, 255);
            SDL_FRect freddyHead = {fX + 10, fY, 45, 60};
            SDL_RenderFillRect(sdlRenderer, &freddyHead);

            // Bear ears
            SDL_SetRenderDrawColor(sdlRenderer, 110, 65, 40, 255);
            SDL_FRect ear1 = {fX + 8, fY - 5, 18, 18};
            SDL_FRect ear2 = {fX + 39, fY - 5, 18, 18};
            SDL_RenderFillRect(sdlRenderer, &ear1);
            SDL_RenderFillRect(sdlRenderer, &ear2);

            // TOP HAT silhouette
            SDL_SetRenderDrawColor(sdlRenderer, 15, 15, 15, 255);
            SDL_FRect hatTop = {fX + 8, fY - 28, 50, 28};
            SDL_FRect hatBrim = {fX + 5, fY - 5, 56, 6};
            SDL_RenderFillRect(sdlRenderer, &hatTop);
            SDL_RenderFillRect(sdlRenderer, &hatBrim);

            // CYAN GLOWING EYES
            SDL_SetRenderDrawColor(sdlRenderer, 0, 150, 220, 255);
            SDL_FRect eye1 = {fX + 18, fY + 20, 10, 15};
            SDL_FRect eye2 = {fX + 37, fY + 20, 10, 15};
            SDL_RenderFillRect(sdlRenderer, &eye1);
            SDL_RenderFillRect(sdlRenderer, &eye2);

            // Eye glow
            SDL_SetRenderDrawColor(sdlRenderer, 0, 150, 220, 80);
            SDL_FRect glow1 = {fX + 15, fY + 17, 16, 21};
            SDL_FRect glow2 = {fX + 34, fY + 17, 16, 21};
            SDL_RenderFillRect(sdlRenderer, &glow1);
            SDL_RenderFillRect(sdlRenderer, &glow2);

            // Pupils
            SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
            SDL_FRect pupil1 = {fX + 21, fY + 24, 4, 7};
            SDL_FRect pupil2 = {fX + 40, fY + 24, 4, 7};
            SDL_RenderFillRect(sdlRenderer, &pupil1);
            SDL_RenderFillRect(sdlRenderer, &pupil2);

            // Bow tie
            SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
            SDL_FRect bowtie = {fX + 23, fY + 60, 20, 12};
            SDL_RenderFillRect(sdlRenderer, &bowtie);

            // Microphone
            SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
            SDL_FRect mic = {fX - 5, fY + 110, 12, 40};
            SDL_RenderFillRect(sdlRenderer, &mic);
        }
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
    drawUiPanel(wekkerBG, 180, remainingSeconds <= 30);

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
    // Toon de voortgang visueel (3 minuten = 180 seconden)
    float progress = (float)remainingSeconds / 180.0f; // 180 sec = 3 min
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

    // Teken kamers (zonder nummer labels - we gebruiken nu pijltjestoetsen)
    for (auto const& [room, rect] : roomLayout) {
        // Bepaal kleur en actieve status
        if (room == activeCamera) {
            SDL_SetRenderDrawColor(sdlRenderer, 0, 200, 0, 255); // Actieve camera (groen)
        } else if (room == LEFT_OFFICE || room == RIGHT_OFFICE) {
            SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255); // Deuren (rood - belangrijk!)
        } else {
            SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 60, 255); // Inactief
        }
        SDL_RenderFillRect(sdlRenderer, &rect);

        // Dikkere rand voor actieve camera
        if (room == activeCamera) {
            SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
            SDL_RenderRect(sdlRenderer, &rect);
            // Extra border voor nadruk
            SDL_FRect innerRect = {rect.x + 1, rect.y + 1, rect.w - 2, rect.h - 2};
            SDL_RenderRect(sdlRenderer, &innerRect);
        } else {
            SDL_SetRenderDrawColor(sdlRenderer, 120, 120, 120, 255);
            SDL_RenderRect(sdlRenderer, &rect);
        }

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

    // Hotkey instructies - ruimtelijke navigatie op minimap
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 200);
    SDL_FRect instructionsBG = {20, 485, 360, 100};
    SDL_RenderFillRect(sdlRenderer, &instructionsBG);
    SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
    SDL_RenderRect(sdlRenderer, &instructionsBG);

    SDL_SetRenderDrawColor(sdlRenderer, 220, 220, 220, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 493, "CAMERA CONTROLS (Use Map!):");

    SDL_SetRenderDrawColor(sdlRenderer, 180, 180, 180, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 510, "[ARROWS] Move on map spatially");
    SDL_RenderDebugText(sdlRenderer, 30, 525, "[Q] Left Door      [E] Right Door");
    SDL_RenderDebugText(sdlRenderer, 30, 540, "[TAB] Quick Check  [SPACE] Stage");
    SDL_RenderDebugText(sdlRenderer, 30, 555, "[S] Close Cameras");

    SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
    SDL_RenderDebugText(sdlRenderer, 30, 573, "Tip: Look at minimap for layout");

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
    // === ACHTERGROND ===
    SDL_SetRenderDrawColor(sdlRenderer, 15, 10, 20, 255);
    SDL_FRect background = {0, 0, 800, 600};
    SDL_RenderFillRect(sdlRenderer, &background);

    // Podium platform
    SDL_SetRenderDrawColor(sdlRenderer, 40, 35, 30, 255);
    SDL_FRect stagePlatform = {80, 380, 640, 180};
    SDL_RenderFillRect(sdlRenderer, &stagePlatform);

    // Podium rand (lichtere kleur)
    SDL_SetRenderDrawColor(sdlRenderer, 60, 50, 40, 255);
    SDL_FRect stageEdge = {80, 380, 640, 15};
    SDL_RenderFillRect(sdlRenderer, &stageEdge);

    // Achterwand
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 30, 255);
    SDL_FRect backWall = {80, 120, 640, 260};
    SDL_RenderFillRect(sdlRenderer, &backWall);

    // Gordijnen (rijke paarse kleur)
    SDL_SetRenderDrawColor(sdlRenderer, 80, 20, 100, 255);
    SDL_FRect curtainLeft = {80, 120, 60, 260};
    SDL_FRect curtainRight = {660, 120, 60, 260};
    SDL_RenderFillRect(sdlRenderer, &curtainLeft);
    SDL_RenderFillRect(sdlRenderer, &curtainRight);

    // Gordijn plooien (donkerder)
    SDL_SetRenderDrawColor(sdlRenderer, 50, 10, 70, 255);
    for (int i = 0; i < 6; i++) {
        SDL_RenderLine(sdlRenderer, 90 + (i * 10), 120, 90 + (i * 10), 380);
        SDL_RenderLine(sdlRenderer, 670 + (i * 10), 120, 670 + (i * 10), 380);
    }

    // "SHOW STAGE" banner bovenaan
    SDL_SetRenderDrawColor(sdlRenderer, 200, 180, 50, 255);
    SDL_FRect banner = {250, 80, 300, 30};
    SDL_RenderFillRect(sdlRenderer, &banner);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderDebugText(sdlRenderer, 310, 87, "*** SHOW STAGE ***");

    // Spotlights (gele cirkels met verloop effect)
    SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 60, 150);
    SDL_FRect spotlight1 = {180, 280, 120, 180};
    SDL_FRect spotlight2 = {340, 280, 120, 180};
    SDL_FRect spotlight3 = {500, 280, 120, 180};
    SDL_RenderFillRect(sdlRenderer, &spotlight1);
    SDL_RenderFillRect(sdlRenderer, &spotlight2);
    SDL_RenderFillRect(sdlRenderer, &spotlight3);

    // Spotlight centers (helderder)
    SDL_SetRenderDrawColor(sdlRenderer, 120, 120, 80, 200);
    SDL_FRect spot1center = {200, 320, 80, 100};
    SDL_FRect spot2center = {360, 320, 80, 100};
    SDL_FRect spot3center = {520, 320, 80, 100};
    SDL_RenderFillRect(sdlRenderer, &spot1center);
    SDL_RenderFillRect(sdlRenderer, &spot2center);
    SDL_RenderFillRect(sdlRenderer, &spot3center);


    // === ANIMATRONICS (verbeterde tekeningen) ===

    // BONNIE (links - paars konijn)
    if (isAnimatronicInRoom(SHOW_STAGE, bonnie)) {
        // Lichaam (paars)
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect bonnieBody = {200, 340, 80, 140};
        SDL_RenderFillRect(sdlRenderer, &bonnieBody);

        // Hoofd (groter, ronder)
        SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 190, 255);
        SDL_FRect bonnieHead = {205, 290, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);

        // Lange konijnenoren (kenmerkend!)
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect ear1 = {210, 250, 18, 50};
        SDL_FRect ear2 = {252, 250, 18, 50};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Binnen oren (roze)
        SDL_SetRenderDrawColor(sdlRenderer, 180, 100, 140, 255);
        SDL_FRect earInner1 = {214, 260, 10, 30};
        SDL_FRect earInner2 = {256, 260, 10, 30};
        SDL_RenderFillRect(sdlRenderer, &earInner1);
        SDL_RenderFillRect(sdlRenderer, &earInner2);

        // Ogen (groot en rood - eng!)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {220, 305, 15, 20};
        SDL_FRect eye2 = {245, 305, 15, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupillen (zwart)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {225, 310, 5, 10};
        SDL_FRect pupil2 = {250, 310, 5, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Gitaar (rood)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255);
        SDL_FRect guitar = {195, 400, 25, 70};
        SDL_RenderFillRect(sdlRenderer, &guitar);

        // Label
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 210, 490, "BONNIE");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 205, 380, "** GONE **");
    }

    // FREDDY (midden - bruin beer met hoed)
    if (isAnimatronicInRoom(SHOW_STAGE, freddy)) {
        // Lichaam (bruin)
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect freddyBody = {360, 340, 80, 140};
        SDL_RenderFillRect(sdlRenderer, &freddyBody);

        // Hoofd (ronder)
        SDL_SetRenderDrawColor(sdlRenderer, 150, 100, 60, 255);
        SDL_FRect freddyHead = {365, 290, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);

        // Ronde berenoren
        SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 45, 255);
        SDL_FRect ear1 = {360, 280, 25, 25};
        SDL_FRect ear2 = {415, 280, 25, 25};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Hoed (zwart - iconisch!)
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {360, 250, 80, 35};
        SDL_FRect hatBrim = {350, 280, 100, 8};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        // Hoed band (rood)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255);
        SDL_FRect hatBand = {360, 276, 80, 6};
        SDL_RenderFillRect(sdlRenderer, &hatBand);

        // Ogen (blauw/cyaan - licht gloeiend)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {380, 305, 15, 20};
        SDL_FRect eye2 = {405, 305, 15, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupillen
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {385, 310, 5, 10};
        SDL_FRect pupil2 = {410, 310, 5, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Strik (zwart)
        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect bowtie = {385, 335, 30, 15};
        SDL_RenderFillRect(sdlRenderer, &bowtie);

        // Microfoon (zilver)
        SDL_SetRenderDrawColor(sdlRenderer, 180, 180, 180, 255);
        SDL_FRect mic = {355, 390, 15, 50};
        SDL_RenderFillRect(sdlRenderer, &mic);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 370, 490, "FREDDY");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 365, 380, "** GONE **");
    }

    // CHICA (rechts - gele kip met cupcake)
    if (isAnimatronicInRoom(SHOW_STAGE, chica)) {
        // Lichaam (geel)
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect chicaBody = {520, 340, 80, 140};
        SDL_RenderFillRect(sdlRenderer, &chicaBody);

        // Hoofd
        SDL_SetRenderDrawColor(sdlRenderer, 230, 210, 90, 255);
        SDL_FRect chicaHead = {525, 290, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);

        // Bek (oranje - kenmerkend!)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {545, 300, 30, 15};
        SDL_FRect beakBottom = {545, 320, 30, 15};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        // Bek detail (donkerder)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 100, 0, 255);
        SDL_RenderLine(sdlRenderer, 545, 315, 575, 315);

        // Ogen (paars/magenta)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {535, 300, 15, 20};
        SDL_FRect eye2 = {570, 300, 15, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupillen
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {540, 305, 5, 10};
        SDL_FRect pupil2 = {575, 305, 5, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Bib (witte slabbetje met "LET'S EAT")
        SDL_SetRenderDrawColor(sdlRenderer, 230, 230, 230, 255);
        SDL_FRect bib = {540, 345, 40, 30};
        SDL_RenderFillRect(sdlRenderer, &bib);
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 542, 355, "EAT!");

        // Cupcake (op een dienblad)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 150, 255);
        SDL_FRect cupcake = {590, 400, 25, 30};
        SDL_RenderFillRect(sdlRenderer, &cupcake);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 100, 255);
        SDL_FRect frosting = {590, 395, 25, 10};
        SDL_RenderFillRect(sdlRenderer, &frosting);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDebugText(sdlRenderer, 530, 490, "CHICA");
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 525, 380, "** GONE **");
    }
}

void Renderer::drawDiningHallView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Achtergrond (restaurant muren)
    SDL_SetRenderDrawColor(sdlRenderer, 35, 30, 25, 255);
    SDL_FRect background = {0, 0, 800, 600};
    SDL_RenderFillRect(sdlRenderer, &background);

    // "DINING HALL" sign bovenaan
    SDL_SetRenderDrawColor(sdlRenderer, 200, 50, 50, 255);
    SDL_FRect sign = {280, 20, 240, 35};
    SDL_RenderFillRect(sdlRenderer, &sign);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 305, 28, "== DINING HALL ==");

    // Vloer (tegels patroon)
    SDL_SetRenderDrawColor(sdlRenderer, 45, 40, 35, 255);
    SDL_FRect floor = {50, 350, 700, 250};
    SDL_RenderFillRect(sdlRenderer, &floor);

    // Tegels lijnen
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    for (int i = 0; i < 8; i++) {
        SDL_RenderLine(sdlRenderer, 50 + (i * 100), 350, 50 + (i * 100), 600);
    }
    for (int j = 0; j < 3; j++) {
        SDL_RenderLine(sdlRenderer, 50, 350 + (j * 100), 750, 350 + (j * 100));
    }

    // Tafels met stoelen (3 tafels)
    for (int t = 0; t < 3; t++) {
        float tableX = 130.0f + (t * 220.0f);

        // Tafel (bruin hout)
        SDL_SetRenderDrawColor(sdlRenderer, 70, 50, 30, 255);
        SDL_FRect table = {tableX, 380, 140, 90};
        SDL_RenderFillRect(sdlRenderer, &table);

        // Tafel rand (lichter)
        SDL_SetRenderDrawColor(sdlRenderer, 90, 65, 40, 255);
        SDL_FRect tableEdge = {tableX, 380, 140, 8};
        SDL_RenderFillRect(sdlRenderer, &tableEdge);

        // Stoelen (4 per tafel)
        SDL_SetRenderDrawColor(sdlRenderer, 50, 35, 20, 255);
        // Linker stoel
        SDL_FRect chair1 = {tableX - 25, 405, 20, 40};
        SDL_RenderFillRect(sdlRenderer, &chair1);
        // Rechter stoel
        SDL_FRect chair2 = {tableX + 145, 405, 20, 40};
        SDL_RenderFillRect(sdlRenderer, &chair2);
        // Boven stoel (achter)
        SDL_FRect chair3 = {tableX + 60, 360, 20, 25};
        SDL_RenderFillRect(sdlRenderer, &chair3);
        // Onder stoel (voor)
        SDL_FRect chair4 = {tableX + 60, 465, 20, 25};
        SDL_RenderFillRect(sdlRenderer, &chair4);

        // Pizza op tafel (sommige tafels)
        if (t == 1) {
            SDL_SetRenderDrawColor(sdlRenderer, 220, 180, 80, 255);
            SDL_FRect pizza = {tableX + 50, 405, 40, 40};
            SDL_RenderFillRect(sdlRenderer, &pizza);
            // Pepperoni
            SDL_SetRenderDrawColor(sdlRenderer, 180, 50, 50, 255);
            SDL_FRect pep1 = {tableX + 58, 413, 8, 8};
            SDL_FRect pep2 = {tableX + 72, 418, 8, 8};
            SDL_FRect pep3 = {tableX + 65, 428, 8, 8};
            SDL_RenderFillRect(sdlRenderer, &pep1);
            SDL_RenderFillRect(sdlRenderer, &pep2);
            SDL_RenderFillRect(sdlRenderer, &pep3);
        }
    }

    // Party decorations (ballonnen/slingers)
    SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 100, 255);
    for (int b = 0; b < 5; b++) {
        SDL_FRect balloon = {100.0f + (b * 150.0f), 100, 20, 30};
        SDL_RenderFillRect(sdlRenderer, &balloon);
        // Touwtje
        SDL_SetRenderDrawColor(sdlRenderer, 200, 200, 200, 255);
        SDL_RenderLine(sdlRenderer, 110 + (b * 150), 130, 110 + (b * 150), 180);
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 255, 255); // Wissel kleur
    }

    // Party banner
    SDL_SetRenderDrawColor(sdlRenderer, 255, 200, 50, 255);
    SDL_RenderLine(sdlRenderer, 80, 150, 720, 170);
    SDL_RenderLine(sdlRenderer, 80, 155, 720, 175);

    // Animatronic detectie (groot en duidelijk)
    bool anyoneHere = false;
    int animatronicCount = 0;
    float startX = 250.0f;

    if (isAnimatronicInRoom(DINING_HALL, bonnie)) {
        // BONNIE - Fully detailed purple rabbit
        float bX = startX + (animatronicCount * 180);

        // Body (purple)
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect bonnieFigure = {bX, 220, 100, 160};
        SDL_RenderFillRect(sdlRenderer, &bonnieFigure);

        // Head
        SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 190, 255);
        SDL_FRect bonnieHead = {bX + 20, 180, 60, 50};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);

        // TALL RABBIT EARS
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect ear1 = {bX + 25, 135, 18, 55};
        SDL_FRect ear2 = {bX + 57, 135, 18, 55};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Inner ears (pink)
        SDL_SetRenderDrawColor(sdlRenderer, 170, 90, 130, 255);
        SDL_FRect earInner1 = {bX + 28, 145, 12, 35};
        SDL_FRect earInner2 = {bX + 60, 145, 12, 35};
        SDL_RenderFillRect(sdlRenderer, &earInner1);
        SDL_RenderFillRect(sdlRenderer, &earInner2);

        // GLOWING RED EYES
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {bX + 32, 195, 14, 20};
        SDL_FRect eye2 = {bX + 54, 195, 14, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 100);
        SDL_FRect glow1 = {bX + 28, 191, 22, 28};
        SDL_FRect glow2 = {bX + 50, 191, 22, 28};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {bX + 36, 200, 6, 10};
        SDL_FRect pupil2 = {bX + 58, 200, 6, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Guitar silhouette
        SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255);
        SDL_FRect guitar = {bX + 5, 310, 25, 60};
        SDL_RenderFillRect(sdlRenderer, &guitar);

        // Warning label
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
        SDL_RenderDebugText(sdlRenderer, bX + 15, 390, "!BONNIE!");

        anyoneHere = true;
        animatronicCount++;
    }

    if (isAnimatronicInRoom(DINING_HALL, chica)) {
        // CHICA - Fully detailed yellow chicken
        float cX = startX + (animatronicCount * 180);

        // Body (yellow)
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect chicaFigure = {cX, 220, 100, 160};
        SDL_RenderFillRect(sdlRenderer, &chicaFigure);

        // Head
        SDL_SetRenderDrawColor(sdlRenderer, 230, 210, 90, 255);
        SDL_FRect chicaHead = {cX + 20, 180, 60, 50};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);

        // ORANGE BEAK (signature feature)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {cX + 38, 195, 24, 14};
        SDL_FRect beakBottom = {cX + 38, 212, 24, 14};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        // Beak separation line
        SDL_SetRenderDrawColor(sdlRenderer, 200, 100, 0, 255);
        SDL_RenderLine(sdlRenderer, cX + 38, 209, cX + 62, 209);

        // MAGENTA EYES
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {cX + 28, 190, 14, 20};
        SDL_FRect eye2 = {cX + 58, 190, 14, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {cX + 32, 195, 6, 10};
        SDL_FRect pupil2 = {cX + 62, 195, 6, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // WHITE BIB with text
        SDL_SetRenderDrawColor(sdlRenderer, 230, 230, 230, 255);
        SDL_FRect bib = {cX + 35, 230, 30, 25};
        SDL_RenderFillRect(sdlRenderer, &bib);
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, cX + 38, 237, "EAT!");

        // Cupcake held in hand
        SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 150, 255);
        SDL_FRect cupcake = {cX + 95, 300, 22, 28};
        SDL_RenderFillRect(sdlRenderer, &cupcake);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 100, 255);
        SDL_FRect frosting = {cX + 95, 296, 22, 8};
        SDL_RenderFillRect(sdlRenderer, &frosting);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, cX + 15, 390, "!CHICA!");

        anyoneHere = true;
        animatronicCount++;
    }

    if (isAnimatronicInRoom(DINING_HALL, freddy)) {
        // FREDDY - Fully detailed brown bear with top hat
        float fX = startX + (animatronicCount * 180);

        // Body (brown)
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect freddyFigure = {fX, 220, 100, 160};
        SDL_RenderFillRect(sdlRenderer, &freddyFigure);

        // Head
        SDL_SetRenderDrawColor(sdlRenderer, 150, 100, 60, 255);
        SDL_FRect freddyHead = {fX + 20, 180, 60, 50};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);

        // Round bear ears
        SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 45, 255);
        SDL_FRect ear1 = {fX + 18, 172, 22, 22};
        SDL_FRect ear2 = {fX + 60, 172, 22, 22};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // TOP HAT (iconic)
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {fX + 15, 145, 70, 32};
        SDL_FRect hatBrim = {fX + 10, 172, 80, 8};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        // Hat band (red)
        SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255);
        SDL_FRect hatBand = {fX + 15, 169, 70, 6};
        SDL_RenderFillRect(sdlRenderer, &hatBand);

        // CYAN GLOWING EYES (Freddy's signature)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {fX + 30, 195, 14, 20};
        SDL_FRect eye2 = {fX + 56, 195, 14, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow effect
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 100);
        SDL_FRect glow1 = {fX + 26, 191, 22, 28};
        SDL_FRect glow2 = {fX + 52, 191, 22, 28};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {fX + 34, 200, 6, 10};
        SDL_FRect pupil2 = {fX + 60, 200, 6, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Snout
        SDL_SetRenderDrawColor(sdlRenderer, 170, 120, 70, 255);
        SDL_FRect snout = {fX + 35, 210, 30, 18};
        SDL_RenderFillRect(sdlRenderer, &snout);

        // Black nose
        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect nose = {fX + 43, 218, 14, 8};
        SDL_RenderFillRect(sdlRenderer, &nose);

        // BOW TIE
        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect bowtie = {fX + 38, 230, 24, 12};
        SDL_RenderFillRect(sdlRenderer, &bowtie);

        // Microphone (silver)
        SDL_SetRenderDrawColor(sdlRenderer, 180, 180, 180, 255);
        SDL_FRect mic = {fX + 8, 280, 12, 50};
        SDL_RenderFillRect(sdlRenderer, &mic);
        SDL_FRect micHead = {fX + 5, 275, 18, 12};
        SDL_RenderFillRect(sdlRenderer, &micHead);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_RenderDebugText(sdlRenderer, fX + 15, 390, "!FREDDY!");

        anyoneHere = true;
        animatronicCount++;
    }

    if (!anyoneHere) {
        // Room clear message (groen = veilig)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 330, 250, "** ROOM CLEAR **");
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 300, 280, "(No animatronics detected)");
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

    // Multiple animatronics support
    bool anyoneHere = false;
    int animatronicCount = 0;
    float startX = 280.0f;

    // Check voor Bonnie (zijn favorite plek!)
    if (isAnimatronicInRoom(BACKROOM, bonnie)) {
        float bX = startX + (animatronicCount * 170);
        anyoneHere = true;

        // BONNIE - lurking in shadows (detailed and scary)
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect bonnieBody = {bX, 220, 90, 160};
        SDL_RenderFillRect(sdlRenderer, &bonnieBody);

        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect bonnieHead = {bX + 10, 170, 70, 65};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);

        // RABBIT EARS
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 160, 255);
        SDL_FRect ear1 = {bX + 18, 115, 20, 65};
        SDL_FRect ear2 = {bX + 52, 115, 20, 65};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Inner ears
        SDL_SetRenderDrawColor(sdlRenderer, 160, 80, 120, 255);
        SDL_FRect earInner1 = {bX + 21, 125, 14, 40};
        SDL_FRect earInner2 = {bX + 55, 125, 14, 40};
        SDL_RenderFillRect(sdlRenderer, &earInner1);
        SDL_RenderFillRect(sdlRenderer, &earInner2);

        // GLOWING RED EYES
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {bX + 25, 190, 16, 22};
        SDL_FRect eye2 = {bX + 49, 190, 16, 22};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 120);
        SDL_FRect glow1 = {bX + 21, 186, 24, 30};
        SDL_FRect glow2 = {bX + 45, 186, 24, 30};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {bX + 29, 196, 8, 11};
        SDL_FRect pupil2 = {bX + 53, 196, 8, 11};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Guitar
        SDL_SetRenderDrawColor(sdlRenderer, 180, 40, 40, 255);
        SDL_FRect guitar = {bX - 8, 320, 25, 55};
        SDL_RenderFillRect(sdlRenderer, &guitar);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
            SDL_RenderDebugText(sdlRenderer, bX + 8, 390, "BONNIE!");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(BACKROOM, chica)) {
        float cX = startX + (animatronicCount * 170);
        anyoneHere = true;

        // CHICA - lurking
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect chicaBody = {cX, 220, 90, 160};
        SDL_RenderFillRect(sdlRenderer, &chicaBody);

        SDL_SetRenderDrawColor(sdlRenderer, 230, 210, 90, 255);
        SDL_FRect chicaHead = {cX + 10, 170, 70, 65};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);

        // BEAK
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {cX + 33, 190, 24, 14};
        SDL_FRect beakBottom = {cX + 33, 207, 24, 14};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        // EYES
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {cX + 23, 185, 14, 20};
        SDL_FRect eye2 = {cX + 53, 185, 14, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {cX + 27, 190, 6, 10};
        SDL_FRect pupil2 = {cX + 57, 190, 6, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, cX + 8, 390, "CHICA!");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(BACKROOM, freddy)) {
        float fX = startX + (animatronicCount * 170);
        anyoneHere = true;

        // FREDDY - lurking
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect freddyBody = {fX, 220, 90, 160};
        SDL_RenderFillRect(sdlRenderer, &freddyBody);

        SDL_SetRenderDrawColor(sdlRenderer, 150, 100, 60, 255);
        SDL_FRect freddyHead = {fX + 10, 170, 70, 65};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);

        // HAT
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {fX + 8, 140, 74, 35};
        SDL_FRect hatBrim = {fX + 3, 170, 84, 8};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        // EYES
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {fX + 25, 190, 14, 20};
        SDL_FRect eye2 = {fX + 51, 190, 14, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 100);
        SDL_FRect glow1 = {fX + 21, 186, 22, 28};
        SDL_FRect glow2 = {fX + 47, 186, 22, 28};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {fX + 29, 195, 6, 10};
        SDL_FRect pupil2 = {fX + 55, 195, 6, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
            SDL_RenderDebugText(sdlRenderer, fX + 6, 390, "FREDDY!");
        }
        animatronicCount++;
    }

    if (!anyoneHere) {
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

    // Multiple animatronics support
    bool anyoneHere = false;
    int animatronicCount = 0;
    float startX = 270.0f;

    if (isAnimatronicInRoom(KITCHEN, bonnie)) {
        float bX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // BONNIE in kitchen
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect bonnieBody = {bX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &bonnieBody);

        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect bonnieHead = {bX + 8, 165, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);

        // Ears
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 160, 255);
        SDL_FRect ear1 = {bX + 15, 125, 18, 50};
        SDL_FRect ear2 = {bX + 47, 125, 18, 50};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Eyes
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {bX + 22, 182, 14, 18};
        SDL_FRect eye2 = {bX + 44, 182, 14, 18};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        if ((SDL_GetTicks() / 350) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
            SDL_RenderDebugText(sdlRenderer, bX + 5, 370, "BONNIE");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(KITCHEN, chica)) {
        float cX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // CHICA - Full detailed scary chicken
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect chicaBody = {cX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &chicaBody);

        SDL_SetRenderDrawColor(sdlRenderer, 230, 210, 90, 255);
        SDL_FRect chicaHead = {cX + 8, 165, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);

        // BEAK
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {cX + 30, 182, 28, 14};
        SDL_FRect beakBottom = {cX + 30, 199, 28, 14};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        // EYES
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {cX + 20, 177, 14, 18};
        SDL_FRect eye2 = {cX + 46, 177, 14, 18};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {cX + 24, 182, 6, 9};
        SDL_FRect pupil2 = {cX + 50, 182, 6, 9};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Bib
        SDL_SetRenderDrawColor(sdlRenderer, 230, 230, 230, 255);
        SDL_FRect bib = {cX + 28, 225, 30, 22};
        SDL_RenderFillRect(sdlRenderer, &bib);
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, cX + 30, 232, "EAT");

        // Cupcake
        SDL_SetRenderDrawColor(sdlRenderer, 255, 100, 150, 255);
        SDL_FRect cupcake = {cX + 80, 285, 20, 25};
        SDL_RenderFillRect(sdlRenderer, &cupcake);

        if ((SDL_GetTicks() / 350) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, cX + 6, 370, "CHICA");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(KITCHEN, freddy)) {
        float fX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // FREDDY in kitchen
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect freddyBody = {fX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &freddyBody);

        SDL_SetRenderDrawColor(sdlRenderer, 150, 100, 60, 255);
        SDL_FRect freddyHead = {fX + 8, 165, 70, 60};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);

        // Hat
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {fX + 6, 138, 74, 32};
        SDL_FRect hatBrim = {fX + 1, 165, 84, 8};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        // Eyes
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {fX + 22, 182, 14, 18};
        SDL_FRect eye2 = {fX + 44, 182, 14, 18};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {fX + 26, 187, 6, 9};
        SDL_FRect pupil2 = {fX + 48, 187, 6, 9};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        if ((SDL_GetTicks() / 350) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
            SDL_RenderDebugText(sdlRenderer, fX + 3, 370, "FREDDY");
        }
        animatronicCount++;
    }

    if (!anyoneHere) {
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

    // Multiple animatronics support
    bool anyoneHere = false;
    int animatronicCount = 0;
    float startX = 270.0f;

    if (isAnimatronicInRoom(RESTROOM, bonnie)) {
        float bX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // BONNIE in restroom
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect bonnieBody = {bX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &bonnieBody);

        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect bonnieHead = {bX + 8, 155, 70, 70};
        SDL_RenderFillRect(sdlRenderer, &bonnieHead);

        // Ears
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 160, 255);
        SDL_FRect ear1 = {bX + 15, 110, 18, 55};
        SDL_FRect ear2 = {bX + 47, 110, 18, 55};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // Eyes
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {bX + 22, 177, 15, 20};
        SDL_FRect eye2 = {bX + 43, 177, 15, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 100);
        SDL_FRect glow1 = {bX + 18, 173, 23, 28};
        SDL_FRect glow2 = {bX + 39, 173, 23, 28};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 255, 255);
            SDL_RenderDebugText(sdlRenderer, bX + 5, 370, "BONNIE");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(RESTROOM, chica)) {
        float cX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // CHICA in restroom
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect chicaBody = {cX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &chicaBody);

        SDL_SetRenderDrawColor(sdlRenderer, 230, 210, 90, 255);
        SDL_FRect chicaHead = {cX + 8, 155, 70, 70};
        SDL_RenderFillRect(sdlRenderer, &chicaHead);

        // Beak
        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {cX + 30, 177, 24, 14};
        SDL_FRect beakBottom = {cX + 30, 194, 24, 14};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        // Eyes
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {cX + 20, 172, 14, 18};
        SDL_FRect eye2 = {cX + 46, 172, 14, 18};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, cX + 6, 370, "CHICA");
        }
        animatronicCount++;
    }

    if (isAnimatronicInRoom(RESTROOM, freddy)) {
        float fX = startX + (animatronicCount * 160);
        anyoneHere = true;

        // FREDDY - Full detailed scary bear with hat
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect freddyBody = {fX, 210, 85, 150};
        SDL_RenderFillRect(sdlRenderer, &freddyBody);

        SDL_SetRenderDrawColor(sdlRenderer, 150, 100, 60, 255);
        SDL_FRect freddyHead = {fX + 8, 155, 70, 70};
        SDL_RenderFillRect(sdlRenderer, &freddyHead);

        // Round bear ears
        SDL_SetRenderDrawColor(sdlRenderer, 130, 80, 45, 255);
        SDL_FRect ear1 = {fX + 6, 147, 24, 24};
        SDL_FRect ear2 = {fX + 55, 147, 24, 24};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // TOP HAT
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {fX + 6, 118, 74, 35};
        SDL_FRect hatBrim = {fX + 1, 147, 84, 8};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        // Hat band
        SDL_SetRenderDrawColor(sdlRenderer, 200, 40, 40, 255);
        SDL_FRect hatBand = {fX + 6, 144, 74, 6};
        SDL_RenderFillRect(sdlRenderer, &hatBand);

        // CYAN EYES
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {fX + 22, 177, 15, 20};
        SDL_FRect eye2 = {fX + 43, 177, 15, 20};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Eye glow
        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 100);
        SDL_FRect glow1 = {fX + 18, 173, 23, 28};
        SDL_FRect glow2 = {fX + 39, 173, 23, 28};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupils
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {fX + 26, 182, 7, 10};
        SDL_FRect pupil2 = {fX + 47, 182, 7, 10};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Bow tie
        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect bowtie = {fX + 30, 225, 26, 14};
        SDL_RenderFillRect(sdlRenderer, &bowtie);

        if ((SDL_GetTicks() / 400) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
            SDL_RenderDebugText(sdlRenderer, fX + 3, 370, "FREDDY");
        }
        animatronicCount++;
    }

    if (!anyoneHere) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderDebugText(sdlRenderer, 340, 280, "CLEAR");
    }
}

void Renderer::drawLeftHallwayView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    // Zeer donkere gang (horror ambiance)
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
    SDL_RenderClear(sdlRenderer);

    // "LEFT HALLWAY" label bovenaan
    SDL_SetRenderDrawColor(sdlRenderer, 150, 0, 0, 255);
    SDL_FRect label = {280, 15, 240, 30};
    SDL_RenderFillRect(sdlRenderer, &label);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 305, 22, "<< LEFT HALLWAY");

    // Perspectief muren (donker grijs/bruin)
    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    // Linker muur (perspectief naar einde gang)
    for (int i = 0; i < 5; i++) {
        SDL_RenderLine(sdlRenderer, 100 - (i * 10), 150 + (i * 50), 100 - (i * 10), 450 + (i * 30));
    }
    // Rechter muur
    for (int i = 0; i < 5; i++) {
        SDL_RenderLine(sdlRenderer, 700 + (i * 10), 150 + (i * 50), 700 + (i * 10), 450 + (i * 30));
    }

    // Vloer tegels (perspectief)
    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 15, 255);
    for (int i = 0; i < 6; i++) {
        float y = 380.0f + (i * 35.0f);
        float widthStart = 600.0f - (i * 80.0f);
        float xStart = 400.0f - (widthStart / 2);
        SDL_RenderLine(sdlRenderer, xStart, y, xStart + widthStart, y);
    }

    // Plafond lampen (zwak licht aan einde)
    SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 50, 150);
    SDL_FRect dimLight = {300, 250, 200, 180};
    SDL_RenderFillRect(sdlRenderer, &dimLight);

    // Zwakkere lichten (verderop)
    SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 40, 100);
    SDL_FRect light2 = {250, 200, 300, 150};
    SDL_RenderFillRect(sdlRenderer, &light2);

    // "EXIT" teken (groen, vaag zichtbaar)
    SDL_SetRenderDrawColor(sdlRenderer, 0, 100, 0, 255);
    SDL_FRect exitSign = {360, 180, 80, 25};
    SDL_RenderFillRect(sdlRenderer, &exitSign);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 255, 0, 255);
    SDL_RenderDebugText(sdlRenderer, 372, 185, "EXIT");

    // Posters aan de muur (versleten)
    SDL_SetRenderDrawColor(sdlRenderer, 100, 80, 60, 255);
    SDL_FRect poster1 = {120, 250, 60, 80};
    SDL_FRect poster2 = {620, 280, 60, 80};
    SDL_RenderFillRect(sdlRenderer, &poster1);
    SDL_RenderFillRect(sdlRenderer, &poster2);

    // Bonnie check (HIJ GEBRUIKT DEZE GANG!)
    if (isAnimatronicInRoom(LEFT_HALLWAY, bonnie)) {
        // BONNIE SILHOUET - DREIGEND EN GROOT
        SDL_SetRenderDrawColor(sdlRenderer, 100, 50, 150, 255);
        SDL_FRect bonnieSilhouette = {320, 240, 160, 280};
        SDL_RenderFillRect(sdlRenderer, &bonnieSilhouette);

        // Hoofd (groot, dreigend)
        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 160, 255);
        SDL_FRect head = {350, 190, 100, 80};
        SDL_RenderFillRect(sdlRenderer, &head);

        // LANGE OREN (zeer herkenbaar)
        SDL_FRect ear1 = {360, 130, 25, 70};
        SDL_FRect ear2 = {415, 130, 25, 70};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        // GLOEIENDE RODE OGEN (eng!)
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {370, 220, 25, 35};
        SDL_FRect eye2 = {405, 220, 25, 35};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        // Glow effect rond ogen
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 100);
        SDL_FRect glow1 = {365, 215, 35, 45};
        SDL_FRect glow2 = {400, 215, 35, 45};
        SDL_RenderFillRect(sdlRenderer, &glow1);
        SDL_RenderFillRect(sdlRenderer, &glow2);

        // Pupillen (zwart)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {378, 230, 8, 15};
        SDL_FRect pupil2 = {413, 230, 8, 15};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        // Gitaar silhouet (kenmerkend voor Bonnie)
        SDL_SetRenderDrawColor(sdlRenderer, 80, 40, 120, 255);
        SDL_FRect guitar = {310, 400, 40, 100};
        SDL_RenderFillRect(sdlRenderer, &guitar);

        // WAARSCHUWING (knipperend rood)
        if ((SDL_GetTicks() / 300) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
            SDL_FRect warningBG = {200, 520, 400, 60};
            SDL_RenderFillRect(sdlRenderer, &warningBG);

            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, 220, 530, "!!!  BONNIE APPROACHING  !!!");
            SDL_RenderDebugText(sdlRenderer, 250, 550, "CLOSE LEFT DOOR NOW!");
        }
    } else {
        // Lege gang - veilig (voorlopig)
        SDL_SetRenderDrawColor(sdlRenderer, 0, 150, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 310, 520, "== HALLWAY CLEAR ==");
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 270, 545, "(No threats detected)");
    }
}

void Renderer::drawRightDoorView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
    SDL_RenderClear(sdlRenderer);

    SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
    SDL_FRect indicator = {250, 10, 300, 40};
    SDL_RenderFillRect(sdlRenderer, &indicator);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 295, 20, ">> RIGHT DOOR VIEW <<");

    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 55, 255);
    SDL_FRect doorFrame = {150, 80, 500, 480};
    SDL_RenderFillRect(sdlRenderer, &doorFrame);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_FRect opening = {200, 120, 400, 400};
    SDL_RenderFillRect(sdlRenderer, &opening);

    bool threat = false;

    if (isAnimatronicInRoom(RIGHT_OFFICE, chica)) {
        threat = true;
        SDL_SetRenderDrawColor(sdlRenderer, 220, 200, 80, 255);
        SDL_FRect face = {220, 180, 360, 300};
        SDL_RenderFillRect(sdlRenderer, &face);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 140, 0, 255);
        SDL_FRect beakTop = {350, 305, 100, 35};
        SDL_FRect beakBottom = {350, 350, 100, 35};
        SDL_RenderFillRect(sdlRenderer, &beakTop);
        SDL_RenderFillRect(sdlRenderer, &beakBottom);

        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 200, 255);
        SDL_FRect eye1 = {275, 255, 70, 80};
        SDL_FRect eye2 = {455, 255, 70, 80};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {300, 285, 20, 30};
        SDL_FRect pupil2 = {480, 285, 20, 30};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        SDL_SetRenderDrawColor(sdlRenderer, 230, 230, 230, 255);
        SDL_FRect bib = {360, 395, 80, 45};
        SDL_RenderFillRect(sdlRenderer, &bib);
        SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 376, 410, "EAT!");
    }

    if (isAnimatronicInRoom(RIGHT_OFFICE, freddy)) {
        threat = true;
        SDL_SetRenderDrawColor(sdlRenderer, 140, 90, 50, 255);
        SDL_FRect face = {220, 165, 360, 315};
        SDL_RenderFillRect(sdlRenderer, &face);

        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 20, 255);
        SDL_FRect hatTop = {260, 105, 280, 55};
        SDL_FRect hatBrim = {235, 155, 330, 12};
        SDL_RenderFillRect(sdlRenderer, &hatTop);
        SDL_RenderFillRect(sdlRenderer, &hatBrim);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 180, 255, 255);
        SDL_FRect eye1 = {275, 255, 70, 80};
        SDL_FRect eye2 = {455, 255, 70, 80};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {300, 285, 20, 30};
        SDL_FRect pupil2 = {480, 285, 20, 30};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);

        SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_FRect nose = {385, 345, 30, 22};
        SDL_RenderFillRect(sdlRenderer, &nose);
    }

    if (threat) {
        if ((SDL_GetTicks() / 250) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
            SDL_FRect warning = {100, 530, 600, 60};
            SDL_RenderFillRect(sdlRenderer, &warning);
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, 150, 538, "!!! THREAT AT RIGHT DOOR !!!");
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, 220, 558, ">>> CLOSE DOOR NOW! <<<");
        }
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 25, 255);
        SDL_FRect emptyHall = {220, 150, 360, 350};
        SDL_RenderFillRect(sdlRenderer, &emptyHall);
        SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 50, 100);
        SDL_FRect distantLight = {320, 250, 160, 150};
        SDL_RenderFillRect(sdlRenderer, &distantLight);
        SDL_SetRenderDrawColor(sdlRenderer, 0, 200, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 310, 320, "** DOOR CLEAR **");
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 270, 350, "(No threat at this door)");
    }
}

void Renderer::drawRightHallwayView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
    SDL_RenderClear(sdlRenderer);

    SDL_SetRenderDrawColor(sdlRenderer, 150, 0, 0, 255);
    SDL_FRect label = {280, 15, 250, 30};
    SDL_RenderFillRect(sdlRenderer, &label);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 300, 22, "RIGHT HALLWAY >>");

    SDL_SetRenderDrawColor(sdlRenderer, 30, 25, 20, 255);
    for (int i = 0; i < 5; i++) {
        SDL_RenderLine(sdlRenderer, 140 + (i * 14), 150 + (i * 50), 140 + (i * 14), 450 + (i * 30));
        SDL_RenderLine(sdlRenderer, 660 - (i * 14), 150 + (i * 50), 660 - (i * 14), 450 + (i * 30));
    }

    SDL_SetRenderDrawColor(sdlRenderer, 25, 20, 15, 255);
    for (int i = 0; i < 6; i++) {
        float y = 380.0f + (i * 35.0f);
        float widthStart = 600.0f - (i * 80.0f);
        float xStart = 400.0f - (widthStart / 2);
        SDL_RenderLine(sdlRenderer, xStart, y, xStart + widthStart, y);
    }

    SDL_SetRenderDrawColor(sdlRenderer, 80, 80, 50, 140);
    SDL_FRect dimLight = {300, 250, 200, 170};
    SDL_RenderFillRect(sdlRenderer, &dimLight);

    bool threat = isAnimatronicInRoom(RIGHT_HALLWAY, chica) || isAnimatronicInRoom(RIGHT_HALLWAY, freddy);

    if (threat) {
        SDL_SetRenderDrawColor(sdlRenderer, 140, 100, 60, 255);
        SDL_FRect silhouette = {320, 230, 160, 290};
        SDL_RenderFillRect(sdlRenderer, &silhouette);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {355, 270, 28, 35};
        SDL_FRect eye2 = {417, 270, 28, 35};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        if ((SDL_GetTicks() / 300) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
            SDL_FRect warningBG = {200, 520, 400, 60};
            SDL_RenderFillRect(sdlRenderer, &warningBG);
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, 208, 530, "!!! RIGHT HALL THREAT !!!");
            SDL_RenderDebugText(sdlRenderer, 235, 550, "WATCH RIGHT DOOR NOW");
        }
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 150, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 310, 520, "== HALLWAY CLEAR ==");
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 270, 545, "(No threats detected)");
    }
}

void Renderer::drawLeftDoorView(const Animatronic& bonnie, const Animatronic& chica, const Animatronic& freddy) {
    SDL_SetRenderDrawColor(sdlRenderer, 5, 5, 8, 255);
    SDL_RenderClear(sdlRenderer);

    SDL_SetRenderDrawColor(sdlRenderer, 200, 0, 0, 255);
    SDL_FRect indicator = {250, 10, 300, 40};
    SDL_RenderFillRect(sdlRenderer, &indicator);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDebugText(sdlRenderer, 300, 20, "<< LEFT DOOR VIEW >>");

    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 55, 255);
    SDL_FRect doorFrame = {150, 80, 500, 480};
    SDL_RenderFillRect(sdlRenderer, &doorFrame);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_FRect opening = {200, 120, 400, 400};
    SDL_RenderFillRect(sdlRenderer, &opening);

    bool threat = false;

    if (isAnimatronicInRoom(LEFT_OFFICE, bonnie)) {
        threat = true;
        SDL_SetRenderDrawColor(sdlRenderer, 120, 70, 180, 255);
        SDL_FRect face = {220, 170, 360, 310};
        SDL_RenderFillRect(sdlRenderer, &face);

        SDL_SetRenderDrawColor(sdlRenderer, 110, 60, 170, 255);
        SDL_FRect ear1 = {270, 100, 60, 90};
        SDL_FRect ear2 = {470, 100, 60, 90};
        SDL_RenderFillRect(sdlRenderer, &ear1);
        SDL_RenderFillRect(sdlRenderer, &ear2);

        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_FRect eye1 = {275, 255, 70, 80};
        SDL_FRect eye2 = {455, 255, 70, 80};
        SDL_RenderFillRect(sdlRenderer, &eye1);
        SDL_RenderFillRect(sdlRenderer, &eye2);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_FRect pupil1 = {300, 285, 20, 30};
        SDL_FRect pupil2 = {480, 285, 20, 30};
        SDL_RenderFillRect(sdlRenderer, &pupil1);
        SDL_RenderFillRect(sdlRenderer, &pupil2);
    }

    if (threat) {
        if ((SDL_GetTicks() / 250) % 2) {
            SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
            SDL_FRect warning = {100, 530, 600, 60};
            SDL_RenderFillRect(sdlRenderer, &warning);
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 0, 255);
            SDL_RenderDebugText(sdlRenderer, 155, 538, "!!! THREAT AT LEFT DOOR !!!");
            SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
            SDL_RenderDebugText(sdlRenderer, 220, 558, ">>> CLOSE DOOR NOW! <<<");
        }
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 25, 255);
        SDL_FRect emptyHall = {220, 150, 360, 350};
        SDL_RenderFillRect(sdlRenderer, &emptyHall);
        SDL_SetRenderDrawColor(sdlRenderer, 60, 60, 50, 100);
        SDL_FRect distantLight = {320, 250, 160, 150};
        SDL_RenderFillRect(sdlRenderer, &distantLight);
        SDL_SetRenderDrawColor(sdlRenderer, 0, 200, 0, 255);
        SDL_RenderDebugText(sdlRenderer, 310, 320, "** DOOR CLEAR **");
        SDL_SetRenderDrawColor(sdlRenderer, 150, 150, 150, 255);
        SDL_RenderDebugText(sdlRenderer, 270, 350, "(No threat at this door)");
    }
}


