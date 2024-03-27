#include "../PPU/display.h"
#include "../PPU/PPU_def.h"
#include <iostream>

int main(){
  int x, y;
  Display d(SCREEN_WIDTH, SCREEN_HEIGHT, SCALE_FACTOR);

  int counter = 0;

  d.clear(0x00000ff);

  while(1){
    std::cin >> x;
    std::cin >> y;

    if(x >= SCREEN_WIDTH or y >= SCREEN_HEIGHT) break;

    if(counter % 2 == 0) d.update(x, y, 0xffffffff);
    if(counter % 2 == 1) d.update(x, y, 0x00ffff00);
    counter++;
  }

}
