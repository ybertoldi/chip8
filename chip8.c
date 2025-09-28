
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>

// uncomment to enable instruction logging
// #define DO_LOGGING 

#ifdef DO_LOGGING
  FILE *logfile;
  #define LOGFILE_NAME "log.log"
  #define LOG_INIT logfile = fopen(LOGFILE_NAME, "w")
  #define LOG(text, ...) fprintf(logfile, text, __VA_ARGS__)
#else 
  #define LOGFILE_NAME
  #define LOG_INIT
  #define LOG(text, ...)
#endif 

// SDL
#define PIXEL_SIZE 15
#define WINDOW_HEIGHT (PIXEL_SIZE * 32)
#define WINDOW_WIDTH  (PIXEL_SIZE * 64)
#define FPS (1000 / 60)

typedef struct {
  SDL_Window *window;
  SDL_Surface *surface;
} Display;

SDL_Event evt;

void squit(void) {
  SDL_Quit();
  printf("SDL exited\n");
}

Display sdl_initialize(uint32_t flags) {
  atexit(squit);
  SDL_Window *window = NULL;
  SDL_Surface *surface = NULL;

  printf("initializing SDL\n");
  if (SDL_Init(flags) < 0) {
    fprintf(stderr, "couldnt initialize sdl: %s\n", SDL_GetError());
    exit(1);
  }
  if ((window = SDL_CreateWindow("qualquer_coisa", 0, 0,WINDOW_WIDTH ,WINDOW_HEIGHT , 0)) ==
      NULL) {
    fprintf(stderr, "could not create window\n");
    exit(1);
  }
  if ((surface = SDL_GetWindowSurface(window)) == NULL) {
    fprintf(stderr, "could not create window\n");
    exit(1);
  }
  printf("SDL initialized\n");
  return (Display){.window = window, .surface = surface};
}


// CHIP8
#define MEMSIZE 4096
#define PROGRAMSTART 0x200
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 64
#define DISPLAY(x, y) display_pixels[(x) + (y) * DISPLAY_WIDTH]

// memory and display
uint8_t mem[MEMSIZE];
uint8_t display_pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
uint16_t stack[STACK_SIZE];
uint8_t keys[16];
bool jump;

// special registers
uint16_t pc; // starts at PROGRAMSTART
uint16_t sp; // stack pointer
uint16_t I;  // memory adresses

uint8_t dt; // time
uint8_t st; // sound

// general purpose registers
uint8_t V[16]; // 16 registradores // V[0xF] é especial
uint8_t curkey;

void chip8_init(void) {
  static uint8_t fontset[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  for (int i = 0; i < 80; i++)
    mem[i] = fontset[i];

  pc = PROGRAMSTART;
  I = 0;
  sp = 0;

  for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    display_pixels[i] = 0;
  for (int i = PROGRAMSTART; i < MEMSIZE; i++)
    mem[i] = 0;
}

bool chip8_load(char *filepath) {
  FILE *romfile;
  int romsize;

  chip8_init();
  if ((romfile = fopen(filepath, "rb")) == NULL) {
    perror("failed to open romfile\n");
    return false;
  }

  fseek(romfile, 0, SEEK_END);
  romsize = ftell(romfile);
  rewind(romfile);

  if (romsize > MEMSIZE - PROGRAMSTART) {
    printf("romsize too big\n");
    return false;
  }
  if (fread(mem + PROGRAMSTART, sizeof(uint8_t), romsize, romfile) < romsize) {
    perror("failed to read rom\n");
    return false;
  }

  fclose(romfile);
  return true;
}

int read_event(){
  bool pressed;
  int key;
  int last_read = -1;
    while (SDL_PollEvent(&evt)) {
      switch (evt.type) {
        case SDL_QUIT: 
          exit(0);
          break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
          pressed = evt.type == SDL_KEYDOWN;
          key = evt.key.keysym.sym;
          switch (key) {
            case SDLK_0: keys[0]  = pressed; if (pressed) last_read = 0;  break;
            case SDLK_1: keys[1]  = pressed; if (pressed) last_read = 1;  break;
            case SDLK_2: keys[2]  = pressed; if (pressed) last_read = 2;  break;
            case SDLK_3: keys[3]  = pressed; if (pressed) last_read = 3;  break;
            case SDLK_4: keys[4]  = pressed; if (pressed) last_read = 4;  break;
            case SDLK_5: keys[5]  = pressed; if (pressed) last_read = 5;  break;
            case SDLK_6: keys[6]  = pressed; if (pressed) last_read = 6;  break;
            case SDLK_7: keys[7]  = pressed; if (pressed) last_read = 7;  break;
            case SDLK_8: keys[8]  = pressed; if (pressed) last_read = 8;  break;
            case SDLK_9: keys[9]  = pressed; if (pressed) last_read = 9;  break;
            case SDLK_a: keys[10] = pressed; if (pressed) last_read = 10; break;
            case SDLK_b: keys[11] = pressed; if (pressed) last_read = 11; break;
            case SDLK_c: keys[12] = pressed; if (pressed) last_read = 12; break;
            case SDLK_d: keys[13] = pressed; if (pressed) last_read = 13; break;
            case SDLK_e: keys[14] = pressed; if (pressed) last_read = 14; break;
            case SDLK_f: keys[15] = pressed; if (pressed) last_read = 15; break;
          }
          break;

        default:
          break;
      }
    }
    return last_read;
}

void read_opcode(uint16_t opcode) {
  uint16_t addr, k;
  uint8_t x, y, n,xpos, ypos, sprite, bit,  instr = (opcode & 0xF000) >> 12;
  int c;
  uint16_t sum;

  switch (instr) {
  case 0x0:
    switch (opcode & 0x0FFF) {
      case 0x00E0:
        LOG("CLS%s\n", "");
        for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) display_pixels[i] = 0;
        break;

      case 0x00EE:
        LOG("RET (stack[%d]=%d)\n", sp, stack[sp]);
        if (sp == 0){
          pc = 0;
        } else {
          pc = stack[--sp];
        }
        break;

      default:
        LOG("SYS addr (=%d)\n", opcode & 0x0FFF);
        pc = opcode & 0x0FFF;
        break;
    }
    break;

  case 0x1: 
    LOG(" JP addr = %d\n",opcode & 0x0FFF );
    pc = opcode & 0x0FFF;
    break;

  case 0x2:
    LOG(" CALL addr = %d; stack[%d] = %d\n", opcode & 0x0FFF, sp, pc);
    addr = opcode & 0x0FFF;
    stack[sp++] = pc;
    pc = addr;
    break;

  case 0x3:
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    LOG(" SE V%d, %d\n",x,k);
    if (V[x] == k) pc += 2;
    break;

  case 0x4:
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    LOG(" SNE V%d, %d\n",x,k);
    if (V[x] != k) pc += 2;
    break;

  case 0x5:
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    LOG(" SE V%d, V%d\n",x,y);
    if (V[x] == V[y]) pc += 2;
    break;

  case 0x6:
    x = (opcode & 0x0F00) >> 8;
    LOG(" LD V%d, %d\n", x, opcode & 0x00FF);
    V[x] = opcode & 0x00FF;
    break;

  case 0x7: 
    x = (opcode & 0x0F00) >> 8;
    LOG(" ADD V%d, %d\n", x, opcode & 0x00FF);
    V[x] += opcode & 0x00FF;
    break;

  case 0x8:
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    switch (opcode & 0x000F) {
    case 0x0: 
    LOG(" LD V%d, V%d\n", x, y);
      V[x] = V[y];
      break;
    case 0x1: 
    LOG(" OR V%d, V%d\n", x, y);
      V[x] |= V[y];
      break;
    case 0x2: 
    LOG(" AND V%d, V%d\n", x, y);
      V[x] &= V[y];
      break;
    case 0x3: 
    LOG(" XOR V%d, V%d\n", x, y);
      V[x] ^= V[y];
      break;
    case 0x4: 
    LOG(" ADD V%d, V%d\n", x, y);
      sum = (uint16_t) V[x] + (uint16_t) V[y];
      V[0xF] = sum > 0xFF;
      V[x] = sum & 0xFF;
      break;
    case 0x5: 
    LOG(" SUB V%d, V%d\n", x, y);
      V[0xF] = V[x] >= V[y];
      V[x] -= V[y];
      break;
    case 0x6: 
    LOG(" SHR V%d, {, V%d}\n", x, y);
      V[0xF] = V[x] & 0x1;
      V[x] >>= 1;
      break;
    case 0x7: 
    LOG(" SUBN V%d, V%d\n", x, y);
      V[0xF] = V[y] >= V[x];
      V[x] = V[y] - V[x];
      break;
    case 0xE:                       
    LOG(" SHL V%d\n", x);
      V[0xF] = (V[x] & 0x80) >> 7;
      V[x] <<=  1;
      break;
    }
    break;

  case 0x9: 
    LOG(" SNE V%d, V%d\n", x, y);
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    if (V[x] != V[y])
      pc += 2;
    break;

  case 0xA: 
    LOG(" LD I, %d\n", opcode & 0x0FFF);
    addr = opcode & 0x0FFF;
    I = addr;
    break;

  case 0xB: 
    LOG(" JP V0, %d\n", opcode & 0x0FFF);
    addr = opcode & 0x0FFF;
    pc = addr + V[0];
    jump = 1;
    break;

  case 0xC: 
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    LOG(" RND V%d, %d\n", x, k);
    V[x] = (rand() % 256) & k;
    break;

  case 0xD:
    x = V[(opcode & 0x0F00) >> 8];
    y = V[(opcode & 0x00F0) >> 4];
    n = opcode & 0x000F;
    LOG(" DRAW %d, %d, %d\n", x, y, n);

    V[0xF] = 0;
    for (int i = 0; i < n; i++) {
      sprite = mem[I + i]; 
      for (int j = 0; j < 8; j++) {
        xpos = (x + j) % DISPLAY_WIDTH;
        ypos = (y + i) % DISPLAY_HEIGHT;

        bit = (sprite >> (7 - j)) & 0x1;
        if (bit && DISPLAY(xpos, ypos))
          V[0xF] = 1;

        DISPLAY(xpos, ypos) = bit ^ DISPLAY(xpos, ypos);
      }
    }
    break;

  case 0xE:
    x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
    case 0x9E:
      LOG("SKP V%d (=%d)\n", x, V[x]);
      if (keys[V[x]])
        pc += 2;
      break;
    case 0xA1:
      LOG("SKNP V%d (=%d)\n", x, V[x]);
      if (!keys[V[x]])
        pc += 2;
      break;
    }
    break;

  case 0xF:
    x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
    case 0x07:
      LOG("LD V%d, DT (=%d)\n", x, dt);
      V[x] = dt;
      break;
    case 0x0A:
      LOG("WAITK V%d, K\n", x);
      while ((c = read_event()) == -1 )
        ;
      V[x] = c;
      LOG(" (=%d)\n", c);
      break;
    case 0x15:
      dt = V[x];
      LOG("LD DT, V%d (=%d)\n", x, V[x]);
      break;
    case 0x18:
      st = V[x];
      LOG("LD ST, V%d (=%d)\n", x, V[x]);
      break;
    case 0x1E:
      I += V[x];
      LOG("ADD I, V%d (=%d)\n", x, V[x]);
      break;
    case 0x29:
      I = V[x] * 5;
      LOG("ADD F, V%d (=%d)\n", x, V[x]);
      break;
    case 0x33:
      LOG("LD B, V%d (=%d)\n", x, V[x]);
      mem[I] = V[x] / 100;
      mem[I + 1] = (V[x] / 10) % 10;
      mem[I + 2] = V[x] % 10;
      break;
    case 0x55:
      LOG("LD [I], V%d\n", x);
      for (uint8_t i = 0; i <= x; i++)
        mem[I++] = V[i];
      break;
    case 0x65:
      LOG("LD V%d, [I]\n", x);
      for (uint8_t i = 0; i <= x; i++)
        V[i] = mem[I++];
      break;
    }
    break;
  }

#ifdef DO_LOGGING
  fflush(logfile);
#endif
}


int main(int argc, char *argv[]) {
  if (argc != 2){
    printf("usage: %s <rom-file>\n", argv[0]);
    exit(0);
  }
  
  LOG_INIT;
  int c;
  uint16_t opcode;
  V[0] = 0;
  V[1] = 0;

  Display display = sdl_initialize(SDL_INIT_VIDEO);
  chip8_init();
  chip8_load(argv[1]);

  while (1) {
    read_event();

    opcode = mem[pc] << 8 | mem[pc + 1];
    pc += 2;
    read_opcode(opcode);

    if (dt > 0) dt--;
    if (st > 0) st--;
    
    SDL_Rect rect = {.h = PIXEL_SIZE, .w = PIXEL_SIZE};
    for (int i = 0; i < 32; i++){
      for (int j = 0; j < 64; j++){
        rect.x = (j * (PIXEL_SIZE));
        rect.y = (i * (PIXEL_SIZE));

        SDL_FillRect(
            display.surface,
            &rect, 
            DISPLAY(j, i) ? 0xFFFFFF : 0x000000
        );
      }
    }
    
    SDL_UpdateWindowSurface(display.window);
    SDL_Delay(FPS / 20);
  }
}
