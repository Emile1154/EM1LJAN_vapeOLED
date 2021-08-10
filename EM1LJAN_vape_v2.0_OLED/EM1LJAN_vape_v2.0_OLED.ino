
//--------------НАСТРОЙКИ--------------------


//--------------ПИНЫ-------------------
//ЦИФРЫ НЕ ДОЛЖНЫ ПОВТОРЯТЬСЯ У ПИНОВ И КНОПОК


//--------------КНОПКИ------------------

#define mosfet 10 // timerOne относится только к 10 и 9 пину, не менять!
#include <TimerOne.h>    // шим
#include "Drawable.h"
#include "vape.h"
#include "Externalizable.h"
Drawable draw;
Vape vape;
#define mosfet 10 // timerOne относится только к 10 и 9 пину, не менять!
#define viewChardge 1  
#define battery 0     //аналоговый пин, подключенный к + батареи

int main(void){
  init();
  #if defined(USBCON)
    USBDevice.attach();
  #endif
  Externalizable::readExternal();
  Timer1.initialize(1500); 
  pinMode(battery, INPUT);    // для замеров напряжений с акб
  pinMode(viewChardge, INPUT);  //     и входа для зарядки
  pinMode(mosfet , OUTPUT);  // выход на ключ
  digitalWrite(mosfet, LOW);
  draw.initDisp();
  delay(300);
  vape.mainMenu();
  if (Vape::wake_up_flag) vape.wake_puzzle();  
  return 0;  
}
