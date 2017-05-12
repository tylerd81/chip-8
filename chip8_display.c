#include <stdio.h>
#include "chip8.h"

void dump_display(void) {
  int row, col;

  for(row = 0; row < C8_DISPLAY_HEIGHT; row++) {
    for(col = 0; col < C8_DISPLAY_WIDTH; col++) {
      if(c8_display[row][col] == 1) {
	printf("*");
      }else{
	printf(" ");
      }      
    }
    printf("\n"); /* next row */
  }
}

/***************************************************************
 *c8_draw_sprite()
 * copies the sprite at mem_loc to the display at position
 * (V[vx], V[vy])
 *
 ***************************************************************/

int c8_draw_sprite(int mem_loc, int num_bytes, int x, int y) {
  /* read num_bytes bytes from the mem_loc and copy them to
   * V[vx], V[vy]. Each byte moves down one row.
   * Set VF == 1 if there are any collisions. 
   * 
   * The sprites should be
   * XORed together. If the result of this XOR is 0 it means there
   * was a collision.
   */

  /* TODO: The sprites are supposed to wrap around to the other side of the 
   * screen if they go passed the boundaries of the screen.
   */
  int i;
  unsigned char read_byte, cur_bit;
  int bit_counter;
  unsigned char result;
  
  if(mem_loc + num_bytes > C8_MAX_RAM) {
    return 0;
  }
  
  for(i = 0; i < num_bytes; i++) {
    read_byte = c8_ram.memory[mem_loc + i]; /* get the byte from memory */
   
    for(bit_counter = 0; bit_counter < 8; bit_counter++) {
      /* shift the byte 1 bit at a time starting on the left, then mask to get the LSB */
      cur_bit = (read_byte >> (7 - bit_counter)) & 0x01;

      /* check if the bit is actually set. If it isn't, is there
       * a reason to copy it to the display?
       * this might mess up collision detection.
       */
      result =  c8_display[y + i][x + bit_counter] ^ cur_bit;
      c8_display[y + i][x + bit_counter] = result;

      if(result == 0) { /* collision detected */	  
	registers.V[0x0F] = 1;
      }

      /*
      if(cur_bit) {
	result = c8_display[y + i][x + bit_counter] ^ cur_bit;
	
	if(result == 0) { /* collision detected */
      /*
	  registers.V[0x0F] = 1;
	}
      }else{
	result = 0;
      }
      c8_display[y + i][x + bit_counter] = result;*/
    }
  }
  return 1;
}

void c8_clear_screen(void) {
  /* set the video memory to all 0s */
  int row, col;
  
  for(row = 0; row < C8_DISPLAY_HEIGHT; row++) {
    for(col = 0; col < C8_DISPLAY_WIDTH; col++) {
      c8_display[row][col] = 0;
    }
  }
}

