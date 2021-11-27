#pragma once
#include "Arduino.h"
class Button{
public:
    Button(byte pin);
    bool click();
    bool hold();
    bool press();
    static uint32_t wake_timer;
    static bool bright;
    static bool cutoff;
private:

    void resetTimer();
    const byte _pin;
    uint32_t tmr;
};
