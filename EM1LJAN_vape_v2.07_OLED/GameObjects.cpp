#include "GameObjects.h"
#include "Game.h"

void Tube::move(){
  x -= speedGame;
  if(x < - tubeWidth - 20) {
      x = 32 + tubeWidth;
      window = (128 - heightWindow)*(float)rand()/RAND_MAX;
      Game::score++;
  }
}


void Bird::fly(){
   dy = 1;
}

void Bird::move(){
  dy += g*Game::time/1000;  // v = at
  y += dy*Game::time/ 1000;  // h = vt 
  
  if(y > 1) {
      y = 1;
      dy = 0;
  }
  if(y < 0.05) {
      y = 0.1;
      dy = 0;
  }
}

void Bird::kill(){
  isAlive = false;
}
