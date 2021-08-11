#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <string.h>
#include <Arduino.h>
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

private:
    void drawBattery();
};
