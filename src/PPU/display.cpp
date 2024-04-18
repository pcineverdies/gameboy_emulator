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

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Set init color
  SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);

  // Clear window
  SDL_RenderClear(this->renderer);

  // Render window
  SDL_RenderPresent(this->renderer);
}

/** Display::update
    Given the content of the screen, updates the display

    @param data uint32_t* array containig the data to update the screen
*/
void Display::update(uint32_t* data){

  uint8_t* pixels;
  int pitch = 0;

  // Clear renderer
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer);

  // Lock texture
  SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch);

  // Copy data from the input array to the texture
  memcpy(pixels, (void*)data, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

  // Unlock, copy and render
  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

/** Display::clear
    Clear the whole display with the same color

    @param color uint32_t color to use to clear
*/
void Display::clear(uint32_t color){

  if(last_cleared) return;

  // Conversion from RGB555 to RGB888
  SDL_SetRenderDrawColor( this->renderer,
                         (color >> 16) & 0xff,
                         (color >>  8) & 0xff,
                         (color >>  0) & 0xff,
                         0xff);

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

