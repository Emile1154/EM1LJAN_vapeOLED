/*
  ---------------------------------------------------------------------------------------------------------------------------------------------------------------------
    Переделанная прошивка GyverMOD v1.2 под OLED дисплей 128x32 px  
  исходник: GyverMOD https://github.com/AlexGyver/GyverMOD
    При первом запуске initial_calibration должен быть равен 1 (строка №30)
  При подключении и открытии монитора порта будет запущен процесс калибровки.
  Вам нужно при помощи вольтметра измерить напряжение на пинах 5V и GND,
  затем отправить его в монитор В МИЛЛИВОЛЬТАХ, т.е. если на вольтметре 4.56
  то отправить примерно 4560. После этого изменить initial_calibration на 0
  и заново прошить Arduino.
   исправлено:
    -режим вариватт, не забудьте указать сопротивление                                                                                                                                    
    -корректный замер напряжения на батарее если работаем через DC-DC повышайку
   добавлено:
    -анимированная индикация заряда батареи
    -индикация подключенной зарядки на дисплее
    -запрет на парение при зарядке акб
  для тех, кто хочет поставить свою анимацию:  
    -массивы fps1 и fps2 - кадры при нажатии кнопки парения
    -массивы one, three, four, five, seven - кадры при включении, второй и шестой кадр не влез, многа жрет дисп пзу( 
    программа для конвертации изображений в массив:
    https://sourceforge.net/projects/image2code/files/image2code/VERSION-1.1/Image2Code-1.1.zip/download
    подробнее об этом можете узнать здесь:
    https://www.youtube.com/watch?v=S8cbIllYc1I&ab_channel=HomeMade-%D0%A2%D0%BE%2C%D1%87%D1%82%D0%BE%D1%82%D1%8B%D0%BC%D0%BE%D0%B6%D0%B5%D1%88%D1%8C%D1%81%D0%B4%D0%B5%D0%BB%D0%B0%D1%82%D1%8C
  created by EM1LJAN                                             25.04.2021
  ----------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
//-------Библиотеки-----------------------
#include "U8glib.h"       // дисп 
#include <TimerOne.h>    // шим
#include <EEPROMex.h>   // библиотека для работы со внутренней памятью ардуино
#include <LowPower.h>   // библиотека сна
//--------------НАСТРОЙКИ--------------------
#define DCDC_Boster 1  // 0 - не используем, 1 - используем повышайку 
#define sleep_timer  30  // время в секундах, автоотключение
#define initial_calibration 0 // 0 - выкл, 1 - вкл (калибровка)
#define battery_low 2.8 // нижний порог напряжения акб
#define vape_threshold 7 // макс. время затяжки в секундах
//--------------ПИНЫ-------------------
//ЦИФРЫ НЕ ДОЛЖНЫ ПОВТОРЯТЬСЯ У ПИНОВ И КНОПОК
#define vievChardge 1  //аналоговый пин, подключающийся к входу +5В у зарядки для лития
#define battery 0     //аналоговый пин, подключенный к + батареи
#define mosfet 10 // timerOne относится только к 10 и 9 пину, не менять!
//--------------КНОПКИ------------------
#define SET 3   // кнопка выбора
#define UP 4   // кнопка вверх
#define DOWN 5  //кнопка вниз
#define FIRE 2  //кнопка парить ,подкл. прервывание к 2 пину, не менять!
//------------ДЛЯ МЕНЮ----------------------
#define KEY_NONE 0
#define KEY_PREV 1
#define KEY_NEXT 2
#define KEY_SET 3

#define MENU_ITEMS 4 // кол-во строк в меню
const char *menu_strings[MENU_ITEMS] = { "VOLT", "WATT", "COIL", "OFF" }; // массив строк
//------------ДЛЯ МЕНЮ----------------------

//------------------ПЕРЕМЕННЫЕ ИЗ GYVERMOD----------------------
boolean change_v_flag, change_w_flag, change_o_flag;
volatile byte vape_mode, vape_release_count;
volatile byte mode = 3, mode_flag = 1;
short bat_vol, bat_volt_f;   // хранит напряжение на акуме
short PWM, PWM_f;           // хранит PWM сигнал
boolean vape_btt, vape_btt_f;
short bat_old, PWM_old = 800;
uint8_t uiKeyCode = KEY_NONE;

volatile boolean wake_up_flag, vape_flag;
unsigned long last_time, vape_press, set_press, last_vape, wake_timer; 
boolean flag;
float my_vcc_const;

float ohms;
float volts;
byte watt;
byte maxW;
boolean downClick;
boolean upClick;
boolean setClick;
//------------------ПЕРЕМЕННЫЕ ИЗ GYVERMOD----------------------
byte menu_current = 0;
boolean menu_redraw_required = true;
boolean up, down, setB, fire;

boolean n = false;
U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI 
#define u8g_logo_width 32
#define u8g_logo_height 32

//-------------МАССИВЫ ИЗОБРАЖЕНИЙ------------------
static unsigned char one[]  = {
0x00,0x00,0x00,0x00,
0x00,0xF8,0x07,0x00,
0x00,0x0F,0x78,0x00,
0x80,0x02,0x80,0x01,
0xE0,0x80,0x03,0x01,
0x20,0x18,0x33,0x06,
0x30,0x18,0x33,0x08,
0x08,0x18,0x33,0x18,
0x0C,0x10,0x13,0x10,
0x04,0x10,0x13,0x20,
0x02,0x30,0x13,0x60,
0x02,0x30,0x13,0x43,
0x02,0x30,0x1A,0x41,
0x02,0x30,0x18,0x41,
0x01,0x00,0x80,0x81,
0x01,0xF0,0x87,0x81,
0x01,0xF0,0x8F,0x80,
0x11,0xF0,0x1F,0x80,
0x39,0xF0,0x7F,0x40,
0x71,0xF0,0x7F,0x40,
0xE2,0xF9,0x7F,0x40,
0xC6,0xFF,0x7F,0x20,
0x84,0xFF,0x7F,0x20,
0x08,0xFF,0x3F,0x20,
0x08,0xFE,0x3F,0x10,
0x10,0xFC,0x3F,0x08,
0x30,0xE0,0x1F,0x0C,
0x40,0x00,0x1E,0x04,
0xC0,0x00,0x04,0x02,
0x00,0x01,0x00,0x01,
0x00,0x0F,0xE0,0x00,
0x00,0xF8,0x3F,0x00,
};

static unsigned char three[] U8G_PROGMEM = {
  0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x02,0x00,0x00,

0xF0,0x04,0x00,0x00,

0x10,0xA0,0x01,0x00,

0x0C,0x24,0x00,0x00,

0x0A,0x64,0x05,0x00,

0x04,0x4C,0x0A,0x00,

0x02,0x48,0x23,0x00,

0x05,0x88,0x40,0x00,

0x02,0xD8,0x82,0x00,

0x05,0x10,0x93,0x00,

0x01,0x00,0x26,0x01,

0x22,0xF0,0x10,0x00,

0x76,0xF0,0x33,0x02,

0x64,0xE0,0x07,0x04,

0xE2,0xF0,0x2F,0x04,

0xC6,0xF9,0x1F,0x00,

0x8C,0xFF,0x1F,0x04,

0x08,0xFE,0x3F,0x04,

0x18,0xFE,0x3F,0x04,

0x20,0xFC,0x3F,0x04,

0x40,0xF8,0x3F,0x04,

0xA0,0xE0,0x3F,0x0C,

0x40,0xC0,0x3F,0x06,

0x80,0x03,0x3C,0x03,

0x00,0x06,0x10,0x01,

0x00,0x2C,0xA0,0x01,

0x00,0xD0,0x6B,0x00,

0x00,0xA0,0x56,0x00,

0x00,0x80,0x04,0x00,
};
static unsigned char four[] U8G_PROGMEM  = {
  0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x01,0x00,0x00,0x00,

0x06,0x00,0x00,0x00,

0x8A,0x00,0x00,0x00,

0x08,0x00,0x00,0x00,

0x18,0x00,0x00,0x00,

0x20,0x00,0x00,0x00,

0x40,0x04,0x00,0x00,

0xC8,0x18,0x00,0x00,

0x50,0x23,0x00,0x00,

0xA0,0x32,0x00,0x00,

0x00,0xE4,0x01,0x00,

0x40,0xE8,0x00,0x00,

0x80,0xF1,0x01,0x00,

0x00,0xE2,0x0B,0x00,

0x00,0xC4,0x07,0x00,

0x00,0xCC,0x0F,0x00,

0x00,0x88,0x0F,0x00,

0x00,0x10,0x3E,0x00,

0x00,0x60,0xBC,0x00,

0x00,0x80,0x38,0x00,

0x00,0x80,0x61,0x01,

0x00,0x00,0xC2,0x02,

0x00,0x00,0x0C,0x00,

0x00,0x00,0x28,0x02,

0x00,0x00,0xE0,0x06,

0x00,0x00,0xA0,0x05,

0x00,0x00,0x80,0x02,

0x00,0x00,0x00,0x00,
};
static unsigned char five[] U8G_PROGMEM  = {
  0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x50,0x00,0x00,0x00,

0xAC,0x03,0x00,0x00,

0x0C,0x14,0x00,0x00,

0x08,0x4C,0x00,0x00,

0x03,0x3C,0x00,0x00,

0x0A,0xFE,0x00,0x00,

0x05,0xFC,0x01,0x00,

0x02,0xF8,0x0B,0x00,

0x0C,0xF6,0x07,0x00,

0x08,0xE4,0x2F,0x00,

0x08,0xC6,0x13,0x00,

0x8A,0x82,0x23,0x00,

0x18,0x67,0x44,0x00,

0x30,0x40,0x85,0x00,

0x20,0x20,0x09,0x00,

0x60,0x60,0x13,0x02,

0xC0,0x40,0x12,0x00,

0x80,0x41,0x14,0x04,

0x00,0xC3,0x24,0x00,

0x00,0xC6,0x64,0x04,

0x00,0x0C,0x4C,0x04,

0x00,0x30,0x0C,0x04,

0x00,0x60,0x10,0x04,

0x00,0x40,0x01,0x02,

0x00,0x00,0x47,0x03,

0x00,0x00,0xB8,0x00,

0x00,0x00,0x00,0x00,
};
static unsigned char seven[] U8G_PROGMEM = {
  0x00,0x00,0x00,0x00,

0x80,0x0D,0x00,0x00,

0x40,0xF2,0x00,0x00,

0x60,0x80,0x03,0x00,

0x20,0x10,0x0E,0x00,

0x10,0x19,0x10,0x00,

0x10,0x32,0x60,0x00,

0x10,0x22,0xC7,0x00,

0x10,0x64,0x83,0x01,

0x00,0x24,0x02,0x03,

0x20,0x68,0x02,0x04,

0x00,0x48,0x06,0x0C,

0x40,0xD0,0x42,0x08,

0x00,0x11,0x66,0x14,

0x80,0x62,0xC0,0x30,

0x00,0xE4,0xA1,0x20,

0x00,0xE9,0x67,0xA8,

0x00,0xF8,0x27,0x40,

0x00,0xE0,0x5F,0xA0,

0x00,0xC8,0x1F,0xC0,

0x00,0x90,0x3F,0xC0,

0x00,0x40,0x3E,0x40,

0x00,0x80,0x5C,0xC0,

0x00,0x00,0x19,0x90,

0x00,0x00,0x14,0xB4,

0x00,0x00,0x60,0x3D,

0x00,0x00,0x80,0x02,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,
};
static unsigned char fps1[] U8G_PROGMEM = {
  0x02,0x02,0x80,0xC0,

0x12,0x02,0x84,0xC8,

0x12,0x22,0x84,0xC8,

0x12,0x22,0x84,0x08,

0x12,0x20,0x84,0x08,

0x10,0x01,0x80,0x08,

0x10,0x00,0xA6,0x08,

0x12,0x7E,0x87,0x08,

0x12,0xFF,0xA3,0x08,

0x82,0xC1,0xA1,0x48,

0xC2,0xE0,0xA3,0x48,

0x42,0x70,0xA3,0x48,

0xA2,0x39,0xA3,0x48,

0x82,0x1F,0xA3,0x48,

0x82,0x0F,0xA3,0x48,

0x82,0x0F,0x83,0x48,

0x02,0x9F,0x81,0x48,

0x12,0xCE,0x01,0x08,

0x12,0xC4,0x01,0x08,

0x12,0x80,0x07,0x00,

0x10,0x00,0x16,0x80,

0x10,0x00,0x10,0x12,

0x40,0x08,0x10,0x12,

0x08,0x08,0xA2,0x00,

0x08,0x08,0xA2,0x40,

0x08,0x09,0x82,0x40,

0x08,0x01,0xA2,0x40,

0x0A,0x01,0xA2,0x4C,

0x0A,0x05,0xA2,0x48,

0x0A,0x01,0xA2,0x48,

0x0A,0x00,0x82,0x48,

0x0A,0x10,0x82,0x48,

0x0A,0x10,0x82,0x48,

0x0A,0x10,0x02,0x48,

0x0A,0x11,0x12,0x48,

0x08,0x11,0x12,0x48,

0x08,0x11,0x12,0x08,

0x08,0x11,0x10,0x08,

0x00,0x11,0x10,0x08,

0x01,0x11,0x10,0x08,

0x00,0x11,0x10,0x08,

0x00,0x11,0x10,0x09,

0x20,0x10,0x12,0x41,

0x22,0x10,0x10,0x41,

0x22,0x10,0x10,0x01,

0x20,0x00,0x10,0x01,

0x20,0x40,0x10,0x00,

0x20,0x40,0x10,0x20,

0x22,0x41,0x00,0x20,

0x22,0x41,0x00,0x00,

0x22,0x05,0x02,0x09,

0x22,0x00,0x02,0x01,

0x02,0x00,0x02,0x01,

0x02,0x00,0x02,0x21,

0x12,0x00,0x22,0x21,

0x10,0x20,0x22,0xA1,

0x10,0x21,0x22,0x01,

0x10,0x21,0x22,0x01,

0x10,0x21,0x22,0x01,

0x10,0x21,0x02,0x01,

0x12,0x21,0x02,0x81,

0x12,0x21,0x00,0x89,

0x12,0x21,0x00,0xA9,

0x12,0x21,0x00,0xA9,

0x12,0x01,0x20,0xA8,

0x02,0x01,0x20,0xA8,

0x42,0x00,0x20,0xA8,

0x42,0x10,0x20,0xA0,

0x4A,0x00,0x20,0xA0,

0x4A,0x00,0x20,0xA0,

0x4A,0x00,0x26,0xA0,

0x48,0x00,0x06,0xA2,

0x48,0x00,0x0D,0x82,

0x48,0x02,0x0F,0x82,

0x08,0x02,0x0F,0x82,

0x08,0x82,0x1F,0x02,

0x08,0x42,0x1E,0x02,

0x08,0x22,0x1C,0x02,

0x08,0x20,0x7C,0x02,

0x48,0x20,0xFC,0x22,

0x48,0x20,0xFC,0x22,

0x40,0x20,0xFE,0x20,

0x40,0xC0,0xFF,0x28,

0x42,0xC0,0xFD,0xA8,

0x42,0x60,0xFF,0xA8,

0x42,0xF0,0xBF,0xA0,

0x42,0xF0,0xBF,0x80,

0x40,0xF0,0xB7,0x80,

0x40,0xF0,0xBF,0x80,

0x48,0xF0,0xF7,0x80,

0x48,0xD2,0x37,0x80,

0x08,0xD2,0x3F,0x08,

0x08,0xD2,0x1F,0x09,

0x08,0xD2,0x0F,0x21,

0x00,0x12,0x00,0x21,

0x80,0x12,0x03,0x21,

0x00,0x92,0x07,0x21,

0x12,0x80,0x07,0x20,

0x02,0x80,0x47,0x08,

0x02,0x80,0x47,0x08,

0x82,0x84,0x43,0x08,

0x82,0x84,0x43,0x88,

0x92,0x84,0x43,0x08,

0x92,0x04,0x43,0x08,

0x90,0x04,0x43,0x09,

0x10,0x04,0x43,0x09,

0x10,0x04,0x43,0x49,

0x10,0x00,0x01,0x49,

0x00,0x00,0x01,0x40,

0x00,0x00,0x01,0x40,

0x04,0x22,0x00,0x00,

0x04,0x22,0x40,0x00,

0x04,0x22,0x40,0x10,

0x04,0x22,0x44,0x10,

0x04,0x22,0x40,0x10,

0x24,0x22,0x40,0x12,

0x24,0x22,0x40,0x12,

0x24,0x22,0x40,0x12,

0x24,0x02,0x04,0x12,

0x24,0x02,0x04,0x12,

0x24,0x00,0x04,0x02,

0x24,0x00,0x04,0x02,

0x04,0x42,0x04,0x02,

0x00,0x42,0x40,0x02,

0x80,0x42,0x40,0x40,

0x8C,0x00,0x40,0x40,

0xC4,0x00,0x00,0x40,

0x04,0x00,0x00,0x00,
};
static unsigned char fps2[]  = {
  0x00,0x00,0x00,0x00,

0x20,0x00,0x04,0x00,

0x00,0x20,0x04,0x02,

0x00,0x20,0x04,0x02,

0x00,0x21,0x04,0x22,

0x01,0x01,0x00,0x00,

0x21,0x00,0x26,0x00,

0x01,0x7E,0x27,0x00,

0x01,0xFF,0x23,0x00,

0x85,0xC1,0x21,0x41,

0xC4,0xE0,0x23,0x41,

0x44,0x70,0x23,0x41,

0xA4,0x39,0x23,0x00,

0x8C,0x1F,0x23,0x20,

0x8C,0x0F,0x03,0x20,

0x88,0x0F,0x03,0x20,

0x08,0x9F,0x01,0x21,

0x08,0xCE,0x01,0x21,

0x08,0xC4,0x01,0x01,

0x08,0x80,0x07,0x01,

0x08,0x00,0x16,0x04,

0x00,0x00,0x10,0x04,

0x40,0x00,0x10,0x04,

0x40,0x00,0x10,0x04,

0x40,0x04,0x10,0x04,

0x40,0x84,0x10,0x04,

0x44,0x84,0x10,0x44,

0x44,0x84,0x00,0x44,

0x44,0x04,0x80,0x44,

0x44,0x04,0x80,0x40,

0x40,0x04,0x80,0x00,

0x40,0x00,0x00,0x00,

0x40,0x80,0x00,0x02,

0x40,0x80,0x10,0x02,

0x40,0x80,0x00,0x02,

0x04,0x80,0x20,0x22,

0x04,0x80,0x20,0x22,

0x04,0x04,0x20,0x02,

0x04,0x04,0x20,0x00,

0x84,0x00,0x20,0x80,

0x04,0x00,0x00,0x90,

0x00,0x00,0x04,0x90,

0x00,0x20,0x04,0x90,

0x00,0x22,0x04,0x10,

0x00,0x22,0x04,0x10,

0x40,0x22,0x84,0x10,

0x40,0x22,0x84,0x00,

0x40,0x00,0x80,0x00,

0x48,0x00,0x80,0x04,

0x08,0x00,0x00,0x44,

0x00,0x00,0x00,0x44,

0x00,0x00,0x02,0x44,

0x00,0x48,0x22,0x44,

0x00,0x08,0x22,0x44,

0x40,0x08,0x22,0x00,

0x40,0x08,0x00,0x00,

0x43,0x08,0x00,0x00,

0x03,0x08,0x00,0x14,

0x03,0x88,0x00,0x14,

0x00,0x88,0x20,0x14,

0x00,0x88,0x20,0x14,

0x00,0x88,0x20,0x14,

0x80,0x88,0x20,0x14,

0x88,0x08,0x20,0x04,

0x89,0x00,0x20,0x04,

0x01,0x00,0x00,0x04,

0x01,0x40,0x00,0x20,

0x21,0x00,0x00,0x20,

0x21,0x01,0x00,0x20,

0x20,0x01,0x00,0x21,

0x20,0x09,0x06,0x01,

0x20,0x09,0x06,0x04,

0x00,0x01,0x0D,0x00,

0x08,0x01,0x0F,0x00,

0x00,0x01,0x0F,0x80,

0x00,0x81,0x1F,0x80,

0x00,0x41,0x1E,0x88,

0x22,0x21,0x1C,0x88,

0x22,0x21,0x7C,0x88,

0x20,0x21,0xFC,0x88,

0x20,0x21,0xFC,0x88,

0x20,0x21,0xFE,0x88,

0x20,0xC1,0xFF,0xA8,

0x20,0xC1,0xFD,0xA8,

0x20,0x61,0xFF,0xA0,

0x20,0xF1,0xBF,0xA0,

0x20,0xF1,0xBF,0xA0,

0x22,0xF1,0xB7,0xA4,

0x22,0xF1,0xBF,0xA4,

0x22,0xF1,0xF7,0xA0,

0x22,0xD1,0x37,0x80,

0x22,0xD1,0x3F,0x80,

0x22,0xD1,0x1F,0x80,

0x22,0xD1,0x0F,0x88,

0x22,0x11,0x00,0x88,

0x02,0xD1,0x0F,0x09,

0x02,0xD0,0x0F,0x09,

0x0A,0xC0,0x1F,0x09,

0x0A,0xC0,0x3F,0x09,

0x0A,0xE0,0x7F,0x08,

0x0A,0xE0,0x7F,0x88,

0x0A,0xE0,0x7D,0x28,

0x0A,0xE4,0x7F,0x28,

0x42,0xE4,0x6F,0x28,

0x42,0xE4,0x7F,0x2A,

0x42,0xE4,0x7F,0x2A,

0x42,0xE4,0x7F,0x2A,

0x42,0x64,0x7F,0x28,

0x42,0xE4,0x5F,0x28,

0x42,0xE4,0x7F,0x28,

0x42,0xE4,0x7D,0xA0,

0x42,0xE4,0x77,0xA0,

0x42,0xE0,0x7F,0xA2,

0x52,0xA0,0x7F,0xA0,

0x52,0x20,0x7F,0xA8,

0x52,0xE0,0x5E,0xA4,

0x52,0xC4,0x6F,0xA4,

0x52,0xC4,0x3B,0xA4,

0x50,0xC4,0x37,0xA4,

0x48,0xC4,0x3F,0xA0,

0x48,0x84,0x1F,0x20,

0x48,0x84,0x1F,0x21,

0x08,0x04,0x17,0x21,

0x08,0x04,0x1E,0x21,

0x89,0x14,0x1E,0x25,

0x01,0x04,0x1E,0x01,

0x01,0x00,0x1C,0x01,

0x01,0x04,0x00,0x01,
};
static unsigned char bat100[] U8G_PROGMEM ={
  0xE0,0xFF,0x0F,
0x30,0x00,0x18,
0x10,0x00,0x50,
0x10,0x00,0xD0,
0x10,0x00,0xD0,
0x10,0x00,0xD0,
0x10,0x00,0xD0,
0x10,0x00,0x50,
0x30,0x00,0x18,
0xE0,0xFF,0x0F

};
static unsigned char bat75[] U8G_PROGMEM ={
  0xE0,0xFF,0x0F,
0x30,0xC0,0x1B,
0x10,0xC0,0x57,
0x10,0x80,0x57,
0x10,0x80,0x57,
0x10,0x00,0x57,
0x10,0x00,0x57,
0x10,0x00,0x56,
0x30,0x00,0x1A,
0xE0,0xFF,0x0F,

};
static unsigned char bat50[] U8G_PROGMEM ={
  0xE0,0xFF,0x0F,
0x30,0xF8,0x1B,
0x10,0xF8,0x57,
0x10,0xF0,0x57,
0x10,0xF0,0x57,
0x10,0xE0,0x57,
0x10,0xE0,0x57,
0x10,0xC0,0x57,
0x30,0xC0,0x1B,
0xE0,0xFF,0x0F,
};
static unsigned char bat25[] U8G_PROGMEM = {
  0xE0,0xFF,0x0F,
0x30,0xFE,0x1B,
0x10,0xFE,0x57,
0x10,0xFC,0x57,
0x10,0xFC,0x57,
0x10,0xF8,0x57,
0x10,0xF8,0x57,
0x10,0xF0,0x57,
0x30,0xF0,0x1B,
0xE0,0xFF,0x0F,

};
static unsigned char bat0[] U8G_PROGMEM = {
  0xE0,0xFF,0x03,
0xB0,0xFF,0x19,
0xD0,0x7F,0x54,
0xD0,0x3F,0x57,
0xD0,0x9F,0x57,
0xD0,0xC7,0x57,
0xD0,0xF3,0x57,
0xD0,0xF8,0x57,
0x30,0xFE,0x1B,
0x80,0xFF,0x0F,

};
static unsigned char charge[] U8G_PROGMEM = {
  0xE0,0xFF,0x0F,
0xB0,0xFF,0x1B,
0xD0,0xC7,0x57,
0xD0,0x07,0x57,
0x50,0xC4,0x57,
0x50,0xC4,0x57,
0xD0,0x07,0x57,
0xD0,0xC7,0x57,
0xB0,0xFF,0x1B,
0xE0,0xFF,0x0F,
};
boolean carefulB; // запрет на парение при подкл. ЗУ
boolean flagR; // для анимации
float filter_k = 0.04;
float PWM_filter_k = 0.1;
//---------------загрузка----------------------------------
void setup(void) {
  if (initial_calibration){
    Serial.begin(9600);
    calibration();  // калибровка, если разрешена
  }
  //----читаем из памяти-----
  volts = EEPROM.readInt(0);
  watt = EEPROM.readInt(2);
  ohms = EEPROM.readFloat(4);
  my_vcc_const = EEPROM.readFloat(8);
  //----читаем из памяти-----

  Timer1.initialize(1500); 
  u8g.setRot90();
  
  menu_redraw_required = 1; 
  //ИНИЦИАЛИЗИРУЕМ КНОПКИ
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(FIRE, INPUT_PULLUP);
  pinMode(SET, INPUT_PULLUP);
  
  pinMode(battery, INPUT);    // для замеров напряжений с акб
  pinMode(vievChardge, INPUT);  //     и входа для зарядки
  pinMode(mosfet , OUTPUT);  // выход на ключ
  
  Timer1.disablePwm(mosfet);    // принудительно отключить койл
  digitalWrite(mosfet, LOW);
  
  bat_vol = readVcc();
  bat_old = bat_vol;
  // проверка заряда акума, если разряжен то прекратить работу
  if (bat_vol < battery_low * 1000) {
    flag = 0;
    Timer1.disablePwm(mosfet);    // принудительно отключить койл
    digitalWrite(mosfet, LOW);    // принудительно отключить койл
    drawBattery();                // показывать зарядку 
  }else{
    flag = 1;
  }
}
//---------------загрузка----------------------------------
//---------------анимация при парении----------------------
void drawRocket(){
  if(flagR){
    u8g.drawXBMP( 0, 0, 32, 128, fps1); //рисуем 1 кадр
  }
  else if(!flagR){
    u8g.drawXBM( 0, 0, 32, 128, fps2);  //рисуем 2 кадр
  }
}
//----------------анимация при парении---------------------
//-------------------главный метод-------------------------
void loop() {
  if (millis() - last_time > 50) {                       // 20 раз в секунду измеряем напряжение
    last_time = millis();
    bat_vol = readVcc();                                 // измерить напряжение аккумулятора в миллиВольтах
    bat_volt_f = filter_k * bat_vol + (1 - filter_k) * bat_old;  // фильтруем
    bat_old = bat_volt_f;                                // фильтруем
    if (bat_volt_f < battery_low * 1000) {               // если напряжение меньше минимального
      flag = 0;                                          // прекратить работу
      Timer1.disablePwm(mosfet);    // принудительно отключить койл
      digitalWrite(mosfet, LOW);    // принудительно отключить койл
      drawBattery();                //показывать заряд
    }
  }
  uiStep();     //метод считывающий нажатия кнопок
  if(flag){               // если батарея не разряжена, то           
    if (menu_redraw_required || analogRead(vievChardge) > 1020 && mode == 3 || bat_vol != bat_old && mode == 3) {  // обновлять меню только при изменении батареи, подключ зарядки,
      u8g.firstPage();                                                                                            //или при нажатии кнопки и только если мы находимся в меню (mode =3)
      do  {
        drawMenu();                         // прорисовывать меню
      } while( u8g.nextPage() );
      menu_redraw_required = false;       //отметить что меню изменили 
    }
    updateMenu();         // считать перемещение курсора меню
    choseStr();           // считывать выбранный элемент меню
    //--------------------------------ВАРИВОЛЬТ-----------------------------------------
    if(mode == 0 && !fire){          // если мы в режиме варивольт (mode = 0) и кнопка парить не нажата
          u8g.firstPage();      
          do{
            varvoltDraw();           // отображать вкладку варивольт
          }while(u8g.nextPage());  
          if(up && !upClick){               //если нажата кнопка вверх
            volts += 0.1;                   // увеличить на 0,1В
            volts = min(volts, (float)bat_volt_f/1000);  //ограничение сверху
            upClick=1;
          }
          if(!up && upClick){
            upClick = 0;
            change_v_flag = 1;
          }     
          if (down && !downClick){   // если нажата кнопка вниз
            volts -= 0.1;             // уменьшить на 0,1В 
            volts = max(volts, 0);    // ограничение снизу
            downClick = 1;        
          }
          if(!down && downClick){
            downClick = 0;
          }
          if(setB && !setClick){    // если нажали сет выйти из вкладки варивольт
            mode = 3;               // поставить режим меню
            u8g.firstPage();
            do{
              drawMenu();           //рисовать меню
            }while(u8g.nextPage());
            setClick = 1;
          }
          if(!setB && setClick){
            setClick = 0;
          }
          
    }
    //--------------------------------ВАРИВОЛЬТ-----------------------------------------
    //---------------------------------------------ВАРИВАТТ-------------------------------------------------------
    if(mode == 1 && !fire){  // если режим вариватт и кнопка парить не нажата
          u8g.firstPage();
          maxW = (sq((float)bat_volt_f / 1000)) / ohms;        // при переходе во вкадку расчитать макс мощность
          do{
            varwattDraw();          //отображать вкладку вариватт
          }while(u8g.nextPage());
          if(up && !upClick){        //при нажатии кнопки вверх увеличить мощность на 1
            watt++;
            watt = min(watt, maxW);   //ограничение сверху по макс мощности
            upClick=1;
          }
          if(!up && upClick){
            upClick = 0;
            change_w_flag = 1;
          }
          if (down && !downClick){   //при нажатии вниз уменьшить на 1
            watt--;
            watt = max(watt, 0);     //ограничение снизу
            downClick = 1;
          }
          if(!down && downClick){
            downClick = 0;
          }
          if(setB && !setClick){    // если нажали сет, выйти в меню
            setClick = 1;
            mode = 3;               // поставить режим меню
            u8g.firstPage();
            do{
              drawMenu();           //рисовать меню
            }while(u8g.nextPage());
          }
          if(!setB && setClick){
            setClick = 0;
          }
    }
    //---------------------------------------------ВАРИВАТТ-------------------------------------------------------
    //----------------------------------------СОПРОТИВЛЕНИЕ-----------------------------------------------------
    if (mode == 2 && !fire){    // указать сопротивление mode = 2 и кнопка парить не нажата
          u8g.firstPage();
          do{
            varcoilDraw();
          }while(u8g.nextPage());
          if(up && !upClick){
            ohms += 0.05;
            ohms = min(ohms, 1.5);
            upClick=1;
          }
          if(!up && upClick){
            upClick = 0;
            change_o_flag = 1;
          }
          if (down && !downClick){
            ohms -= 0.05;
            ohms = max(ohms, 0.05);
            downClick = 1;
          }
          if(!down && downClick){
            downClick = 0;
          }
          if(setB && !setClick){
            setClick = 1;
            mode = 3;
            u8g.firstPage();
            do{
              drawMenu();
            }while(u8g.nextPage());
          }
          if(!setB && setClick){
            setClick = 0;
          }
      }
      //----------------------------------СОПРОТИВЛЕНИЕ-------------------------------------
      //---------------------ОТРАБОТКА НАЖАТИЙ КНОПКИ ПАРИТЬ--------------------------------
      if (fire && !wake_up_flag && mode !=3 && mode != 2 && !carefulB) {    //если кнопка парить нажата ,не спим , находимся во вкладке ВАРИВАТТ или ВАРИВОЛЬТ, и стоим не на зарядке 
            if (!vape_flag) {
              vape_flag = 1;
              vape_mode = 1;            // первичное нажатие
              delay(20);                // анти дребезг (сделал по-тупому, лень)
              vape_press = millis();    // первичное нажатие
            }
            if (vape_release_count == 1) {
              vape_mode = 2;               // двойное нажатие
              delay(20);                   // анти дребезг (сделал по-тупому, лень)
            }
            if (millis() - vape_press > vape_threshold * 1000) {  // "таймер затяжки"
              vape_mode = 0;
              digitalWrite(mosfet, 0);
            }
            if (vape_mode == 1) {
              if(mode == 0){                                                   //ВАРИВОЛЬТ
                PWM = (float)volts*1000 / bat_volt_f * 1024;                  // считаем значение для ШИМ сигнала 
              }                                                              
              if(mode == 1){                                                //ВАРИВАТТ
                 PWM = (float )watt / maxW * 1024;                         // считаем значение для ШИМ сигнала 
              }
              if (PWM > 1023) PWM = 1023;                                  // ограничил PWM "по тупому"
              PWM_f = PWM_filter_k * PWM + (1 - PWM_filter_k) * PWM_old;  // фильтруем
              PWM_old = PWM_f;                                           // фильтруем
              
              Timer1.pwm(mosfet, PWM_f);                               //открываем ключ
              u8g.firstPage(); 
              do{
                 drawRocket();                                // рисуем анимацию
              }while(u8g.nextPage());
              if (round(millis() / 150) % 2 == 0)
                  flagR = !flagR; else flagR = !flagR;        
              
              }   
            vape_btt = 1;
        }
        if (!fire && vape_btt) {  
          // если кнопка ПАРИТЬ отпущена
          if (millis() - vape_press > 180) {
              vape_release_count = 0;
              vape_mode = 0;
              vape_flag = 0;
          }
          vape_btt = 0;
          if (vape_mode == 1) {
             vape_release_count = 1;
             vape_press = millis();
          }
          digitalWrite(mosfet, 0);
          mode_flag = 0;
      
          // если есть изменения в настройках, записать в память
          if (change_v_flag) {
              EEPROM.writeInt(0, volts);
              change_v_flag = 0;
          }
          if (change_w_flag) {
              EEPROM.writeInt(2, watt);
              change_w_flag = 0;
          }
          if (change_o_flag) {
              EEPROM.writeFloat(4, ohms);
              change_o_flag = 0;
          }      
       }
  }
  if (wake_up_flag) wake_puzzle(); 
  
  if (millis() - wake_timer > sleep_timer * 1000) {  // если кнопки не нажимались дольше чем sleep_timer секунд
    good_night();
  }
}
//-------------------главный метод-------------------------
//----------------ВЫБОР МЕНЮ---------------------
void choseStr(){
  if(setB && mode == 3 && !wake_up_flag){            //выбераем если находимся в меню (mode = 3) и не во сне
      switch(menu_current){                   //элементы меню
        case 0:
          mode = 0;                           // mode = 0 режим варивольт
          break;
        case 1: 
          mode = 1;                          // mode = 1 режим вариватт
          break;
        case 2:
          mode = 2;                          //mode = 2 поставить сопротивление
          break;
        case 3:
          good_night();                      // четвертая строка "OFF" выкл вейп
          break;  
      }
    }
}
//----------------ВЫБОР МЕНЮ---------------------
//----------------СЧИТЫВАНИЕ КНОПОК----------------
void uiStep(void) {
  up = !digitalRead(UP);
  down = !digitalRead(DOWN);
  setB = !digitalRead(SET);
  fire = !digitalRead(FIRE);
  if(mode == 3){                        //считывать только если мы в меню (mode = 3)
    if(down && !downClick){
      uiKeyCode = KEY_PREV;
      downClick = true;
    }
    if(!down && downClick){
      //uiKeyCode = KEY_NONE;
      downClick=false;
    }
    if(up && !upClick ){
      uiKeyCode = KEY_NEXT;
      upClick = true;
    }
    if(!up && upClick){
      //uiKeyCode = KEY_NONE;
      upClick= false;
    }
  }
  if (up || down || setB || fire) wake_timer = millis(); // обновлять таймер при  нажатии любой кнопки
  /*
  uiKeyCodeSecond = uiKeyCodeFirst;
  if ( !digitalRead(FIRE) )
    uiKeyCodeFirst = KEY_FIRE;
  else if ( !digitalRead(UP))
    uiKeyCodeFirst = KEY_NEXT;
  else if ( !digitalRead(DOWN))
    uiKeyCodeFirst = KEY_PREV;
  else if ( !digitalRead(SET))
    uiKeyCodeFirst = KEY_SET;
  else 
    uiKeyCodeFirst = KEY_NONE;
  
  if ( uiKeyCodeSecond == uiKeyCodeFirst )
    uiKeyCode = uiKeyCodeFirst;
  else
    uiKeyCode = KEY_NONE;
  */
}
//----------------СЧИТЫВАНИЕ КНОПОК----------------
//----------------ПЕРЕМЕЩЕНИЕ СТРОКИ------------------
void updateMenu(void) {
  /*
  if ( uiKeyCode != KEY_NONE && last_key_code == uiKeyCode ) {
    return;
  }
  last_key_code = uiKeyCode;
  */
  if(mode == 3){                            //только если находимся в меню
    switch ( uiKeyCode ) {                          
    case KEY_PREV:
      uiKeyCode = KEY_NONE;                   //чтобы не зациклилось обновляем свитч                 ПРИМЕР:   |  1 элемент
      menu_current++;                         //кнопка вниз прибавит меню, тобишь строка спуститься            V  2 элемент
      if ( menu_current >= MENU_ITEMS )       // если мы превысили кол-во элементов вернуться в начало
        menu_current = 0;
      menu_redraw_required = true;             //поднимаем флаг о изменении, разрешающий обновить экран
      break;
    case KEY_NEXT:                            //точно также как и в предыдущем case
      uiKeyCode = KEY_NONE;
      if ( menu_current == 0 )
        menu_current = MENU_ITEMS;
      menu_current--;
      menu_redraw_required = true;
      break;
    }
  }
}
//----------------ПЕРЕМЕЩЕНИЕ СТРОКИ------------------
//-----------------ОТРИСОВКА ПРИ ВКЛЮЧЕНИИ-------------------
void draw(byte b) {
    if(b == 0){
     u8g.drawXBM( 0, 50, u8g_logo_width, u8g_logo_height, one);
    }
    else if (b == 2){
      u8g.drawXBMP( 0, 50, u8g_logo_width, u8g_logo_height, three);
    }
    else if (b == 2){
      u8g.drawXBMP( 0, 50, u8g_logo_width, u8g_logo_height, four);
    }
    else if(b == 3){
     // u8g.drawXBMP( 0, 50, u8g_logo_width, u8g_logo_height, five);
    }
    else if (b == 4){
      u8g.drawXBMP( 0, 50, u8g_logo_width, u8g_logo_height, seven);
    }
}
//-----------------ОТРИСОВКА ПРИ ВКЛЮЧЕНИИ-------------------
//-----------------------ИНДИКАТОР ЗАРЯДА--------------------------
void drawBattery(){
  u8g.setFont(u8g_font_helvB08);   //ставим шрифт
  u8g.setPrintPos(5,115);           //ставим положение
  u8g.print((float)bat_volt_f/1000); //пишем напряжение на батареи 
  if(analogRead(vievChardge) < 1020){     //если зарядка не подключена, то
    carefulB = false;                    // разрешить парить
    if(bat_volt_f >= 3850){                  // если напряжение больше 3,85В рисовать полную батарею
      u8g.drawXBMP(2, 118, 23, 10, bat100);    
    }
    else if(bat_volt_f <3850 && bat_volt_f >= 3500){   //если напряжение меньше 3,85В и более 3,5В рисовать 75% батареи
      u8g.drawXBMP(2, 118, 23, 10, bat75);                              //  и т.д
    }
    else if(bat_volt_f <3500 && bat_volt_f >= 3150){
      u8g.drawXBMP(2, 118, 23, 10, bat50);
    }
    else if(bat_volt_f < 3150 && bat_volt_f >= 3000){   
      u8g.drawXBMP(2, 118, 23, 10, bat25);
    }
    else if(bat_volt_f < 3000){
      u8g.drawXBMP(2, 118, 23, 10, bat0);
    }
  }
  else{                           //если зарядка подключена
    carefulB = true;                 //запрет на парение
    u8g.drawXBMP(2, 118, 23, 10, charge);      // рисовать зарядку
  }
}
//-----------------------ИНДИКАТОР ЗАРЯДА--------------------------
//------------------МЕНЮ--------------------
void drawMenu(void) {
  byte i = 1;
  u8g.setFont(u8g_font_tpssb);
  u8g.setFontPosTop();
  for( i = 0; i < MENU_ITEMS; i++ ) {
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {
      u8g.drawBox(0, i*12+30, 32, 12);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(0, i*12+30, menu_strings[i]);
  }
  drawBattery();
}
//------------------МЕНЮ--------------------
//--------------ВКЛАДКА "ВАРИВАТТ"-------------
void varwattDraw(){
    u8g.setFont(u8g_font_fub17);
    u8g.drawStr(5, 35, "W");
    u8g.drawFrame(0,44,32,40);
    if(watt>9){                //Центрирование числа
      u8g.setPrintPos(2,74);
    }else{
      u8g.setPrintPos(9,74);
    }
    u8g.print(watt);
    u8g.setFont(u8g_font_tpssb);
    u8g.setPrintPos(2,95);
    u8g.print(maxW);
    u8g.drawStr(0,104, "maxW");
    drawBattery();
}
//--------------ВКЛАДКА "ВАРИВАТТ"-------------
//--------------ВКЛАДКА "ВАРИВОЛЬТ"-------------
void varvoltDraw(){
    u8g.setFont(u8g_font_fub17);
    u8g.drawStr(7, 35, "V");
    u8g.drawFrame(0,44,32,40);
    u8g.setPrintPos(0,74);
    u8g.print(volts);
    drawBattery(); 
}
//--------------ВКЛАДКА "ВАРИВОЛЬТ"-------------
//--------------ВКЛАДКА "КОЙЛ"-------------
void varcoilDraw(){
  u8g.setFont(u8g_font_fub11);
  u8g.drawStr(1, 35, "Om");
  u8g.drawFrame(0,44,32,40); 
  u8g.setPrintPos(0,74);
  u8g.print(ohms);
  drawBattery(); 
}
//--------------ВКЛАДКА "КОЙЛ"-------------
//--------калибровка----------
void calibration() {
  for (byte i = 0; i < 7; i++) EEPROM.writeInt(i, 0);          // чистим EEPROM для своих нужд
  my_vcc_const = 1.1;
  Serial.print("Real VCC is: "); Serial.println(readVcc());     // общаемся с пользователем
  Serial.println("Write your VCC (in millivolts)");
  while (Serial.available() == 0); int Vcc = Serial.parseInt(); // напряжение от пользователя
  float real_const = (float)1.1 * Vcc / readVcc();              // расчёт константы
  Serial.print("New voltage constant: "); Serial.println(real_const, 3);
  EEPROM.writeFloat(8, real_const);                             // запись в EEPROM
  while (1); // уйти в бесконечный цикл
  //------конец калибровки-------
}
long readVcc() { //функция чтения внутреннего опорного напряжения, универсальная (для всех ардуин)
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
  if(DCDC_Boster == 1){
    result = (my_vcc_const * 1023 * 1000 / result)*analogRead(battery)/1023;// переделанный метод для измерения напряжения на акб,          
                                                                      //короче говоря U = (battery/1023)*K где K=[4.5В - 5.2В];K = напряжение питания, поскольку питаем от повышайки
  }else{
    result = (my_vcc_const * 1023 * 1000 / result);
  }
  return result; // возвращает bat_vol   
}
void wake_up() {
  u8g.sleepOn(); //выкл дисп
  Timer1.disablePwm(mosfet);    // принудительно отключить койл
  digitalWrite(mosfet, LOW);    // принудительно отключить койл
  wake_timer = millis();         // запомнить время пробуждения
  wake_up_flag = 1;
  vape_release_count = 0;
  vape_mode = 0;
  vape_flag = 0;
  mode_flag = 1;
}
void wake_puzzle() {
  detachInterrupt(0);    // отключить прерывание
  vape_btt_f = 0;
  boolean wake_status = 0;
  byte click_count = 0;
  u8g.sleepOff();   // разбудить дисплей
  while (true) {
    u8g.firstPage();    //обновить экран
    fire = !digitalRead(FIRE);    
    if (fire && !vape_btt_f) {
      vape_btt_f = 1;
      switch(click_count){    //анимация по нажатиям
        case 0:           
          do{
            draw(click_count);       // в качестве параметра метода даем кол-во нажатий, по ним и идёт отображение нужной картинки
          }while(u8g.nextPage());
        case 1:
          do{
            draw(click_count);
          }while(u8g.nextPage());
        case 2:
          do{
            draw(click_count);
          }while(u8g.nextPage());
        case 3:
          do{
            draw(click_count);
          }while(u8g.nextPage());
        case 4:
          do{
            draw(click_count);
          }while(u8g.nextPage());
      }
      click_count++;
      if (click_count > 4) {               // если 5 нажатий сделаны за 3.5 секунды
        u8g.firstPage();                    //обновить экран
        do{
            drawMenu();                         //войти в меню
        }while(u8g.nextPage());  
        wake_status = 1;                                      // флаг "проснуться"
        break;
      }
    }
    if (!fire && vape_btt_f) {
      vape_btt_f = 0;
      delay(70);
    }
    if (millis() - wake_timer > 3500) {    // если 5 нажатий не сделаны за 3.5секунды
      wake_status = 0;                     // флаг "спать"
      break;
    }
  }
  if (wake_status) {
    wake_up_flag = 0;
    delay(100);
  } else {
    good_night();     // дальше спать
  }
}
//----------------------------метод сна------------------------------------
void good_night() {
  Timer1.disablePwm(mosfet);    // принудительно отключить койл
  digitalWrite(mosfet, LOW);    // принудительно отключить койл
  delay(50);  
  u8g.firstPage();     // обновить экран
  u8g.sleepOn();       //пора спать
  delay(50);  
  attachInterrupt(0, wake_up, FALLING);                   // подключить прерывание для пробуждения на 2 пин 
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);    // спать. mode POWER_OFF, АЦП выкл
}
//----------------------------метод сна------------------------------------
