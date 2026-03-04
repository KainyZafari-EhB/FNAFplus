//
// Created by kainy on 2/14/2026.
//

#ifndef FNAF_ANIMATRONIC_H
#define FNAF_ANIMATRONIC_H
#include <string>
#include <utility>


class Office;

enum Room {
    SHOW_STAGE,
    DINING_HALL,
    BACKROOM,
    KITCHEN,
    RESTROOM,
    LEFT_HALLWAY,
    RIGHT_HALLWAY,
    LEFT_OFFICE,
    RIGHT_OFFICE,
    JUMPSCARE
};

class Animatronic {
    std::string name;
    int aiLevel = 10; // 1-10 | level van hoe snel ze bewegen
    Room currentRoom = SHOW_STAGE; // in welke room zitten ze nu
public:
        Animatronic(std::string name, int level) : name(std::move(name)), aiLevel(level), currentRoom(SHOW_STAGE) {}
    void move(Office& office); // (De logica die bepaalt of ze een kamer verder gaan).
    void moveToNextRoom(Office &office);
    Room getCurrentRoom() const {
        return currentRoom;
    }
};


#endif //FNAF_ANIMATRONIC_H