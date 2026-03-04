//
// Created by kainy on 2/14/2026.
//

#ifndef FNAF_OFFICE_H
#define FNAF_OFFICE_H
#include "Animatronic.h"


class Office {
public:
    bool leftDoorClosed = false;
    bool rightDoorClosed = false;
    bool cameraActive = false;
    float powerLevel = 100.0f;          // De totale stroom
    float baseUsage = 0.1f;        // Standaard verbruik (bijv. 0.1% per seconde)
    float usagePerDevice = 0.25f;   // Hoeveel extra stroom elk apparaat kost

    void stroomUitgevallen();
    Room currentCamera = SHOW_STAGE; // Default camera
};


#endif //FNAF_OFFICE_H