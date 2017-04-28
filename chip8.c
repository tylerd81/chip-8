#include <stdio.h>

struct {
    char V0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF;
    int I;
    int PC; /* program counter */
    int SP; /* stack pointer, only 8 bit but doesn't matter, will be index into stack (0-15) */
    int stack[16];
    
    int DT; /* delay timer */
    int ST; /* sound timer */
    
}registers;

void c8_start(void);
int decode_instruction(int instr);
int c8_jump(int addr);
int c8_ret(void);
void show_registers(void);

int main(void) {
    printf("Welcome To Tyler's Chip-8 Interpreter!\n");
    c8_start();
    return 0;
}

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
    
    printf("Initialization complete.\n");    
    
}

int decode_instructions(int instr) {
    return 1;
}

int c8_jump(int addr) {
    return 1;
}

int c8_ret(void) {
    return 1;
}

void show_registers(void) {

}