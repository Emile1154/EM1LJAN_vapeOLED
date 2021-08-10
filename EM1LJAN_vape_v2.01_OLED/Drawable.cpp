#include "Drawable.h"
#include "images.h"

#define battery_low 3.0 // нижний порог напряжения акб

#define viewChardge 1  //аналоговый пин, подключающийся к входу +5В у зарядки для лития
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include "vape.h"

void Drawable::menuDraw() {
  static const char *menu_strings[MENU_ITEMS] = { "VOLT", "WATT", "COIL", "GAME", "OFF" }; // массив строк
  display.clearDisplay();
  display.setTextSize(1);
  for(byte i = 0; i < MENU_ITEMS; i++ ) {
    display.setTextColor(SSD1306_WHITE);
    if (i == Vape::selectedMode) {
      display.fillRoundRect(0, i*12+29, 32, 10, 10,SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    }
    display.setCursor(3, i*12+30);
    display.println(menu_strings[i]);
  }
  Drawable::drawBattery();
  display.display();
}

void Drawable::wattDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setTextSize(3);
  display.setCursor(8, 20);
  display.write("W");
  display.setCursor(10, 62);
  if(Vape::watt>9){                //Центрирование числа
    display.setCursor(4, 62);
  }
  display.setTextSize(2);   
  display.println(Vape::watt);
  display.setTextSize(1);
  display.setCursor(0, 90);
  display.println(Vape::maxWatt);
  display.setCursor(13, 90);
  display.println("MAX");
  Drawable::drawBattery();
  display.display();
}

void Drawable::voltDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setTextSize(3);
  display.setCursor(8, 20);
  display.print("U");
  display.setTextSize(2);
  display.setCursor(0, 62);
  display.println(Vape::volts);
  Drawable::drawBattery();
  display.display();
}

void Drawable::coilDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setTextSize(2);
  display.setCursor(4, 20);
  display.print("Om");
  display.setTextSize(1);
  display.setCursor(3, 62);
  display.println(Vape::ohms);
  Drawable::drawBattery();
  display.display();
}

void Drawable::animationDraw(bool b){
  display.clearDisplay();
  display.drawBitmap(0, 0,arr[b], 32, 128,WHITE );   
  display.display();
}

void Drawable::drawLogo(short b) {
  display.clearDisplay();
  display.drawBitmap(0, 50, logoArr[b], 32, 32,WHITE );   
  display.display();
}

void Drawable::lowBattery(){
  display.clearDisplay();
  display.drawBitmap(0, 0,low, 32, 128,WHITE );   
  display.display();
  return;
}

void Drawable::initDisp(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextWrap(false);
  display.clearDisplay();
  display.setRotation(1);
  static uint32_t tmr;
  for(byte j = 0; j<5; j++){
    if(millis() - tmr >= 350) {
      tmr = millis();
      Drawable::drawLogo(j);
    }
  }
  Drawable::drawLogo(0);
}

void Drawable::sleep(){
  display.clearDisplay();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void Drawable::wake(){
  display.ssd1306_command(SSD1306_DISPLAYON);
}

void Drawable::eco(bool b){
  display.dim(b);
}

void Drawable::drawBattery(){
  display.setTextSize(1);
  display.setCursor(3, 103);        //ставим положение
  display.print((float)Vape::currentVoltage/1000); //пишем напряжение на батареи
  display.print("v"); 
  if(analogRead(viewChardge) < 900){     //если на входе зарядки меньше 4,5 В
    Vape::carefulLithium = false;                    // разрешить парить
    display.drawBitmap(7, 112,bat, 19, 10, WHITE);
    byte lenght = map(Vape::currentVoltage,battery_low*1000, 4200,0,15);
    display.fillRoundRect(8, 113, lenght, 8, 1,SSD1306_WHITE);
  }
  else{                           //если на входе зарядки 4.5 и более вольт
    Vape::carefulLithium = true;                 //запрет на парение
    display.drawBitmap(7, 112,charge, 19, 10,WHITE );      // рисовать зарядку
  }
}
