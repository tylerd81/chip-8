#include "chip8.h"
#include <string.h>
#include <stdio.h>

FILE *log_file;

FILE *init_c8_log(char *filename) {
  log_file = fopen(filename, "w");
  return log_file;
}

void c8_log_write(char *str) {
  fwrite(str, sizeof(char), strlen(str), log_file);
}

void c8_log_close(void) {
  fclose(log_file);
}
