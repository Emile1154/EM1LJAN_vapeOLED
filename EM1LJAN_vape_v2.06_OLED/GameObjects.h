#pragma once

#define tubeWidth 18
#define heightWindow 40
#define birdX 2
#define speedGame 0.7
#define g -3
#define birdSize 15
#define birdPosX 0
#include "Button.h"

class Tube{
  public:
    void move();
    float x = 60 + tubeWidth/2;
    int window = 0;
};

class Bird{
  public:
    void fly();
    void move();
    void kill();
    bool isAlive = true; 
    float dy = 0;
    float y = 0.5;
};
