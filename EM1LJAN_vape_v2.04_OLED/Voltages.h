#pragma once
#include <Arduino.h>


class Voltages{
public:
  void currentVoltage();
  static void calculateMaxWatt();
  void calibration();
  void run();
private:
  static short bat_vol, bat_old;
  long readVcc();
};
