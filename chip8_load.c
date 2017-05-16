#include <stdio.h>
#include <string.h>
#include "chip8.h"

/***************************************
 * c8_load_from_file()
 * Loads the Chip 8 instructions from the
 * file into memory starting at location
 * 0x0500. It then sets the PC to 0x500
 ***************************************/
int c8_load_from_file(char *filename) {
  FILE *fp;

  int bytes_read = 0;
  unsigned short num_bytes;
  unsigned char *mem_ptr = &c8_ram.memory[START_LOAD_ADDRESS];  
    
  fp = fopen(filename, "rb");
  if(fp == NULL) {
    printf("Error opening %s\n", filename);
    return 0;
  }
  
  /* first 16-bits is the number of bytes to read */
  fread(&num_bytes, sizeof(unsigned short), 1, fp);

  /* fread reads the LSB first and then the MSB, so we swap them. */
  num_bytes = (num_bytes << 8) | (num_bytes >> 8);
    
  /* read the data into memory */
  bytes_read = fread(mem_ptr, sizeof(char), num_bytes, fp);

  fclose(fp);

  registers.PC = START_LOAD_ADDRESS;

  return bytes_read;
}

int c8_load_bin(char *filename, int num_bytes) {
  FILE *fp;

  int bytes_read = 0;

  unsigned char *mem_ptr = &c8_ram.memory[START_LOAD_ADDRESS];  
    
  fp = fopen(filename, "rb");
  if(fp == NULL) {
    printf("Error opning %s\n", filename);
    return 0;
  }
  
  /* read the data into memory */
  bytes_read = fread(mem_ptr, sizeof(char), num_bytes, fp);

  fclose(fp);

  registers.PC = START_LOAD_ADDRESS;

  return bytes_read;
}

int c8_load_fonts(void) {
  unsigned char fonts[] = {0xf0, 0x90, 0x90, 0x90, 0xf0, 0x00, /* 0 */
			   0x20, 0x60, 0x20, 0x20, 0x70, 0x00, /* 1 */
			   0xf0, 0x10, 0xf0, 0x80, 0xf0, 0x00, /* 2 */
			   0xf0, 0x10, 0xf0, 0x10, 0xf0, 0x00, /* 3 */
			   0x90, 0x90, 0xf0, 0x10, 0x10, 0x00, /* 4 */
			   0xf0, 0x80, 0xf0, 0x10, 0xf0, 0x00, /* 5 */
			   0xF0, 0x80, 0xF0, 0x90, 0xF0, 0x00, /* 6 */
			   0xF0, 0x10, 0x20, 0x40, 0x40, 0x00, /* 7 */
			   0xF0, 0x90, 0xF0, 0x90, 0xF0, 0x00, /* 8 */
			   0xF0, 0x90, 0xF0, 0x10, 0xF0, 0x00, /* 9 */
			   0xF0, 0x90, 0xF0, 0x90, 0x90, 0x00, /* A */
			   0xE0, 0x90, 0xE0, 0x90, 0xE0, 0x00, /* B */
			   0xF0, 0x80, 0x80, 0x80, 0xF0, 0x00, /* C */
			   0xE0, 0x90, 0x90, 0x90, 0xE0, 0x00, /* D */
			   0xF0, 0x80, 0xF0, 0x80, 0xF0, 0x00, /* E */
			   0xF0, 0x80, 0xF0, 0x80, 0x80, 0x00  /* F */
  };
  
  int start_addr = FONT_ADDRESS;

  memcpy(&c8_ram.memory[start_addr], fonts, 96);  
  return 1;
}
  
