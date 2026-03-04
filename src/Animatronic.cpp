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
        // CHECK 1: Staat de robot al voor de deur van het kantoor? Dan proberen ze binnen te komen!
        if (currentRoom == LEFT_OFFICE) {
            if (office.leftDoorClosed) {
                currentRoom = DINING_HALL; // Teruggejaagd door de dichte deur
                std::cout << "[BEWEGING] " << name << " werd tegengehouden door de LINKER DEUR!" << std::endl;
            } else {
                currentRoom = JUMPSCARE;   // Deur is open - ze komen binnen!
                std::cout << "[GEVAAR] " << name << " IS BINNEN VIA LINKER DEUR! JUMPSCARE!" << std::endl;
            }
            return; // Stop hier
        }

        if (currentRoom == RIGHT_OFFICE) {
            if (office.rightDoorClosed) {
                currentRoom = DINING_HALL; // Teruggejaagd door de dichte deur
                std::cout << "[BEWEGING] " << name << " werd tegengehouden door de RECHTER DEUR!" << std::endl;
            } else {
                currentRoom = JUMPSCARE;   // Deur is open - ze komen binnen!
                std::cout << "[GEVAAR] " << name << " IS BINNEN VIA RECHTER DEUR! JUMPSCARE!" << std::endl;
            }
            return; // Stop hier
        }

        // CHECK 2: Als ze niet voor de deur staan, verplaats ze naar de volgende kamer
        moveToNextRoom(office);
        std::cout << "[BEWEGING] " << name << " is nu in kamer: " << currentRoom << std::endl;
    }
}

void Animatronic::moveToNextRoom(Office& office) {
    if (name == "Bonnie") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = BACKROOM;
        else if (currentRoom == BACKROOM) currentRoom = LEFT_HALLWAY;
        else if (currentRoom == LEFT_HALLWAY) currentRoom = LEFT_OFFICE; // Naar linker kantoor deur!
    }
    else if (name == "Chica") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = KITCHEN;
        else if (currentRoom == KITCHEN) currentRoom = RIGHT_HALLWAY;
        else if (currentRoom == RIGHT_HALLWAY) currentRoom = RIGHT_OFFICE; // Naar rechter kantoor deur!
    }
    else if (name == "Freddy") {
        if (currentRoom == SHOW_STAGE) currentRoom = DINING_HALL;
        else if (currentRoom == DINING_HALL) currentRoom = RESTROOM;
        else if (currentRoom == RESTROOM) currentRoom = RIGHT_HALLWAY;
        else if (currentRoom == RIGHT_HALLWAY) currentRoom = RIGHT_OFFICE; // Naar rechter kantoor deur!
    }
}