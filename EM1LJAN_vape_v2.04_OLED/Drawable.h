#pragma once
#include <Adafruit_SSD1306.h>
#include "Game.h"
#include "GameObjects.h"

#define MENU_ITEMS 5 // кол-во строк в меню

class Drawable{
public:
    void menuDraw();
    void wattDraw();
    void voltDraw();
    void coilDraw();
    void animationDraw(bool b);
    void drawLogo(short b);
    void lowBattery();
    void initDisp();
    void sleep();
    void wake();
    void eco(bool b);
    void drawGame();
    void setGameObjects(Bird* b , Tube* t);
private:
    Bird* bird;
    Tube* tube;
    void drawBattery();
};
