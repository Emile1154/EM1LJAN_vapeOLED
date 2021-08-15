#include "Externalizable.h"
#include <EEPROMex.h>   // библиотека для работы со внутренней памятью ардуино
static void Externalizable::readExternal(){
  Vape::volts = EEPROM.readInt(0);
  Vape::watt = EEPROM.readByte(2);
  Vape::ohms = EEPROM.readFloat(4);
  Vape::my_vcc_const = EEPROM.readFloat(8);   
  return;
}


static void Externalizable::writeExternal(){
  // если есть изменения в настройках, записать в память
  if (Vape::changeV) {
    EEPROM.writeInt(0,Vape::volts);
    Vape::changeV = false;
  }
  if (Vape::changeW) {
    EEPROM.writeByte(2,Vape::watt);
    Vape::changeW = false;
  }
  if (Vape::changeOm) {
    EEPROM.writeFloat(4,Vape::ohms);
    Vape::changeOm = false;
  }
  return;        
}
