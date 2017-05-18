#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "chip8.h"

#define C8_WINDOW_WIDTH 1024
#define C8_WINDOW_HEIGHT 768

unsigned int timer_callback(unsigned int interval, void *p);

int main(int argc, char *argv[]) {

  SDL_Window *window = NULL;
  SDL_Surface *screenSurface = NULL;
  int status = 0;
  unsigned int instr;
  unsigned int pixel_width;
  unsigned int pixel_height;
  int running = 1;

  SDL_TimerID timer_id;
  SDL_Event event;
  
  int key, key_status;
  int go = 0;
  
  
  char binfile[20];
  int bytesize;

  
  printf("BIN File to load: ");
  scanf("%s", &binfile);
  printf("Bytes: ");
  scanf("%d", &bytesize);
  
  
  c8_start();
  /*c8_load_from_file("display.bin");*/

 
  status = c8_load_bin(binfile, bytesize);
  if(status == 0) {
    printf("Error loading the BIN file.");
    return 0;
  }
  
  /* calculate the pixel width and height
   * The Chip 8 display is 64x32 pixels
   * pixel size:
   * 64 * x = C8_WINDOW_WIDTH
   * 32 * x = C8_WINDOW_HEIGHT
   * x will be the size of the pixels
   */

  pixel_width = C8_WINDOW_WIDTH / 64;
  pixel_height = C8_WINDOW_HEIGHT / 32;
  
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
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


  /*setup the timer 
    The timer ticks 60 times a second
  */
  timer_id = SDL_AddTimer(1000 / 60, timer_callback, NULL);  
  
  c8_set_state(PAUSED);
  
  while(running) {
      
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
	running = 0;
      }
      if(event.type == SDL_KEYDOWN) {
	switch(event.key.keysym.sym) {
	case SDLK_q:
	  running = 0;
	  break;
	  
	case SDLK_SPACE:
	  show_registers();
	  break;
	case SDLK_RETURN:
	  dump_mem(512, 16);
	  break;
	case SDLK_k:
	  dump_display();
	  break;

	case SDLK_p:
	  c8_set_state(PAUSED);
	  break;

	case SDLK_g:
	  c8_set_state(RUNNING);
	  go = 1;
	  break;
	  
	case SDLK_r:
	  c8_set_state(RUNNING);
	  
	  instr = c8_fetch_instruction();
	  printf("Executing: 0x%04X\n", instr);
	  status = c8_decode_instruction(instr);

	  c8_set_state(RUNNING);
	  break;
	  
	default:
	  key = get_key(event.key.keysym.sym);
	  key_status =  c8_keydown(key);

	  if(key_status == 1 && c8_state == WAITING_FOR_KEY) {
	    c8_got_keypress(key);
	  }
	  break;  
	}
      }
      
      if(event.type == SDL_KEYUP) {
	key = get_key(event.key.keysym.sym);
	c8_keyup(key);
      }
    }
  


    /* keep fetching and decoding as long as the status is ok 
     * this is kept separate from the SDL loop so that SDL can
     * continue running even if the interpreter has finished.
     */

    if(go) {
      if(status >= 0 && c8_state == RUNNING) {
	instr = c8_fetch_instruction();
	status = c8_decode_instruction(instr);
      }
    }
    
    draw_screen(screenSurface, pixel_width, pixel_height);  
    SDL_UpdateWindowSurface(window);
  }   

  SDL_RemoveTimer(timer_id);
  SDL_DestroyWindow(window);
  SDL_Quit();
  c8_quit();
  return 0;
}

unsigned int timer_callback(unsigned int interval, void *p) {
  if(is_timer_running()) {
    c8_timer_tick();
  }    
  return interval;
}

int get_key(SDL_Keycode keycode) {
  int key = -1;
  
  switch(keycode) {
  case SDLK_0:
    key = 0;
    break;
    
  case SDLK_1:
    key = 1;
    break;

  case SDLK_2:
    key = 2;
    break;

  case SDLK_3:
    key = 3;
    break;

  case SDLK_4:
    key = 4;
    break;

  case SDLK_5:
    key = 5;
    break;
    
  case SDLK_6:
    key = 6;
    break;
    
  case SDLK_7:
    key = 7;
    break;
    
  case SDLK_8:
    key = 8;
    break;
    
  case SDLK_9:
    key = 9;
    break;
    
  case SDLK_a:
    key = 10;
    break;
    
  case SDLK_b:
    key = 11;
    break;
    
  case SDLK_c:
    key = 12;
    break;
    
  case SDLK_d:
    key = 13;
    break;
    
  case SDLK_e:
    key = 14;
    break;
    
  case SDLK_f:
    key = 15;
    break;
   
  }  

  return key;  
}

void draw_screen(SDL_Surface *surface, int pixel_width, int pixel_height) {

  int x, y;
  unsigned int on_color = SDL_MapRGB(surface->format, 0, 255, 0);
  unsigned int off_color = SDL_MapRGB(surface->format, 0, 0, 0);
  
  /* clear the screen */
  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

  for(y = 0; y < C8_DISPLAY_HEIGHT; y++) {
    for(x = 0; x < C8_DISPLAY_WIDTH; x++) {
      if(c8_display[y][x] == 1) {
	draw_pixel(surface, x, y, pixel_width, pixel_height, on_color);
      }else{
	draw_pixel(surface, x, y, pixel_width, pixel_height, off_color);
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


