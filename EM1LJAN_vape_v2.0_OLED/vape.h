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
    void voltMenu();
    void wattMenu();
    void ohmsMenu();
    void wake_puzzle();
    
    static short selectedMode;
    static float volts, ohms, my_vcc_const;
    static short watt;
    static short currentVoltage;
    static short maxWatt;
    static boolean changeV, changeW , changeOm, wake_up_flag;
private:
    void chose();
    void good_night();
    void smoke();
    void check();
    static void wake_up();
};
