#pragma once

#include "Drawable.h"
#include "Voltages.h"
#include "Button.h"
#include "Externalizable.h"
#include "Arduino.h"
#include <LowPower.h>   // библиотека сна
#include <TimerOne.h>    // шим

class Vape{
public:
    void mainMenu();
    void run();
    void wake_puzzle();
    static bool charge();
    void settings();
    void info();
    
    static float volts, ohms, my_vcc_const; //may to array?
    static short watt;
    static short currentVoltage;
    static short maxWatt;
    static bool changeV, changeW , changeOm, changeSettings, wake_up_flag;
    static float values[];
    
    Button* ptrFireButton;
    Button* ptrSetButton;

    void good_night();
private:
    int* secondsToDate(int second);
    void choseValue(byte selected);
    void voltMenu();
    void wattMenu();
    void ohmsMenu();
    void chose(byte selected);
    void smoke(bool vw);
    void check();    
    static void wake_up();

};
