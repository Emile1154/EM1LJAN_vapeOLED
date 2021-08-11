
#include "vape.h"
static Vape vape;
short Vape::selectedMode = 0;
float Vape::volts = 0;
float Vape::ohms = 0;
float Vape::my_vcc_const = 0;
short Vape::watt = 0;
short Vape::currentVoltage=  0;
short Vape::maxWatt = 0;
bool Vape::changeV = false;
bool Vape::changeW = false;
bool Vape::changeOm = false;
bool Vape::wake_up_flag = false;
bool Vape::carefulLithium = false;

#include "Settings.h"

Button fireButton(FIRE);
Button rightButton(RIGHT);
Button leftButton(LEFT);
Button setButton(SET);

static Drawable drawable;
static Voltages voltage;

void Vape::smoke(){
  static uint32_t vape_press; 
  static boolean vape_btt;
  static float PWM_filter_k = 0.1;
  static short PWM, PWM_f, PWM_old = 800;           // хранит PWM сигнал
  Externalizable::writeExternal();
  vape_press = millis();
  while(!Vape::wake_up_flag & !Vape::carefulLithium & !Button::cutoff){
        Vape::check();      
        if (millis() - vape_press > vape_threshold * 1000 || !fireButton.hold()) {  // "таймер затяжки"
              digitalWrite(mosfet, 0);
              Button::cutoff = true; // отсечка
              vape_press = millis(); 
              break;
        }
        else if(selectedMode == 0){                                                   //ВАРИВОЛЬТ
            PWM = (float)volts*1000 / currentVoltage * 1024;                  // считаем значение для ШИМ сигнала 
        }                                                              
        else if(selectedMode == 1){                                                //ВАРИВАТТ
            PWM = (float)watt / maxWatt * 1024;                         // считаем значение для ШИМ сигнала 
        }
        else if (PWM > 1023) {
            PWM = 1023;                                  
        }
        PWM_f = PWM_filter_k * PWM + (1 - PWM_filter_k) * PWM_old;  // фильтруем
        PWM_old = PWM_f;                                           // фильтруем
                        
        Timer1.pwm(mosfet, PWM_f);                               //открываем ключ
        round(millis() / 100) % 2 == 0 ? drawable.animationDraw(true) : drawable.animationDraw(false);   
  }
  return;
}

void Vape::run(){
  voltage.run();
  drawable.initDisp();
  Vape::mainMenu();
}

void Vape::mainMenu(){
  while(!wake_up_flag) {
     drawable.menuDraw();
     if(rightButton.click()) {
        if(Vape::selectedMode == 0){
           Vape::selectedMode = MENU_ITEMS;
        }
        Vape::selectedMode--;
     }
     if(leftButton.click()) {
        Vape::selectedMode++;
        if (Vape::selectedMode >= MENU_ITEMS ){
           Vape::selectedMode = 0;                         // если мы превысили кол-во элементов вернуться в начало
        }
     }
     if(setButton.click()){
        break;
     }
     Vape::check();  
  }
  if(!Vape::wake_up_flag){
    Vape::chose();
  }
}

void Vape::chose(){
  switch(Vape::selectedMode) {
     case 0:
       Vape::voltMenu();
       break;
     case 1:
       Vape::wattMenu();
       break;
     case 2:
       Vape::ohmsMenu();
       break;
     case 3:
        
       break;
     case 4:
       Vape::good_night();
       break; 
  }
  return;
}

void Vape::voltMenu(){
  while(!Vape::wake_up_flag){
    Vape::check();
    drawable.voltDraw();
    if(rightButton.click()){
      Vape::volts += 0.1;                   
      Vape::volts = min(Vape::volts, (float)Vape::currentVoltage/1000);  //ограничение сверху
      Vape::changeV = true;
    }
    if(leftButton.click()){
      Vape::volts -= 0.1;             
      Vape::volts = max(Vape::volts, 0);    // ограничение снизу
      Vape::changeV = true;
    }
    if(fireButton.hold()){
      Vape::smoke();
    }
    if(setButton.click()){
      break;
    }
  }
  Vape::mainMenu();
  return;
}

void Vape::wattMenu(){
  while(!Vape::wake_up_flag){
    Vape::check();
    drawable.wattDraw();
    if(rightButton.click()){
       Vape::watt++;
       Vape::watt = min(Vape::watt, Vape::maxWatt);   //ограничение сверху по макс мощности
       Vape::changeW = true;
    }
    if(leftButton.click()){
       Vape::watt--;
       Vape::watt = max(Vape::watt, 0);     //ограничение снизу
       Vape::changeW = true;
    }
    if(fireButton.hold()){
       Vape::smoke();
    }
    if(setButton.click()){
       break;
    }
  }
  Vape::mainMenu();
  return; 
}

void Vape::ohmsMenu(){
  while(!Vape::wake_up_flag){
     Vape::check();
     drawable.coilDraw();
     if(rightButton.click()){
        Vape::ohms += 0.05;
        Vape::ohms = min(ohms, 2);
        Vape::changeOm = true;
     }
     if(leftButton.click()){
        Vape::ohms -= 0.05;
        Vape::ohms = max(ohms, 0.05);
        Vape::changeOm = true;
     }
     if(setButton.click()){
        break;
     }
  }
  Vape::mainMenu();
  return;
}

void Vape::check(){
     while(true){
        voltage.currentVoltage();
        Voltages::calculateMaxWatt();
        if (Vape::currentVoltage > battery_low * 1000) {               // если напряжение больше минимального
            break;
        }
        digitalWrite(mosfet, LOW);    // принудительно отключить койл   
        drawable.lowBattery();  // отобразить 
     } 
     if(Button::bright){
         drawable.eco(false);
     }
     if(millis() - Button::wake_timer > lowBrightTimer * 1000) {
         Button::bright = false;
         drawable.eco(true);
     }
     if (millis() - Button::wake_timer > sleep_timer * 1000) {  // если кнопки не нажимались дольше чем sleep_timer секунд
         Vape::good_night();
     }
}

void Vape::wake_puzzle() {
    detachInterrupt(0);    // отключить прерывание
    drawable.wake();
    static short click_count = 0;
    Button::wake_timer = millis();         // запомнить время пробуждения 
    while(click_count < 5){
      if (fireButton.click()) {
        drawable.drawLogo(click_count);
        click_count++;
      }
      if(millis() - Button::wake_timer > 3500) {    // если 5 нажатий не сделаны за 3.5секунды   
        Vape::good_night();     // дальше спать                   
      }
    }
    Vape::wake_up_flag = false;      
    click_count = 0;  
    Vape::mainMenu(); 
    return;
}

void Vape::wake_up() {
    digitalWrite(mosfet, LOW);    // принудительно отключить койл
    Vape::wake_up_flag = true;
}
      
void Vape::good_night() {
    digitalWrite(mosfet, LOW);    // принудительно отключить койл
    drawable.sleep();
    delay(50);  
    attachInterrupt(0, wake_up, FALLING);                   // подключить прерывание для пробуждения на 2 пин 
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);    // спать. mode POWER_OFF, АЦП выкл
}
