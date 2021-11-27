#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include "Button.h"

uint32_t Button::wake_timer = 0L;
bool Button::bright = false;
bool Button::cutoff = false;

Button::Button(byte pin):_pin(pin){
      pinMode(_pin, INPUT_PULLUP);
}

bool Button::click() {
   if(!digitalRead(_pin) && millis() - Button::tmr > 250) {
      Button::tmr = millis();
      Button::resetTimer();
      return true;
   }
   return false;
}  

bool Button::hold(){
   if(!digitalRead(_pin)){
      Button::resetTimer();
      return true;
   }
   Button::cutoff = false;
   return false;
}

bool Button::press(){
  static uint16_t _tmr;
  static bool flag;
  if(!digitalRead(_pin) && !flag && millis() - _tmr >= 120){
      flag = true;
      _tmr = millis();
      return true;
   }
   else if(digitalRead(_pin) && millis() - _tmr >= 120){
      _tmr = millis();
      flag = false;
   }
   return false;
}
  
void Button::resetTimer(){
   Button::wake_timer = millis(); // обновлять таймер при  нажатии любой кнопки
   Button::bright = true;
}
