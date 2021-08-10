#pragma once
#include "Arduino.h"
class Button{
public:
    Button(byte pin);
    bool click();
    bool hold();
    bool pressing();
    static long wake_timer;
    static bool bright;
private:
    void resetTimer();
    const byte _pin;
    uint32_t tmr;
};
