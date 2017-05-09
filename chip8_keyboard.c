#include <stdio.h>
#include "chip8.h"

/*******************************************************
 * Chip 8 Keyboard
 * Each value in the array is either 0 or 1, 1 if the key
 * is pressed and 0 if it is not. There are sixteen elements
 * in the array, one for each key. 0-9 and then A-F are 
 * in elements 10-15.
 *******************************************************/

int c8_keyboard[C8_NUM_KEYS];

int c8_init_keyboard(void) {
  /* no keys are pressed to begin with */
  int i;

  for(i = 0; i < C8_NUM_KEYS; i++) {
    c8_keyboard[i] = 0;
  }
  return 1;     
}

int c8_keydown(int keynum) {

  if(keynum < 0 || keynum > C8_NUM_KEYS)
    return 0; /* invalid key */
  
  c8_keyboard[keynum] = 1;
  return 1;
}

int c8_keyup(int keynum) {
  if(keynum < 0 || keynum > C8_NUM_KEYS)
    return 0;

  c8_keyboard[keynum] = 0;
  return 1;
}

/* c8_get_key_state() function returns 0 if the key is not pressed
 * or 1 if the key is pressed.
 */
int c8_get_key_state(int keynum) {
  if(keynum < 0 || keynum > C8_NUM_KEYS)
    return -1; /* invalid key */

  return c8_keyboard[keynum];
}

void dump_key_state(void) {
  int i;
  for(i = 0; i < C8_NUM_KEYS; i++){
    printf("Key: %d State: %s\n", i, c8_keyboard[i] ? "pressed": "not pressed");
  }
}
