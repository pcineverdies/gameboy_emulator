#include "display.h"
#include <stdexcept>

/** Display::Display
    Constructor of the class.
    Creates the window and the renderer for SDL2

    The window is WxH, scaled by an integer factor S.
    In this way, each pixel from the original display is mapped into a square
    of size SxS

    @param W uint8_t width of the screen
    @param H uint8_t height of the screen
    @param S uint8_t scale factor for the screen

*/
Display::Display(uint8_t W, uint8_t H, uint8_t S) {

  width = W;
  height = H;
  scale_factor = S;
  last_cleared = false;

  // Init SDL
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    std::runtime_error("SDL_Init failed");
  }

  // Create the window and the renderer
  if(SDL_CreateWindowAndRenderer(width * scale_factor, height * scale_factor,
                                 0, &this->window, &this->renderer) != 0){
    std::runtime_error("SDL_CreateWindowAndRenderer failed");
  }

  // Set init color
  SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);

  // Clear window
  SDL_RenderClear(this->renderer);

  // Render window
  SDL_RenderPresent(this->renderer);
}

/** Display::update
    Given a point of the screen, update it with the provided color

    @param x uint8_t x coordinate of the point to update
    @param y uint8_t y coordinate of the point to update
    @param color uint32_t color to use to update
*/
void Display::update(uint8_t x, uint8_t y, uint32_t color){

  if(x >= width or y>=height){
    std::invalid_argument("Provided coordinate to the display are not correct");
  }

  SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(this->renderer,
                         (color >> 24) & 0xff,
                         (color >> 16) & 0xff,
                         (color >> 8) & 0xff,
                         color & 0xff);

  for(int i = 0; i < scale_factor; i++){
    for(int j = 0; j < scale_factor; j++){
      SDL_RenderDrawPoint(this->renderer,
                          i + x * scale_factor,
                          j + y * scale_factor);
    }
  }

  // Render modifications
  if(x == width - 1 and y == height - 1)
  SDL_RenderPresent(this->renderer);

  last_cleared = false;
}

/** Display::clear
    Clear the whole display with the same color

    @param color uint32_t color to use to clear
*/
void Display::clear(uint32_t color){

  if(last_cleared) return;

  SDL_SetRenderDrawColor(this->renderer,
                         (color >> 24) & 0xff,
                         (color >> 16) & 0xff,
                         (color >> 8) & 0xff,
                         color & 0xff);

  for(int i = 0; i < width*scale_factor; i++){
    for(int j = 0; j < height*scale_factor; j++){
      SDL_RenderDrawPoint(this->renderer, i, j);
    }
  }

  // Render modifications
  SDL_RenderPresent(this->renderer);

  last_cleared = true;
}

/** Display::~Display
    Destroyer of the class.

*/
Display::~Display(){
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

