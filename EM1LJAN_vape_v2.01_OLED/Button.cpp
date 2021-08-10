#include "Button.h"
#include "Drawable.h"

long Button::wake_timer = 0;
bool Button::bright = false;

Button::Button(byte pin):_pin(pin){
      pinMode(_pin, INPUT_PULLUP);
}

bool Button::click() {
   if(!digitalRead(_pin) && millis() - tmr > 250) {
      tmr = millis();
      resetTimer();
      return true;
   }
   return false;
}  

bool Button::hold(){
   if(!digitalRead(_pin)){
      resetTimer();
      return true;
   }
   return false;
}
  
void Button::resetTimer(){
   wake_timer = millis(); // обновлять таймер при  нажатии любой кнопки
   bright = true;
}
