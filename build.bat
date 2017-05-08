set INCLUDE=-I/Code/SDL/include/SDL2 -Dmain=SDL_main
set LIB=-L/Code/SDL/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
set SRC=chip8.c chip8_display.c chip8_log.c chip8_sdl.c

gcc %INCLUDE% %SRC% %LIB% -o chip8_sdl.exe 
