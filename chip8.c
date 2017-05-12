/*******************************************************
 * Chip 8 Interpreter
 * Tyler D Started 4/29/2017
 *******************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

/* globals */
struct _registers registers;
int stack[16];
struct _c8_ram c8_ram;
c8_status c8_state;
unsigned char c8_display[C8_DISPLAY_HEIGHT][C8_DISPLAY_WIDTH];

/* keypress_register will hold the register number that the 
 * keypress should be saved in while in WAITING_FOR_KEY state
 */
int keypress_register;

/*******************************************************
 * main
 *******************************************************/
/*
int main(void) {
  unsigned int instr;
  int status = 1;
  
  printf("*****************************************\n");
  printf("* Welcome To Tyler's Chip-8 Interpreter!*\n");
  printf("*****************************************\n");
  
  c8_start();

  c8_test();
  
  
  return 0;
  
}
*/
/*******************************************************
 * C8_start()
 * Initializes all the registers and the main memory.
 * Must be called first.
 *******************************************************/
void c8_start(void) {

  int i = 0;

  init_c8_log("chip8.log");

  c8_log_write("C8 initializing...\n");

  /* initialize all the registers */
  for(i = 0; i < 16; i++) {
    registers.V[i] = 0;
  }
  
  registers.I = 0;
  registers.PC = 0;
  registers.SP = 0;

  registers.DT = 0;
  registers.ST = 0;

  for(int i = 0; i < 16; i++) {
      stack[i] = 0;
  }

  //setup the memory
  //init all to 0 or not?
  c8_ram.used_bytes = 512; //space for the interpreter
  c8_ram.total_mem = 4096;
  memset(c8_ram.memory, 0, c8_ram.total_mem);

  c8_init_keyboard();
  c8_load_fonts();
  
  c8_state = RUNNING;
  
  c8_log_write("Initialization complete.\n");
}

void c8_set_state(c8_status state) {
  c8_state = state;
}
   
void c8_quit(void) {
  c8_log_close();
}

void c8_test(void) {
  /* 
  c8_ram.memory[0] = 0xF0;
  c8_ram.memory[1] = 0x90;
  c8_ram.memory[2] = 0x90;
  c8_ram.memory[3] = 0x90;
  c8_ram.memory[4] = 0xF0;

  c8_ram.memory[5] = 0xF0;
  c8_ram.memory[6] = 0x10;
  c8_ram.memory[7] = 0xF0;
  c8_ram.memory[8] = 0x10;
  c8_ram.memory[9] = 0xF0;
  
  c8_clear_screen();
  
  c8_draw_sprite(0, 5, 0, 0);
  c8_draw_sprite(5, 5, 3, 0);
  */

  c8_ram.memory[0] = 0x60;
  c8_ram.memory[1] = 0x12; /* put 0x12 into V0 */

  c8_ram.memory[2] = 0x70;
  c8_ram.memory[3] = 0x05; /* add 0x05 to V0 */

  c8_ram.memory[4] = 0xFF;
  c8_ram.memory[5] = 0xFF;

  registers.PC = 0;
  
    
}
/*******************************************************
 * int c8_fetch_instruction()
 * Gets the instruction from memory that the PC is
 * currently pointing to. It increments PC after getting
 * the instruction.
 *
 *******************************************************/
unsigned int c8_fetch_instruction() {
  /* each instruction is 2bytes */
  /* read the first 8 bits into the 16 bit int.
   * then shift it left 8 bits so we have:
   * nnnnnnnnn00000000 where n is the value we 
   * just read.
   * Read next 8 bits and then OR the values together:
   * nnnnnnnnn00000000 | 00000000nnnnnnnn = nnnnnnnnnnnnnnnn
   */


  unsigned int instr = 0;
  unsigned char b1, b2;
  b1 = c8_ram.memory[registers.PC++];
  b2 = c8_ram.memory[registers.PC++];

  instr = b1;
  instr <<= 8;
  instr |= b2;

  /*  printf("Got instruction 0x%04X\n", instr);*/
  /* each instruction is supposed to be located at an even address */
  return instr;
}

/*******************************************************
 * c8_decode_instructions(int instr)
 * The instructions are 16 bits, so we only need the
 * lower 16 bits of the int.
 *
 * Remember that the chip 8 uses 12 bit addresses for a 
 * total of 4096 bytes.
 *
 *******************************************************/
int c8_decode_instruction(unsigned int instr) {
  //mask is 1111 0000 0000 0000b for getting bit 15
  unsigned int op_flag = 0xF000 & instr;
  unsigned int low_12bits = 0x0FFF;  
  unsigned int low_8bits = 0x00FF;
  unsigned int low_4bits = 0x000F;
  unsigned int x, y, op;
  
  int address;
  
  switch(op_flag) {
    
  case 0x0000: /* 0x00EE - Ret 0x00E0 - CLS */
    if((instr & low_8bits) == 0xEE) {     
      c8_ret();
    }else if((instr & low_8bits) == 0xE0){
      c8_cls();
    }
    break;
    
  case 0x1000: /* 1nnn - JP addr */
    c8_jump(instr & low_12bits);
    break;

  case 0x2000: /* 2nnn - Call addr */
    c8_call(instr & low_12bits);
    break;

  case 0x3000: /* 3xkk - SE Vx, byte - Skips instruction */
    c8_se((instr & 0x0F00) >> 8, instr & low_8bits);
    break;

  case 0x4000: /* 4xkk - SNE Vx, byte - skips next instruction if V[x] != byte */
    c8_sne((instr & 0x0F00) >> 8, instr & low_8bits);
    break;

  case 0x5000: /* 5xy0 - SE Vx, Vy - if V[x] == V[y] it skips next instruction */
    c8_se_vx_vy((instr & 0x0F00) >> 8, (instr & 0x00F0) >> 4);
    break;
    
  case 0x6000: /* 6xkk - LD Vx, byte */
    c8_ld( ((instr & 0x0F00)>>8), instr & low_8bits);
    break;

  case 0x7000: /* 7xkk - ADD - V[x] = V[x] + kk  */
    c8_add((instr & 0x0F00) >> 8, instr & low_8bits);
    break;

  case 0x8000: /* 8xy0 and 8xy1 */
    op = instr & low_4bits;
    x = (instr & 0x0F00) >> 8;
    y = (instr & 0x00F0) >> 4;
    
    if(op == 0x0000) {
      /* 8xy0 LD Vx, Vy - stores Vy in Vx */
      c8_ld_vx_vy(x,y);
      
    }else if(op == 0x0001) {
      /* 8xy1 OR Vx, Vy - OR's Vx and Vy and stores result in Vx */
      c8_or_vx_vy(x,y);

    }else if(op == 0x0002) {
      /* 8xy2 AND Vx, Vy */
      c8_and_vx_vy(x,y);
      
    }else if(op == 0x0003) {
      /* 8xy3 XOR Vx, Vy */
      c8_xor_vx_vy(x,y);
      
    }else if(op == 0x0004) {
      /* 8xy4 Add Vx, Vy */
      
      c8_add_vx_vy(x,y);

    }else if(op == 0x0005) {
      /* 8xy5 SUB Vx, Vy */
      c8_sub_vx_vy(x,y);

    }else if(op == 0x0006) {
      /* 8xy6 - SHR Vx (Not sure if you are supposed to implement the {, VY part} */
      c8_shr_vx(x);
      
    }else if(op == 0x0007) {
      /* 8xy7 SUBN Vx, Vy */
      c8_subn_vx_vy(x, y);
    }else if(op == 0x000E) {
      /* 8xyE SHL Vx */
      c8_shl_vx(x);

    }else{
      printf("Unknown instruction. 0x%04X\n", instr & low_4bits);
    }
    break;    
    
  case 0x9000:
    /* 9xy0 - SNE Vx, Vy */
    x = (instr & 0x0F00) >> 8;
    y = (instr & 0x00F0) >> 4;
    c8_sne_vx_vy(x, y);
    break;

  case 0xA000:
    /* Annn LD I, addr */
    c8_ld_i(instr & low_12bits);
    break;

  case 0xB000:
    /* Bnnn - JP V0, addr */
    c8_jp_v0(instr & low_12bits);
    break;

  case 0xC000:
    /* Cxkk - RND Vx, byte */
    x = (instr & 0x0F00) >> 8;
    op = (instr & low_8bits);
    c8_rnd(x, op);
    break;

  case 0xD000:
    /* Dxyn - DRW Vx, Vy, nibble */
    x = (instr & 0x0F00) >> 8;
    y = (instr & 0x00F0) >> 4;
    op = instr & low_4bits;
    c8_drw(x,y,op);
    break;

  case 0xE000:
    op = instr & low_8bits;
    x = (instr & 0x0F00) >> 8;
    
    /* Ex9E */
    if(op == 0x9E) { /* SKP Vx */
      c8_skp_vx(x);
    }else if(op == 0xA1) { /* SKNP Vx */
      /* ExA1 */
      c8_sknp_vx(x);
    }
    break;

  case 0xF000:
    op = instr & low_8bits;
    x = (instr & 0x0F00) >> 8;

    if(op == 0x07) {
      c8_ld_vx_dt(x);
      
    }else if(op == 0x0A) {
      c8_wait_for_keypress(x);
      
    }else if(op == 0x15) {
      c8_ld_dt_vx(x);
      
    }else if(op == 0x18) {
      c8_ld_st_vx(x);

    }else if(op == 0x1E) {      
      c8_add_i_vx(x);
      
    }else if(op == 0x29) {
      c8_ld_f_vx(x); /* load the location of the sprite into I */
      
    }else if(op == 0x33) {
      c8_ld_b_vx(x); /* Store the BCD value at mem address I */
      
    }else if(op == 0x55) {
      c8_ld_i_vx(x); /* store V0 - Vx at mem address I */
      
    }else if(op == 0x65) {
      c8_ld_vx_i(x); /* read values at mem address I into V0 - Vx */
    }
    
    if((instr & 0xFFFF) == 0xFFFF) {
      c8_set_state(DEAD);    
      return -1;
    }
    break;
    
  default:
    printf("Unknown Instruction\n");	   
    break;
  }
  return op_flag;
}

/***************************************************
 * c8_add_i_vx(int x)
 * add I + V[x] and store the results in I 
 *
 ***************************************************/
int c8_add_i_vx(int x) {
  registers.V[x] += registers.I;
  return 1;
}

/***************************************************
 * c8_ld_f_vx(int x)
 * Sets I equal to the memory address of sprite for
 * the value that is store in V[x].
 ***************************************************/
int c8_ld_f_vx(int x) {
  registers.I = FONT_ADDRESS + (registers.V[x] * (FONT_WIDTH * FONT_HEIGHT));
  /*  printf("Font At: 0x%04X\n", registers.I);*/
  return 1;
}

/***************************************************
 * Loads memory address I, I+1, and I+2 with the
 * BCD value of V[x].
 *
 ***************************************************/
int c8_ld_b_vx(int x) {
  unsigned char h,t,o; /* hundreds, tens, ones */
  int ptr_ctr;
    
  h = x / 100;
  x %= 100;
  t = x / 10;
  o = x % 10;

  c8_ram.memory[registers.I] = h;
  c8_ram.memory[registers.I + 1] = t;
  c8_ram.memory[registers.I + 2] = o;
  

  return 1;
}

/***************************************************
 * LD [I], Vx
 * Store the values in V0 - V[x] into memory starting
 * at I.
 ***************************************************/
int c8_ld_i_vx(int x) {
  int i;

  for(i = 0; i < x; i++) {
    c8_ram.memory[registers.I + i] = registers.V[i];
  }
  return 1;
}

/******************************************************
 * LD V[x], [I]
 * Load V0 through V[x] with the values starting at 
 * memory address I.
 *
 ******************************************************/
int c8_ld_vx_i(int x) {
  int i = 0;

  for(i = 0; i < x; i++) {
    registers.V[x] = c8_ram.memory[registers.I + i];
  }
  return 1;
}

/*******************************************************
 * c8_ld_vx_dt(int x)
 * Loads the value in the DT register into V[x]
 *******************************************************/
int c8_ld_vx_dt(int x) {
  registers.V[x] = registers.DT;
  return 1;
}

/*******************************************************
 * c8_wait_for_keypress(int x)
 * This function will set the state of the emulator to 
 * WAITING_FOR_KEY. It will also set the keypress_register
 * value to x.
 *******************************************************/
int c8_wait_for_keypress(int x) {
  c8_set_state(WAITING_FOR_KEY);
  printf("Waiting for keypress...\n");
  keypress_register = x;
  return 1;
}

int c8_got_keypress(int key) {
  printf("got keypress...\n");
  registers.V[keypress_register] = key;
  
  /*registers.PC += 2;*/
  
  c8_set_state(RUNNING);
  return 1;
}

/*******************************************************
 * c8_ld_dt_vx(int x)
 ******************************************************/
int c8_ld_dt_vx(int x) {
  return 1;
}

/*******************************************************
 * c8_ld_st_vx(int x)
 *******************************************************/
int c8_ld_st_vx(int x) {
  return 1;
}

/*******************************************************
 * c8_skp_vx(int key)
 * Skip the next instruction if the key is pressed.
 *******************************************************/
int c8_skp_vx(int key) {
  if(c8_get_key_state(key) == 1) {
    registers.PC += 2;
  }
  return 1;
}

/*******************************************************
 * c8_sknp_vx(int key)
 * Skip the next instruction if the key is NOT pressed.
 *******************************************************/
int c8_sknp_vx(int key) {
  if(c8_get_key_state(key) == 0) {
    registers.PC += 2;
  }
  return 1;
}

/*******************************************************
 * c8_drw(int x, int y, int num_bytes)
 * Draw the sprite located in memory location I.
 * V[x], V[y] is the (x,y) location on the screen.
 *******************************************************/
int c8_drw(int x, int y, int num_bytes) {
  c8_draw_sprite(registers.I, num_bytes, registers.V[x], registers.V[y]);
  return 1;  
}

/*******************************************************
 * c8_rnd(int x, int b)
 * Generate a random number from 0 to 255 and then AND
 * that value with b. The result is store in V[x]
 *******************************************************/
int c8_rnd(int x, int b) {
  int r;
  srand(time(NULL));

  r = rand()%255;
  r &= b;
  
  registers.V[x] = r;
  return 1;
}
/*******************************************************
 * c8_jp_v0(int addr)
 * Sets PC to V0 + addr
 *******************************************************/
int c8_jp_v0(int addr) {
  registers.PC = registers.V[0] + addr;
  return 0;
}

/*******************************************************
 * c8_ld_i(unsigned int i) 
 * Sets I = to i
 *******************************************************/
int c8_ld_i(unsigned int i) {
  registers.I = i;
  return 1;
}

/*******************************************************
 * c8_sne_vx_vy(int x, int y)
 * Skips the next instruction if Vx != Vy
 *******************************************************/
int c8_sne_vx_vy(int x, int y) {
  if(registers.V[x] != registers.V[y]) {
    registers.PC += 2;
  }
  return 1;
}

/*******************************************************
 * c8_shl_vx(int x)
 * Shift V[x] left 1 bit and put the bit in VF
 *******************************************************/
int c8_shl_vx(int x) {
  registers.V[0x0F] = registers.V[x] & 0x80;
  registers.V[x] <<= 1;
  return 1;
}
//0111 1110 VF=0
/*******************************************************
 * c8_subn_vx_vy(int x, int y)
 *
 * Subtracts Vx from Vy and stores the result in Vx
 *******************************************************/
int c8_subn_vx_vy(int x, int y) {
  if(registers.V[y] > registers.V[x]) {
    registers.V[0x0F] = 1;
  }else{
    registers.V[0x0F] = 0;
  }

  registers.V[x] = registers.V[y] - registers.V[x];
  return 1;
}
/*******************************************************
 * c8_shr_vx(int x)
 * Shift Vx to the right 1 bit.
 * The bit that is shifted out is set to VF
 *
 * &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
 * There might be a bug here, the doc says what 
 * looks like an optional Vy
 * &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
 *******************************************************/
int c8_shr_vx(int x) {
  registers.V[0x0F] = 0x01 & registers.V[x];
  registers.V[x] >>= 1;
  return 1;
}

/*******************************************************
 * c8_xor_vx_vy(int x, int y)
 * XOR V[x] and V[y] and store the value in V[x]
 *******************************************************/
int c8_xor_vx_vy(int x, int y) {
  registers.V[x] ^= registers.V[y];
  return 1;
}

/*******************************************************
 * c8_add_vx_vy(int x, int y)
 * add V[x] and V[y] and store the value in V[x]. If the
 * result is greater than 255 then VF is set to 1. Only 
 * the lower 8 bits of the result are used.
 *******************************************************/
int c8_add_vx_vy(int x, int y) {
  unsigned short result;

  result = registers.V[x] + registers.V[y];
  if(result > 255) {
    registers.V[0x0F] = 1;
  }else{
    registers.V[0x0F] = 0;
  }

  registers.V[x] = result & 0x00FF;
  return 1;
}

/*******************************************************
 * c8_sub_vx_vy(int x, int y)
 * if Vx > Vy then VF is set to 1, otherwise 0. 
 * Vx - Vy and then result is store in Vx
 *******************************************************/
int c8_sub_vx_vy(int x, int y) {

  if(registers.V[x] > registers.V[y]) {
    registers.V[0x0F] = 1;
  }else{
    registers.V[0x0F] = 0;
  }

  registers.V[x] -= registers.V[y];
  return 1;
}
  
/*******************************************************
 * c8_and_vx_vy(int x, int y)
 * AND V[x] and V[y] and store the value in V[x]
 *******************************************************/
int c8_and_vx_vy(int x, int y) {
  registers.V[x] = registers.V[x] & registers.V[y];
  return 1;
}

/*******************************************************
 * c8_ld_vx_vy(int x, int y)
 * Stores the value V[y] in the V[x] register.
 *******************************************************/
int c8_ld_vx_vy(int x, int y) {
  registers.V[x] = registers.V[y];
  return 1;
}

/*******************************************************
 * c8_or_vx_vy(int x, int y)
 * OR the two registers and store the value in V[x]
 *******************************************************/
int c8_or_vx_vy(int x, int y) {
  registers.V[x] = registers.V[x] | registers.V[y];
  return 1;
}

/*******************************************************
 * c8_add()
 * Adds the value in k to V[x] and stores the sum in V[x]
 *
 *******************************************************/
int c8_add(int x, unsigned char k) {
  registers.V[x] += k;
  return registers.V[x];
}

/*******************************************************
 * c8_se_vx_vy()
 * Uses the register to check if it should skip next 
 * instruction. If V[x] == V[y] skip next instruction
 *
 *******************************************************/
int c8_se_vx_vy(int x, int y) {
  if(registers.V[x] == registers.V[y]) {
    registers.PC += 2;
  }
  return 1;
}

/*******************************************************
 * c8_sne()
 * SNE skips next instruction if V[x] != k
 *
 *******************************************************/
int c8_sne(int x, unsigned char k) {
  if(registers.V[x] != k) {
    registers.PC += 2;
  }
  return 1;
}

/*******************************************************
 * c8_se()
 * SE skips the next instruction is V[x] == k
 *
 *******************************************************/
int c8_se(int x, unsigned char k) {
  if(registers.V[x] == k) {
    registers.PC += 2;
  }

  return 1;
}

/*******************************************************
 * c8_ld()
 *
 * LD Vx, byte
 * Loads register Vx with byte
 *
 * 6xkk
 *
 *******************************************************/
int c8_ld(int x, unsigned char b) {
  registers.V[x] = b;
  return x;
}

/*******************************************************
 * c8_call(int address)
 *
 * 2nnn - Call addr
 * Increment stack pointer and then put the current PC
 * on the stack. Then set PC to address.
 *
 *******************************************************/
int c8_call(int address) {
  char log_str[LOG_STR_LEN];
  
  /* check if stack is going to overflow */
  if(registers.SP == 15) {
    snprintf(log_str, LOG_STR_LEN, "Stack overflow. Exiting.\n");
    return 0;
  }

  registers.SP++;
  stack[registers.SP] = registers.PC; /* save old PC */
  registers.PC = address;
  return 1;
}

/*******************************************************
 * c8_jump(int addr)
 * 1nnn - JP addr
 * Sets PC to addr (doesn't actually save anything on the
 * stack)
 *
 *******************************************************/
int c8_jump(int addr) {
  registers.PC = addr;
  return 1;
}

/*******************************************************
 * c8_ret()
 * 00EE - RET
 * Sets PC to the value at the top of the stack and then
 * decrements SP.
 *
 *******************************************************/
int c8_ret(void) {

  registers.PC = stack[registers.SP];

  if(registers.SP > 0) {
    registers.SP--;
  }else{
    c8_log_write("Attempted to pop something off the stack when nothing was there. c8_ret()\n");
  }
  return 1;
}

void c8_cls(void) {
  c8_log_write("Clearing the screen....\n");
}

/*******************************************************
 * show_registers()
 * Displays all the register values.
 *******************************************************/
void show_registers(void) {

  char state[20];

  switch(c8_state) {
  case RUNNING:
    sprintf(state, "Running");
    break;
  case PAUSED:
    sprintf(state, "Paused");
    break;

  case DEAD:
    sprintf(state, "Dead");
    break;
  case WAITING_FOR_KEY:
    sprintf(state, "Waiting for key");
    break;
  }
  
  printf("-------------------------------------------------------\n");
  printf("State: %s\n", state);
  printf("| (V0-VF)                                             |\n");

  printf("| [V0: 0x%02X] [V1: 0x%02X] [V2: 0x%02X] [V3: 0x%02X]         |\n",
	 registers.V[0], registers.V[1], registers.V[2], registers.V[3]);

  printf("| [V4: 0x%02X] [V5: 0x%02X] [V6: 0x%02X] [V7: 0x%02X]         |\n",
	 registers.V[4], registers.V[5], registers.V[6], registers.V[7]);

  printf("| [V8: 0x%02X] [V9: 0x%02X] [VA: 0x%02X] [VB: 0x%02X]         |\n",
	 registers.V[8], registers.V[9], registers.V[0xA], registers.V[0xB]);

  printf("| [VC: 0x%02X] [VD: 0x%02X] [VE: 0x%02X] [VF: 0x%02X]         |\n",
	 registers.V[0xC], registers.V[0xD], registers.V[0xE], registers.V[0xF]);

  printf("|=====================================================|\n");
  printf("|(Timers)                                             |\n");
  printf("| [DT: 0x%02X] [ST: 0x%02X]                               |\n",
	 registers.DT, registers.ST);
  printf("|=====================================================|\n");
  
  printf("|(Instruction Registers)                              |\n");
  printf("| [I: 0x%03X]                                          |\n", registers.I);
  printf("| [SP: 0x%02X]                                          |\n", registers.SP);
  printf("| [PC: 0x%04X]                                        |\n", registers.PC);
  printf("|=====================================================|\n");
  
  printf("|(Memory)                                             |\n");
  printf("| Free Space: %d bytes.                               \n",
	 c8_ram.total_mem - c8_ram.used_bytes);
  printf("------------------------------------------------------\n");
}

void dump_stack(void) {
  for(int i = 0; i < 16; i++) {
    if(i == registers.SP) {
      printf("*");
    }

    printf("{%d|0x%03X}\n", i, stack[i]);
  }
}

void dump_mem(int start, int num_bytes) {
  int i;

  if(start + num_bytes > 4096) {
    num_bytes -= (start + num_bytes) - 4096;
  }
  
  for(int i = 0; i < num_bytes; i++) {
    printf("0x%02X: 0x%02X\n", start + i, c8_ram.memory[start+i]);
  }
}

