#pragma once
#include <Arduino.h>


class Voltages{
public:
  void currentVoltage();
  void calibration();
  
  
  
private:
    void calculateMaxWatt();
    long readVcc();
    static short bat_vol, bat_old;
};
