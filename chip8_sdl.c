#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "chip8.h"

#define C8_WINDOW_WIDTH 1024
#define C8_WINDOW_HEIGHT 768

int main(int argc, char *argv[]) {

  SDL_Window *window = NULL;
  SDL_Surface *screenSurface = NULL;
  int status = 0;
  unsigned int instr;
  unsigned int pixel_width;
  unsigned int pixel_height;
  int running = 1;
  SDL_Event event;
  
  c8_start();


  /* calculate the pixel width and height
   * The Chip 8 display is 64x32 pixels
   * pixel size:
   * 64 * x = C8_WINDOW_WIDTH
   * 32 * x = C8_WINDOW_HEIGHT
   * x will be the size of the pixels
   */

  pixel_width = C8_WINDOW_WIDTH / 64;
  pixel_height = C8_WINDOW_HEIGHT / 32;
  
  if(SDL_Init(SDL_INIT_VIDEO < 0)) {
    printf("Error initializing SDL\n");
    return 0;
  }

  window = SDL_CreateWindow("Chip-8 Interpreter",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    C8_WINDOW_WIDTH,
			    C8_WINDOW_HEIGHT,
			    SDL_WINDOW_SHOWN);

  screenSurface = SDL_GetWindowSurface(window);

  show_registers();
  c8_test();


  while(running) {

    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
	running = 0;
      }
      if(event.type == SDL_KEYDOWN) {
	switch(event.key.keysym.sym) {
	case SDLK_SPACE:
	  show_registers();
	  break;
	case SDLK_RETURN:
	  c8_set_state(RUNNING);
	  break;
	}
      }
    }

    /* keep fetching and decoding as long as the status is ok 
     * this is kept separate from the SDL loop so that SDL can
     * continue running even if the interpreter has finished.
     */
    if(status >= 0 && c8_state == RUNNING) {
      instr = c8_fetch_instruction();
      status = c8_decode_instruction(instr);
    }

    draw_screen(screenSurface, pixel_width, pixel_height);  
    SDL_UpdateWindowSurface(window);
  }   

  
  SDL_DestroyWindow(window);
  SDL_Quit();
  c8_quit();
  return 0;
}

void draw_screen(SDL_Surface *surface, int pixel_width, int pixel_height) {

  int x, y;
  unsigned int color = SDL_MapRGB(surface->format, 0, 255, 0);

  /* clear the screen */
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

  for(y = 0; y < C8_DISPLAY_WIDTH; y++) {
    for(x = 0; x < C8_DISPLAY_HEIGHT; x++) {
      if(c8_display[y][x] == 1) {
	draw_pixel(surface, x, y, pixel_width, pixel_height, color);
      }
    }
  }
}

void draw_pixel(SDL_Surface *surface, int x, int y, int pixel_width,
		int pixel_height, unsigned int color) {
  
  SDL_Rect rect;
  
  rect.x = x * pixel_width;
  rect.y = y * pixel_height;
  rect.w = pixel_width;
  rect.h = pixel_height;

  SDL_FillRect(surface, &rect, color);
}


