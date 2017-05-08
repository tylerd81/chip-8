/*******************************************************
 * Chip 8 Interpreter
 * Tyler D Started 4/29/2017
 *******************************************************/
#include <stdio.h>
#include <string.h>
#include "chip8.h"

/* globals */
struct _registers registers;
int stack[16];
struct _c8_ram c8_ram;
c8_status c8_state;
unsigned char c8_display[C8_DISPLAY_HEIGHT][C8_DISPLAY_WIDTH];

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

  printf("Got instruction 0x%04X\n", instr);
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
  unsigned int low_4bits = 0x00FF;
  int address;
  
  switch(op_flag) {
    
  case 0x0000: /* 0x00EE - Ret 0x00E0 - CLS */
    if((instr & low_4bits) == 0xEE) {     
      c8_ret();
    }else if((instr & low_4bits) == 0xE0){
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
    c8_se(instr & 0x0F00, instr & low_4bits);
    break;

  case 0x4000: /* 4xkk - SNE Vx, byte - skips next instruction if V[x] != byte */
    c8_sne(instr & 0x0F00, instr & low_4bits);
    break;

  case 0x5000: /* 5xy0 - SE Vx, Vy - if V[x] == V[y] it skips next instruction */
    c8_se_vx_vy(instr & 0x0F00, instr & 0x00F0);
    break;
    
  case 0x6000: /* 6xkk - LD Vx, byte */
    c8_ld( ((instr & 0x0F00)>>8), instr & low_4bits);
    break;

  case 0x7000: /* 7xkk - ADD - V[x] = V[x] + kk  */
    c8_add(instr & 0x0F00, instr & low_4bits);
    break;
    
  case 0xF000:
    c8_set_state(DEAD);
    return -1;
    
  default:
    break;
  }
  return op_flag;
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

