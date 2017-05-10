#include <stdio.h>
#include <SDL.h>

/******************************************************
 * Chip 8 Status
 ******************************************************/
typedef enum c8_status {
  RUNNING,
  PAUSED,
  DEAD,
  WAITING_FOR_KEY
} c8_status;

#define FONT_ADDRESS 0x00
#define FONT_WIDTH 1 /* number of bytes wide */
#define FONT_HEIGHT 6 /* number of bytes high */

/*******************************************************
 * Chip 8 Keyboard
 *******************************************************/
#define C8_NUM_KEYS 16
extern int keypress_register;
/*******************************************************
 * Chip 8 Registers
 *******************************************************/
struct _registers {
  unsigned char V[16];
  unsigned int I;
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
#define START_LOAD_ADDRESS 500

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
int c8_ld_vx_vy(int x, int y);
int c8_sub_vx_vy(int x, int y);
int c8_add_vx_vy(int x, int y);
int c8_xor_vx_vy(int x, int y);
int c8_and_vx_vy(int x, int y);
int c8_or_vx_vy(int x, int y);
int c8_shr_vx(int x);
int c8_subn_vx_vy(int x, int y);
int c8_shl_vx(int x);
int c8_sne_vx_vy(int x, int y);
int c8_ld_i(unsigned int i);
int c8_jp_v0(int addr);
int c8_rnd(int x, int b);
int c8_drw(int x, int y, int num_bytes);
int c8_skp_vx(int key);
int c8_sknp_vx(int key);
int c8_ld_vx_dt(int x);
int c8_wait_for_keypress(int x);
int c8_got_keypress(int key);
int c8_ld_dt_vx(int x);
int c8_ld_st_vx(int x);
int c8_add_i_vx(int x);
int c8_ld_f_vx(int x);
int c8_ld_b_vx(int x);
int c8_ld_i_vx(int x);
int c8_ld_vx_i(int x);

void show_registers(void);
void c8_test(void);
void dump_stack(void);
void dump_mem(int start, int num_bytes);

/* display functions */
void dump_display(void);
int c8_draw_sprite(int mem_loc, int num_bytes, int x, int y);
void c8_clear_screen(void);

/* loader */
int c8_load_from_file(char *filename);
int c8_load_fonts(void);

/* keyboard functions */
int c8_init_keyboard(void);
int c8_keydown(int keynum);
int c8_keyup(int keynum);
int c8_get_key_state(int keynum);
void dump_key_state(void);

/* SDL specific functions */
void draw_screen(SDL_Surface *surface, int pixel_width, int pixel_height);
void draw_pixel(SDL_Surface *surface, int x , int y, int pixel_width, int pixel_height, unsigned int color);
int get_key(SDL_Keycode keycode);

/* log functions */
FILE *init_c8_log(char *filename);
void c8_log_write(char *str);
void c8_log_close(void);
