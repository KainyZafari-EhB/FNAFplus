//
// Created by kainy on 2/14/2026.
//

#include "../include/Animatronic.h"

#include <ctime>
#include <iostream>

#include "random"
#include "../include/Office.h"

void Animatronic::move(Office& office) {
    static std::default_random_engine engine(static_cast<unsigned int>(std::time(0)));
    std::uniform_int_distribution<int> dobbelsteen(1, 20);

    int kans = dobbelsteen(engine);

    // DEBUG: Zo zie je of ze überhaupt een poging doen
    // std::cout << name << " gooit " << kans << " tegen AI " << aiLevel << std::endl;

    if (kans <= aiLevel) {
        // CHECK 1: Staat de robot al in de gang? Dan is de volgende stap het kantoor.
        if (currentRoom == LEFT_HALLWAY) {
            if (office.leftDoorClosed) {
                currentRoom = DINING_HALL; // Teruggejaagd
                std::cout << "[BEWEGING] " << name << " werd tegengehouden door de DEUR!" << std::endl;
            } else {
                currentRoom = JUMPSCARE;   // Binnengeglipt
                std::cout << "[GEVAAR] " << name << " IS BINNEN! JUMPSCARE!" << std::endl;
            }
            return; // Belangrijk: stop hier zodat moveToNextRoom niet ook nog wordt aangeroepen
        }

        if (currentRoom == RIGHT_HALLWAY) {
            if (office.rightDoorClosed) {
                currentRoom = DINING_HALL;
                std::cout << "[BEWEGING] " << name << " werd tegengehouden door de DEUR!" << std::endl;
            } else {
                currentRoom = JUMPSCARE;
                std::cout << "[GEVAAR] " << name << " IS BINNEN! JUMPSCARE!" << std::endl;
            }
            return;
        }

        // CHECK 2: Als ze niet in de gang staan, verplaats ze dan gewoon naar de volgende kamer
        moveToNextRoom(office);
        std::cout << "[BEWEGING] " << name << " is nu in kamer: " << currentRoom << std::endl;
    }
}

void Animatronic::moveToNextRoom(Office& office) {
    if (name == "Bonnie") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = BACKROOM;
        else if (currentRoom == BACKROOM) currentRoom = LEFT_HALLWAY;
        // Bonnie gaat hier nooit naar LEFT_OFFICE,
        // dat doen we in de JUMPSCARE check in move()
    }
    else if (name == "Chica") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = KITCHEN;
        else if (currentRoom == KITCHEN) currentRoom = RIGHT_HALLWAY;
    }
    else if (name == "Freddy") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = RESTROOM;
        else if (currentRoom == RESTROOM) currentRoom = RIGHT_HALLWAY;
    }
}