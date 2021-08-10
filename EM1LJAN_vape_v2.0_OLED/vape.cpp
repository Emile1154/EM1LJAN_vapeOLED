
#include "vape.h"
static Vape vape;
short Vape::selectedMode = 0;
float Vape::volts = 0;
float Vape::ohms = 0;
float Vape::my_vcc_const = 0;
short Vape::watt = 0;
short Vape::currentVoltage=  0;
short Vape::maxWatt = 0;
boolean Vape::changeV = false;
boolean Vape::changeW = false;
boolean Vape::changeOm = false;
boolean Vape::wake_up_flag=  false;
#define SET 3   // кнопка выбора
#define RIGHT 4   // кнопка добавить
#define LEFT 5  //кнопка убавить
#define FIRE 2  //кнопка парить ,подкл. прервывание к 2 пину, не менять!

#define battery_low 3.0 // нижний порог напряжения акб
#define vape_threshold 7 // макс. время затяжки в секундах

#define mosfet 10 // timerOne относится только к 10 и 9 пину, не менять!
#define sleep_timer  30  // время в секундах, автоотключение

Button fireButton(FIRE);
Button rightButton(RIGHT);
Button leftButton(LEFT);
Button setButton(SET);

static Drawable drawable;
static Voltages voltage;

void Vape::smoke(){
  static long vape_press; 
  static boolean vape_btt;
	static float PWM_filter_k = 0.1;
  static short PWM, PWM_f, PWM_old = 800;           // хранит PWM сигнал
  Externalizable::writeExternal();
	while(true){      
        vape_press = millis();
        if (millis() - vape_press > vape_threshold * 1000 || !fireButton.hold()) {  // "таймер затяжки"
              digitalWrite(mosfet, 0);
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

void Vape::mainMenu(){
  while(true) {
     drawable.menuDraw();
     if(rightButton.click()) {
        if(selectedMode == 0){
           selectedMode = MENU_ITEMS;
        }
        selectedMode--;
     }
     if(leftButton.click()) {
        selectedMode++;
        if ( selectedMode >= MENU_ITEMS ){
           selectedMode = 0;                         // если мы превысили кол-во элементов вернуться в начало
        }
     }
     if(setButton.click()){
        break;
     }
     check();  
  }
  chose();
}

void Vape::chose(){
  switch(selectedMode) {
     case 0:
       voltMenu();
       break;
     case 1:
       wattMenu();
       break;
     case 2:
       ohmsMenu();
       break;
     case 3:
       break;
     case 4:
       good_night();
       break;
  }
  return;
}

void Vape::voltMenu(){
  while(true){
    check();
    drawable.voltDraw();
    if(rightButton.click()){
      volts += 0.1;                   // увеличить на 0,1В
      volts = min(volts, (float)currentVoltage/1000);  //ограничение сверху
      changeV = true;
    }
    if(leftButton.click()){
      volts -= 0.1;             // уменьшить на 0,1В 
      volts = max(volts, 0);    // ограничение снизу
      changeV = true;
    }
    if(fireButton.hold()){
      smoke();
    }
    if(setButton.click()){
      break;
    }
  }
  mainMenu();
  return;
}

void Vape::wattMenu(){
  while(true){
    check();
    drawable.wattDraw();
    if(rightButton.click()){
       watt++;
       watt = min(watt, maxWatt);   //ограничение сверху по макс мощности
       changeW = true;
    }
    if(leftButton.click()){
       watt--;
       watt = max(watt, 0);     //ограничение снизу
       changeW = true;
    }
    if(fireButton.hold()){
       smoke();
    }
    if(setButton.click()){
       break;
    }
  }
  mainMenu();
  return; 
}

void Vape::ohmsMenu(){
  while(true){
     check();
     drawable.coilDraw();
     if(rightButton.click()){
        ohms += 0.05;
        ohms = min(ohms, 2);
        changeOm = true;
     }
     if(leftButton.click()){
        ohms -= 0.05;
        ohms = max(ohms, 0.05);
        changeOm = true;
     }
     if(setButton.click()){
        break;
     }
  }
  mainMenu();
  return;
}


void Vape::check(){
    while(true){
        voltage.currentVoltage();
        if (currentVoltage > battery_low * 1000) {               // если напряжение меньше минимального
            break;
        }
     	else{
       		Timer1.disablePwm(mosfet);    // принудительно отключить койл
       		digitalWrite(mosfet, LOW);    // принудительно отключить койл   
       		drawable.lowBattery();  // отобразить 
     	}
   }
   if(millis() - Button::wake_timer > 10000) {
         drawable.eco(true);
   }
   if (millis() - Button::wake_timer > sleep_timer * 1000) {  // если кнопки не нажимались дольше чем sleep_timer секунд
         good_night();
   }
   
   return;
}


void Vape::wake_puzzle() {
    detachInterrupt(0);    // отключить прерывание
    static short click_count = 0;
    drawable.sleep(false);  
    while (true) {
        if (fireButton.click()) {
          drawable.drawLogo(click_count);
          click_count++;
          if (click_count > 4) {               // если 5 нажатий сделаны за 3.5 секунды                               
            break;
          }
        }
    	  if (millis() - Button::wake_timer > 3500) {    // если 5 нажатий не сделаны за 3.5секунды
          good_night();     // дальше спать                   
        }
    }
    wake_up_flag = 0;        
    mainMenu();
}


void Vape::wake_up() {
    digitalWrite(10, LOW);    // принудительно отключить койл
    Button::wake_timer = millis();         // запомнить время пробуждения
    wake_up_flag = 1;
    //if (wake_up_flag) vape.wake_puzzle(); 
}
      
//----------------------------метод сна------------------------------------
void Vape::good_night() {
    digitalWrite(mosfet, LOW);    // принудительно отключить койл
    delay(50);
    drawable.sleep(true);  
    delay(50);  
    attachInterrupt(0, wake_up, FALLING);                   // подключить прерывание для пробуждения на 2 пин 
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);    // спать. mode POWER_OFF, АЦП выкл
}
//----------------------------метод сна------------------------------------
