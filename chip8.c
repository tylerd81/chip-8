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
  unsigned char V[16];
  int I;
  int PC; /* program counter */
  int SP; /* stack pointer, only 8 bit but doesn't matter, will be index into stack (0-15) */
    
  int DT; /* delay timer */
  int ST; /* sound timer */
}registers;

/*******************************************************
 * The Stack 
 *
 *******************************************************/
int stack[16];

/*******************************************************
 * Chip 8 RAM
 *******************************************************/
struct {
  unsigned char memory[4096];
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
unsigned int c8_fetch_instruction();
int c8_call(int address);
int c8_decode_instructions(unsigned int instr);
int c8_jump(int addr);
int c8_ret(void);
void c8_cls(void);
int c8_ld(int x, unsigned char b);
void show_registers(void);
void c8_test(void);
void dump_stack(void);

/*******************************************************
 * main
 *******************************************************/
int main(void) {
  unsigned int instr;
  int status = 1;
  
  printf("*****************************************\n");
  printf("* Welcome To Tyler's Chip-8 Interpreter!*\n");
  printf("*****************************************\n");
  
  c8_start();

  c8_test();
  
  while(status >= 0) {
    instr = c8_fetch_instruction();  
    status = c8_decode_instructions(instr);
    show_registers();
    /*  dump_stack();*/
  }

  printf("Status: %d\n", status);
  return 0;
  
}

/*******************************************************
 * C8_start()
 * Initializes all the registers and the main memory.
 * Must be called first.
 *******************************************************/
void c8_start(void) {

  int i = 0;
  printf("C8 Initializing...\n");

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

  printf("Initialization complete.\n");
}

void c8_test(void) {

  /* 0x220C = call to 0x20C */
  int mem_ptr = 502;
  int reg_ptr = 0x60;
  int i;
  
  for(i = 0; i < 16; i++) {
    
    c8_ram.memory[mem_ptr++] = reg_ptr++;
    c8_ram.memory[mem_ptr++] = 0xFF;
  }
  
  /* jump to 524 */
  c8_ram.memory[500] = 0x22;
  c8_ram.memory[501] = 0x0C;

  /* kill the program */
  c8_ram.memory[mem_ptr++] = 0xFF;
  c8_ram.memory[mem_ptr++] = 0xFF;


    
  registers.PC = 502;
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
int c8_decode_instructions(unsigned int instr) {
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
    printf("Jump to 0x%3X\n", instr & low_12bits);
    c8_jump(instr & low_12bits);
    break;

  case 0x2000: /* 2nnn - Call addr */
    printf("Call to 0x%3X\n", instr & low_12bits);
    c8_call(instr & low_12bits);
    break;

  case 0x6000: /* 6xkk - LD Vx, byte */
    printf("&&& [0x%04X] &&&\n", instr);
    printf("*** [0x%04X] ***\n", (instr & 0x0F00)>>8);
    
    c8_ld( ((instr & 0x0F00)>>8), instr & low_4bits);
    break;
    
  case 0xF000:
    printf("Killed\n");
    return -1;
    
  default:
    printf("Instruction: 0x%04X NYI.\n", op_flag);
    break;
  }
  return op_flag;
}

int c8_ld(int x, unsigned char b) {
  printf("Loading V%d with %d\n", x, b);
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
  /* check if stack is going to overflow */
  if(registers.SP == 15) {
    printf("Stack overflow. Exiting.\n");
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
  printf("RET\n");
  registers.PC = stack[registers.SP];
  if(registers.SP > 0) {
    registers.SP--;
  }else{
    printf("Attempted to pop something off the stack when nothing was there. c8_ret()\n");
  }
  return 1;
}

void c8_cls(void) {
  printf("Clearing the screen....\n");
}

/*******************************************************
 * show_registers()
 * Displays all the register values.
 *******************************************************/
void show_registers(void) {

  printf("-------------------------------------------------------\n");
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

