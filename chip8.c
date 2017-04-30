/*******************************************************
 * Chip 8 Interpreter
 * Tyler D Started 4/29/2017
 *******************************************************/
#include <stdio.h>
#include <string.h>

#define C8_DISPLAY_HEIGHT 32
#define C8_DISPLAY_WIDTH 64
/*******************************************************
 * Chip 8 Registers
 *******************************************************/
struct {
  unsigned char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
  int I;
  int PC; /* program counter */
  int SP; /* stack pointer, only 8 bit but doesn't matter, will be index into stack (0-15) */
  int stack[16];

  int DT; /* delay timer */
  int ST; /* sound timer */
}registers;

/*******************************************************
 * Chip 8 RAM
 *******************************************************/
struct {
  char memory[4096];
  int used_bytes;
  int total_mem;
} c8_ram;

/*******************************************************
 * Chip 8 Display
 * Each element of the array is either 1 or 0. The
 * display is monochrome so there are only two possible
 * values.
 *******************************************************/
char c8_display[C8_DISPLAY_HEIGHT][C8_DISPLAY_WIDTH];

/*******************************************************
 * Prototypes
 *******************************************************/
void c8_start(void);
int c8_decode_instructions(int instr);
int c8_jump(int addr);
int c8_ret(void);
void show_registers(void);

/*******************************************************
 * main
 *******************************************************/
int main(void) {
  printf("Welcome To Tyler's Chip-8 Interpreter!\n");
  printf("char: %d\n", sizeof(char));
  c8_start();
  c8_decode_instructions(0x1666);
  show_registers();
  return 0;
}

/*******************************************************
 * C8_start()
 * Initializes all the registers and the main memory.
 * Must be called first.
 *******************************************************/
void c8_start(void) {

  printf("C8 Initializing...\n");

  /* initialize all the registers */
  registers.V0 = 0;
  registers.V1 = 0;
  registers.V2 = 0;
  registers.V3 = 0;
  registers.V4 = 0;
  registers.V5 = 0;
  registers.V6 = 0;
  registers.V7 = 0;
  registers.V8 = 0;
  registers.V9 = 0;
  registers.VA = 0;
  registers.VB = 0;
  registers.VC = 0;
  registers.VD = 0;
  registers.VE = 0;
  registers.VF = 0;

  registers.I = 0;
  registers.PC = 0;
  registers.SP = 0;

  registers.DT = 0;
  registers.ST = 0;

  for(int i = 0; i < 16; i++) {
      registers.stack[i] = 0;
  }

  //setup the memory
  //init all to 0 or not?
  c8_ram.used_bytes = 512; //space for the interpreter
  c8_ram.total_mem = 4096;
  memset(c8_ram.memory, 0, c8_ram.total_mem);

  printf("Initialization complete.\n");
}

/*******************************************************
 * c8_decode_instructions(int instr)
 * The instructions are 16 bits, so we only need the
 * lower 16 bits of the int.
 *******************************************************/
int c8_decode_instructions(int instr) {
  //mask is 1111 0000 0000 0000b for getting bit 15
  int op_flag = 0xF000 & instr;
  int low_4bits = 0x0FFF;

  switch(op_flag) {
    case 0x1000:
      printf("JP instruction\n");
      registers.PC = instr & low_4bits;
      break;
    default:
      printf("Instruction: 0x%04X NYI.\n", op_flag);
      break;
    }
    return op_flag;
}

/*******************************************************
 * c8_jump(int addr)
 *******************************************************/
int c8_jump(int addr) {
  return 1;
}

/*******************************************************
 * c8_ret()
 *******************************************************/
int c8_ret(void) {
  return 1;
}

/*******************************************************
 * show_registers()
 * Displays all the register values.
 *******************************************************/
void show_registers(void) {
  printf("(V0-VF)\n");

  printf("[V0: 0x%02X] [V1: 0x%02X] [V2: 0x%02X] [V3: 0x%02X]\n",
          registers.V0, registers.V1, registers.V2, registers.V3);

  printf("[V4: 0x%02X] [V5: 0x%02X] [V6: 0x%02X] [V7: 0x%02X]\n",
          registers.V4, registers.V5, registers.V6, registers.V7);

  printf("[V8: 0x%02X] [V9: 0x%02X] [VA: 0x%02X] [VB: 0x%02X]\n",
          registers.V8, registers.V9, registers.VA, registers.VB);

  printf("[VC: 0x%02X] [VD: 0x%02X] [VE: 0x%02X] [VF: 0x%02X]\n",
          registers.VC, registers.VD, registers.VE, registers.VF);

  printf("\n(Timers)\n[DT: 0x%02X] [ST: 0x%02X]\n", registers.DT,
         registers.ST);

  printf("\n(Instruction Registers)\n");
  printf("[I: 0x%03X]\n", registers.I);
  printf("[SP: 0x%02X]\n", registers.SP);
  printf("[PC: 0x%04X]\n", registers.PC);

  printf("\n(Memory)\n");
  printf("Free Space: %d bytes.\n", c8_ram.total_mem - c8_ram.used_bytes);
}
