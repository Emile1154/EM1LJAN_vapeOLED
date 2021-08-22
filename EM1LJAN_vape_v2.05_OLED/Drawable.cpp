#include "Drawable.h"
#include "Settings.h"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include "vape.h"
#include "images.h"


void Drawable::menuDraw(int arr[], char* strs[] , byte point, bool edit) {
  display.clearDisplay();
  display.setTextSize(1);
  static bool color = false;
  if(!edit){
    color = false;
  }
  static byte tmr = millis();    // not error, so conceived 
  for(byte i = 0; i < arr[5]; i++ ) {
    display.setTextColor(SSD1306_WHITE);
    if (i == point) {
      display.fillRoundRect(0, i*arr[1]+arr[2]-1, 32, arr[3], arr[4], !color); 
      display.setTextColor(color);
      if(edit && millis() - tmr >= 250){
         color = !color;
         tmr = millis();      
      }
    }
    display.setCursor(arr[0], i*arr[1] + arr[2]);
    display.println(strs[i]);
    
    if(arr[5] == 4 && i < 3){
       display.println(Vape::values[i]);
    }
  }
  
  display.setTextColor(SSD1306_WHITE);
  Drawable::drawBattery();
  display.display();
  return;
}

void Drawable::wattDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setCursor(8, 20);
  display.setTextSize(3);
  display.print("W");
  short x = 4;
  if(Vape::watt<10){                //Центрирование числа
    x = 10;
  }
  display.setCursor(x, 52);
  
  display.setTextSize(2);   
  display.println(Vape::watt);
  display.setTextSize(1);
  display.setCursor(1, 72);
  
  display.setCursor(0, 90);
  display.println(Vape::maxWatt);
  display.setCursor(13, 90);
  display.println("MAX");

  display.drawBitmap(2, 70, arrows, 28, 9,WHITE );   
  Drawable::drawBattery();
  display.display();
  return;
}

void Drawable::voltDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setCursor(8, 20);
  display.setTextSize(3);
  display.print("U");
  display.setTextSize(2);
  display.setCursor(0, 52);
  display.println(Vape::volts);
  display.drawBitmap(2, 70, arrows, 28, 9,WHITE );   
  Drawable::drawBattery();
  display.display();
  return;
}

void Drawable::coilDraw(){
  display.clearDisplay();
  display.drawRect(0,44,32,40,WHITE);
  display.setCursor(4, 20);
  display.setTextSize(2);
  display.print("Om");
  display.setTextSize(1);
  display.setCursor(4, 58);
  display.println(Vape::ohms);
  display.drawBitmap(2, 70, arrows, 28, 9,WHITE );   
  Drawable::drawBattery();
  display.display();
  return;
}


void Drawable::infoDraw(int time[]){ 
  display.clearDisplay();
  static const char chars[] = {'H', 'M', 'S'};
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("smokd");
  for(byte i = 0; i < 3; i++){
    display.setCursor(4, 11+i*9);
    display.print(time[i]);
  }
  for(byte i = 0; i < 3; i++){
    display.setCursor(20, 11+i*9);
    display.print(chars[i]);
  }
  display.drawFastHLine(0,38,32,true);
  //prev
  display.setCursor(0,40);
  display.println("prev:");
  display.print(Vape::values[4]/1000.0);
  display.println("s");

  //logo 
  display.drawBitmap(0, 75, logo1, 32, 32,WHITE );   
  //vers info 
  display.setCursor(0,110);
  display.println("v2.05");
  display.display();
  return;
}

void Drawable::animationDraw(bool b){
  display.clearDisplay();
  display.drawBitmap(0, 0,arr[b], 32, 128,WHITE );   
  display.display();
  return;
}

void Drawable::drawLogo(short b) {
  display.clearDisplay();
  display.drawBitmap(0, 50, logoArr[b], 32, 32,WHITE );   
  display.display();
  return;
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
  static uint16_t tmr = millis();
  static byte i = 0;
  static byte count = 0;
  while(count < 3){
    if(i == 5){
      i = 0;
      count++;
    }
    else if(millis() - tmr >= 100) {
      Drawable::drawLogo(i);
      i++;
      tmr = millis();
    }
  }
  Drawable::drawLogo(0);
  return;
}

void Drawable::sleep(){
  display.clearDisplay();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  return;
}

void Drawable::wake(){
  display.clearDisplay();
  display.ssd1306_command(SSD1306_DISPLAYON);
  return;
}

void Drawable::eco(bool b){
  display.dim(b);
  return;
}

void Drawable::setGameObjects(Bird* b , Tube* t){
  bird = b;
  tube = t;
}

void Drawable::drawGame(){
  display.clearDisplay();
  
  Game::direction ? display.drawBitmap(birdPosX,  128*(1 - bird->y), birdUp, 16, 14, SSD1306_WHITE) : display.drawBitmap(birdPosX,  128*(1 - bird->y), birdDown, 14, 15, SSD1306_WHITE);

  display.fillRect(tube->x - tubeWidth/2 + 2, 0, tubeWidth - 4, tube->window, SSD1306_WHITE);
  display.fillRect(tube->x - tubeWidth/2 + 2, tube->window + heightWindow, tubeWidth - 4, 128 -  tube->window - heightWindow, SSD1306_WHITE);
  display.drawFastVLine(tube->x + tubeWidth/2 - 4, 0, tube->window, SSD1306_BLACK);
  display.drawFastVLine(tube->x + tubeWidth/2 - 4, tube->window + heightWindow, 128 - tube->window + heightWindow, SSD1306_BLACK);
  display.drawFastVLine(tube->x + tubeWidth/2 - 6, 0, tube->window, SSD1306_BLACK);
  display.drawFastVLine(tube->x + tubeWidth/2 - 6, tube->window + heightWindow, 128 - tube->window + heightWindow, SSD1306_BLACK);

  display.fillRect(tube->x - tubeWidth/2, tube->window - 4, tubeWidth, 4, SSD1306_WHITE);
  display.fillRect(tube->x - tubeWidth/2, tube->window + heightWindow, tubeWidth, 4, SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);       
  display.setCursor(22, 0);
  display.print(Game::score);
  display.display();
}

void Drawable::drawBattery(){
  display.setTextSize(1);
  display.setCursor(3, 103);        //ставим положение
  display.print((float)Vape::currentVoltage/1000); //пишем напряжение на батареи
  display.print("v"); 


  if(Vape::charge()){
    display.drawBitmap(7, 112,charge, 19, 10,WHITE );      // рисовать зарядку
    return;
  }
  display.drawBitmap(7, 112,bat, 19, 10, WHITE);
  byte lenght = map(Vape::currentVoltage,Vape::values[0]*1000, 4200,0,15);
  display.fillRoundRect(8, 113, lenght, 8, 1,SSD1306_WHITE);
}
