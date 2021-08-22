#include "vape.h"

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


float Vape::values[5] = {3.0, 10.0, 30.0, 0.0, 0.0}; //minVoltage, cutoff, sleep - default values , element total smoked time, prev smoked time 
bool Vape::changeSettings = false;

#include "Settings.h"
#include "Game.h"

static const int menu_args1[6] = {3, 12, 15, 10, 10, 7};   // x, y, n, h, radius ,size strArr;
static const int menu_args2[6] = {2, 25, 5,  25, 3,  4};
static const char* menu_strings1[7] = {"VOLT", "WATT", "COIL", "GAME", "TUNE", "INFO", "OFF"};
static const char* menu_strings2[4] = {"min \nvolt", "max \npuff", "sleep \ntime", "\n back"}; 
    
Button fireButton(FIRE);
Button setButton(SET);
Button rightButton(RIGHT);
Button leftButton(LEFT);

static Drawable drawable;
static Voltages voltage;

Game game;

void Vape::smoke(bool vw){
  uint32_t vape_press = millis(); 
  static boolean vape_btt;
	static const float PWM_filter_k = 0.1;
  static short PWM, PWM_f, PWM_old = 800;           // хранит PWM сигнал
  
	while(!Vape::wake_up_flag & !Vape::charge() & !Button::cutoff){
        check();      
        if (millis() - vape_press > values[1] * 1000 || !fireButton.hold()) {  // "таймер затяжки"
          
          digitalWrite(mosfet, 0);
          Button::cutoff = true; // отсечка
          values[3] -= vape_press/1000.0;
          values[4] = millis() - vape_press;       
          vape_press = millis(); 
          values[3] += vape_press/1000.0;
          Externalizable::writeExternal();
          break;
        }
        else if(!vw){                                                   //ВАРИВОЛЬТ
            PWM = (float)volts*1000 / currentVoltage * 1024;                  // считаем значение для ШИМ сигнала 
        }                                                              
        else if(vw){                                                //ВАРИВАТТ
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
  Externalizable::readExternal();
  voltage.run();
  ptrFireButton = &fireButton;
  ptrSetButton = &setButton;
  game.setButtons(ptrFireButton, ptrSetButton);
  drawable.initDisp();
  Vape::mainMenu();
}

void Vape::mainMenu(){
  static byte selected;
  while(!wake_up_flag) {
     drawable.menuDraw(menu_args1, menu_strings1, selected, false);
     if(rightButton.click()) {
        if(selected == 0){
           selected = MENU_ITEMS;
        }
        selected--;
     }
     if(leftButton.click()) {
        selected++;
        if (selected >= MENU_ITEMS ){
           selected = 0;                         // если мы превысили кол-во элементов вернуться в начало
        }
     }
     if(setButton.click()){
        break;
     }
     Vape::check();  
  }
  if(!Vape::wake_up_flag){
    Vape::chose(selected);
  }
}

void Vape::settings(){
  static byte selected;
  while(!wake_up_flag) {
     drawable.menuDraw(menu_args2, menu_strings2, selected, false);
     if(rightButton.click()) {
        if(selected == 0){
           selected = SIZE;
        }
        selected--;
     }
     if(leftButton.click()) {
        selected++;
        if (selected >= SIZE ){
           selected = 0;                         // если мы превысили кол-во элементов вернуться в начало
        }
     }
     if(setButton.click()){
        break;
     }
     Vape::check();  
  }
  if(!Vape::wake_up_flag){
    Vape::choseValue(selected);
  }
}

void Vape::choseValue(byte selected){
  float min, max, value, step;
  if(selected == 0){  // minVolt
      step = 0.1;
      max = 3.5;
      min = 2.7;
  }
  if(selected == 1){  // cutoff time
      step = 0.5;
      max = 15;
      min = 3;
  }
  if(selected == 2){ // sleep time
      step = 0.5;
      max = 60;
      min = 15;
  }
  while(true){
    Vape::check();
    drawable.menuDraw(menu_args2, menu_strings2, selected, true);
    if(setButton.click() || selected == 3){
      Externalizable::writeExternal();
      break;
    }
    if(rightButton.click()){
      values[selected] += step;              
      values[selected] = min(values[selected], max);  //ограничение сверху
      Vape::changeSettings = true;
    }
    if(leftButton.click()){
      values[selected] -= step;             
      values[selected] = max(values[selected], min);    // ограничение снизу
      Vape::changeSettings = true;
    }
  }
  if(selected == 3){
    Vape::mainMenu();
    return;
  }
  Vape::settings();
}

void Vape::chose(byte selected){
  switch(selected) {
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
       game.createGame();
       break;
     case 4:
       Vape::settings();
       break;
     case 5:
       Vape::info();
       break;
     case 6:
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
      Vape::smoke(false);
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
       Vape::smoke(true);
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
        if (Vape::currentVoltage > values[0] * 1000) {             
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
     if (millis() - Button::wake_timer > values[2] * 1000) {  
         Vape::good_night();
     }
}

void Vape::wake_puzzle() {
    detachInterrupt(0);    // отключить прерывание
    drawable.wake();
    short click_count = 0;
    Button::wake_timer = millis();         // запомнить время пробуждения 
    while(click_count < 5){
      if (fireButton.press()) {
        drawable.drawLogo(click_count);
        click_count++;
      }
      if(millis() - Button::wake_timer > 3500) {    // если 5 нажатий не сделаны за 3.5секунды   
        Vape::good_night();     // дальше спать       
        break;            
      }
    }
    Vape::wake_up_flag = false;      
    Vape::mainMenu(); 
    return;
}

int* Vape::secondsToDate(int seconds){
  int* time = new int[3]; 
  int minutes = seconds / 60;
  seconds %= 60;
  int hours = minutes / 60;
  minutes %= 60;
  time[0] =  hours;
  time[1] = minutes;
  time[2] = seconds;
  return time; 
}

void Vape::info(){
  int* arr = Vape::secondsToDate(values[3]);
  while(!Vape::wake_up_flag){
     Vape::check();
     drawable.infoDraw(arr);
     if(setButton.click()){
        break;
     }
  }
  Vape::mainMenu();
  return;
}

bool Vape::charge(){
  if(analogRead(viewChardge) > 900){     //если на входе зарядки более 4,5 В
    return true;
  }                 
  return false;     
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
