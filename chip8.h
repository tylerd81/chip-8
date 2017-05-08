#include <stdio.h>
#include <SDL.h>

typedef enum c8_status {
  RUNNING,
  PAUSED,
  DEAD
} c8_status;

/*******************************************************
 * Chip 8 Registers
 *******************************************************/
struct _registers {
  unsigned char V[16];
  int I;
  int PC; /* program counter */

  /* stack pointer, only 8 bit but doesn't matter, will be index into stack (0-15) */
  int SP; 
    
  int DT; /* delay timer */
  int ST; /* sound timer */
};
extern struct _registers registers;
extern c8_status c8_state;

/*******************************************************
 * Chip 8 RAM
 *******************************************************/
#define C8_MAX_RAM 4096

struct _c8_ram {
  unsigned char memory[C8_MAX_RAM];
  int used_bytes;
  int total_mem;
} c8_ram;
extern struct _c8_ram c8_ram;

/* registers, stack, memory and display are actually defined
 * in chip8.c
 */

/*******************************************************
 * The Stack 
 *
 *******************************************************/
extern int stack[16];

/*******************************************************
 * Chip 8 Display
 * Each element of the array is either 1 or 0. The
 * display is monochrome so there are only two possible
 * values.
 *******************************************************/
#define C8_DISPLAY_HEIGHT 32
#define C8_DISPLAY_WIDTH 64

/* each char is representing 1 bit, it is either 1 or 0 */
extern unsigned char c8_display[C8_DISPLAY_HEIGHT][C8_DISPLAY_WIDTH];


#define LOG_STR_LEN 512
extern FILE *log_file;

/*******************************************************
 * Prototypes
 *******************************************************/
void c8_start(void);
void c8_quit(void);
void c8_set_state(c8_status state);
unsigned int c8_fetch_instruction();
int c8_decode_instruction(unsigned int instr);

/* Instructions */
int c8_call(int address);
int c8_jump(int addr);
int c8_ret(void);
void c8_cls(void);
int c8_ld(int x, unsigned char b);
int c8_se(int x, unsigned char k);
int c8_sne(int x, unsigned char k);
int c8_se_vx_vy(int x, int y);
int c8_add(int x, unsigned char k);


void show_registers(void);
void c8_test(void);
void dump_stack(void);

/* display functions */
void dump_display(void);
int c8_draw_sprite(int mem_loc, int num_bytes, int x, int y);
void c8_clear_screen(void);

/* SDL specific functions */
void draw_screen(SDL_Surface *surface, int pixel_width, int pixel_height);
void draw_pixel(SDL_Surface *surface, int x , int y, int pixel_width, int pixel_height, unsigned int color);

/* log functions */
FILE *init_c8_log(char *filename);
void c8_log_write(char *str);
void c8_log_close(void);
