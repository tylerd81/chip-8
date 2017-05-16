#include <stdio.h>
#include "chip8.h"

_c8_timer_status c8_timer_status = TIMER_STOPPED;

int is_timer_running(void) {
  if(c8_timer_status == TIMER_RUNNING) {
    return 1;
  }else{
    return 0;
  }
}

void c8_timer_tick(void) {
  if(is_timer_running()) {
    if(registers.DT > 0) {
      registers.DT--;
    }else{
      /* DT has reached 0 */
      c8_timer_status = TIMER_STOPPED;
    }
  }
}


  
