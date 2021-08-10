#pragma once
#include "Arduino.h"
class Button{
public:
    Button(byte pin);
    bool click();
    bool hold();
     
    static long wake_timer;
private:
    void resetTimer();
    const byte _pin;
    uint32_t tmr;
};
