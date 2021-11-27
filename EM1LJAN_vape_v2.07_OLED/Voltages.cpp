#include "Voltages.h"
#include "vape.h"
short Voltages::bat_vol = 0;
short Voltages::bat_old = 0;
#include <EEPROMex.h>   // библиотека для работы со внутренней памятью ардуино
#include "Settings.h"
long Voltages::readVcc(){
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both
  long result = (high << 8) | low;
  #if DCDC_Booster == 1 && initial_calibration == 0
    result = (Vape::my_vcc_const * 1023 * 1000 / result)*analogRead(battery)/1023;//короче говоря U = (battery/1023)*K где K=[4.5В - 5.2В];K = напряжение питания, поскольку питаем от повышайки                                                                       
  #else
    result = Vape::my_vcc_const * 1023 * 1000 / result;
  #endif  
  return result;   
}

void Voltages::currentVoltage(){
  static uint32_t last_time;  
  if (millis() - last_time > 50) {                       // 20 раз в секунду измеряем напряжение
    last_time = millis();
    bat_vol = readVcc();                                 // измерить напряжение аккумулятора в миллиВольтах
    Vape::currentVoltage = 0.04 * bat_vol + (1 - 0.04) * bat_old;  // фильтруем
    bat_old = Vape::currentVoltage;                                // фильтруем
  }
  return;
}

void Voltages::calculateMaxWatt(){
  Vape::maxWatt = (sq((float)Vape::currentVoltage / 1000)) / Vape::ohms; 
  return;
}

void Voltages::run(){
   #if initial_calibration == 1
     Serial.begin(9600);
     delay(500);
     Voltages::calibration();
  #endif
   Voltages::bat_vol = Voltages::readVcc();
   Voltages::bat_old = Voltages::bat_vol;
   return;
}

void Voltages::calibration(){
  for (byte i = 0; i < 7; i++) EEPROM.writeInt(i, 0);          // чистим EEPROM для своих нужд
  Vape::my_vcc_const = 1.1;
  Serial.print("Real VCC is: "); Serial.println(readVcc());     // общаемся с пользователем
  Serial.println("Write your VCC (in millivolts)");
  while (Serial.available() == 0); 
  short Vcc = Serial.parseInt(); // напряжение от пользователя
  float real_const = (float)1.1 * Vcc / readVcc();              // расчёт константы
  Serial.print("New voltage constant: "); Serial.println(real_const, 3);
  EEPROM.writeFloat(8, real_const);                             // запись в EEPROM
  while (1); // уйти в бесконечный цикл
}
