#include "Game.h"

#include "Drawable.h"
#include "vape.h"


unsigned long Game::rs = 0;
float Game::time = 0;
int Game::score = 0;
bool Game::direction = false;
short Game::width = 14;
short Game::height = 15;

Drawable drawable;

Tube* tube; 
Bird* bird;
Vape v;  

void Game::setButtons(Button* fire, Button* set){
  fireButton = fire;
  setButton = set;
}

void Game::createGame(){
  bird = new Bird();
  tube = new Tube();
  
  drawable.setGameObjects(bird , tube);
  rs = millis();
  time = 0;
  score = 0;
  Game::flappyBird();
}



void Game::flappyBird(){
  
  bool exit = false;
  bool restart = false;

  while(true){
    Game::check();
    drawable.drawGame();
    time = (millis() - Game::rs);
    Game::rs = millis();
    bird->move();
    if(fireButton->hold() && !Game::direction && bird->isAlive){
      Game::direction = true;
      Game::width = 16;
      Game::height = 14;
      bird->fly();
    }
    if(!fireButton->hold() && Game::direction){
      Game::direction = false;
      Game::width = 14;
      Game::height = 15;
    }
    if(bird->isAlive){
      tube->move();
    }
    if(!bird->isAlive && bird->y < 0.07){
       break;
    }
    
    if(setButton->click()){
       break;
    }
  }
  
  while(true){
    if(fireButton->click()){
      restart = true;
      break;
    }
    if(setButton->click()){
      exit = true;
      break;
    }
  }
  if(restart){
    restart = false;
    if(bird->isAlive){
      Game::flappyBird(); 
    }
    delete tube;
    delete bird;
    tube = nullptr;
    bird = nullptr;
    Game::createGame();
  }
  if(exit){
    delete tube;
    delete bird;
    tube = nullptr;
    bird = nullptr;
    exit = false;
    v.mainMenu();
  }
}

void Game::check(){
  if((tube->x - tubeWidth/2 < birdPosX  + Game::width) && (tube->x + tubeWidth/2 > birdPosX)) {
      bool collision = (((1 - bird->y) * 128 < tube->window) || ((1-bird->y) * 128 + Game::height > tube->window + heightWindow)); 
      if(collision){
         bird->kill();
      } 
  }
}
