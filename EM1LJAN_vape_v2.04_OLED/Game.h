#pragma once

#include "GameObjects.h"
#include "Button.h"
#include "Vape.h"
class Game{
public:
  void setButtons(Button* fire, Button* set);
  void createGame();
  void flappyBird();
  
  static float time;
  static int score;
  static bool direction;
  
private:
  Button* fireButton;
  Button* setButton;
  
  static short width;
  static short height;
  static unsigned long rs;
  void check();
};
