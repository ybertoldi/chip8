
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TODO(s) printf("TODO: %s\n", s); exit(0);

#define MEMSIZE 4096
#define PROGRAMSTART 0x200
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 64
#define DISPLAY(x, y) display[(x) + (y) * DISPLAY_WIDTH]


// memory and display
uint8_t mem[MEMSIZE];
uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
uint8_t stack[STACK_SIZE];

// special registers
uint16_t pc; // starts at PROGRAMSTART
uint16_t sp; // stack pointer
uint16_t I;  // memory adresses

uint8_t dt; // time
uint8_t st; // sound

// general purpose registers
uint8_t V[16]; // 16 registradores // V[15] é especial
uint8_t curkey;


void chip8_init(void){
  static uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
  };
  for (int i = 0; i < 80; i++) 
    mem[i] = fontset[i];

  pc = PROGRAMSTART;
  I  = 0;
  sp = 0;

  // clear display and program area
  for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) 
    display[i] = 0;
  for (int i = PROGRAMSTART; i < MEMSIZE; i++)
    mem[i] = 0;
}

bool chip8_load(char *filepath){
  FILE *romfile;
  int romsize;

  chip8_init();
  if ((romfile = fopen(filepath, "rb")) == NULL){
    perror("failed to open romfile\n");
    return false;
  }

  fseek(romfile, 0, SEEK_END);
  romsize = ftell(romfile);
  rewind(romfile);

  if (romsize > MEMSIZE - PROGRAMSTART){
    printf("romsize too big\n");
    return false;
  }
  if (fread(mem + PROGRAMSTART, sizeof(uint8_t), romsize, romfile) < romsize){
    perror("failed to read rom\n");
    return false;
  }

  fclose(romfile);
  return true;
}

void read_opcode(uint16_t opcode){
  uint16_t addr, k;
  uint8_t x, y, n, instr = (opcode & 0xF000) >> 12;

  switch (instr) {
  case 0x0:
    if (opcode & 0xF00){
      pc = opcode & 0xFFF;
    } else if (opcode & 0xF) {
      pc = stack[sp--];          
    } else {
      for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) 
        display[i] = 0;
    }

    break;

  case 0x1: // JP addr
    pc = opcode & 0x0FFF;
    break;

  case 0x2: // CALL addr
    addr = opcode & 0x0FFF;
    stack[sp++] = pc;
    pc = addr;
    break;

  case 0x3: // SE Vx, byte
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    if (V[x] == k)
      pc += 2;
    break;

  case 0x4: // SNE Vx, byte
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    if (V[x] != k)
      pc += 2;
    break;

  case 0x5: // SE Vx, Vy
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    if (V[x] == V[y])
      pc += 2;
    break;

  case 0x6: // LD Vx, byte
    x = opcode & 0x0F00 >> 8;
    V[x] = opcode & 0x00FF;
    break;

  case 0x7: // ADD Vx, byte
    x = opcode & 0x0F00 >> 8;
    V[x] += opcode & 0x00FF;
    break;

  case 0x8:
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    switch (opcode & 0x000F) {
      case 0x1: // LD Vx, Vy
          V[x] = V[y];
          break;
      case 0x2: // OR Vx, Vy
          V[x] |= V[y];
          break;
      case 0x3: // XOR Vx, Vy
          V[x] ^= V[y];
          break;
      case 0x4: // ADD Vx, Vy
          V[x] += V[y];
          V[15] = (V[x] < V[y]);
          break;
      case 0x5: // SUB Vx, Vy
          V[15] = V[x] >= V[y];
          V[x] -= V[y];
          break;
      case 0x6: // SHR Vx, {, Vy}
          V[15] = V[x] & 0x1;
          V[x] = V[x] << 1;
          break;
      case 0x7: // SUBN Vx, Vy
          V[15] = V[y] >= V[x];
          V[x] = V[y] - V[x];
          break;
      case 0xE: // SHL Vx
          V[15] = (V[x] & 0x8) ? 1 : 0; // 0x8 = b'1000'.  Vf guarda o carry
          V[x] = V[x] << 1;
          break;
    }
    break;

  case 0x9: // SNE Vx, Vy
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    if (V[x] != V[y])
      pc += 2;
    break;

  case 0xA: //LD I, addr
    addr = opcode & 0x0FFF;
    I = addr;
    break;

  case 0xB: // JP V0, addr
    addr = opcode & 0x0FFF;
    pc = addr + V[0];
    break;

  case 0xC: // RND Vx, byte
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    V[x] = (rand() % 256) & k;
    break;

  case 0xD:
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    n = opcode & 0x000F;

    V[15] = 0;
    uint8_t sprite, bit;
    for (int i = 0; i < n; i++) {
      sprite = mem[I + i];
      for (int j = 0; j < 8; j++) {
        bit = (sprite >> (7 - j)) & 0x1;
        if (bit && DISPLAY(x + j, y + i)) V[15] = 1;
        DISPLAY(x + j, y + i) = bit ^ DISPLAY(x + j, y + i);
      }
    }
    break;

  case 0xE:
    x = opcode & 0x0F00 >> 8;
    switch (opcode & 0x000F) {
    case 0xE:
      if (curkey == V[x])
        pc += 2;
      break;
    case 0x1:
      if (curkey != V[x])
        pc += 2;
      break;
    }
    break;

  case 0xF:
    x = opcode & 0x0F00 >> 8;
    switch (opcode & 0x00FF) {
      case 0x07:
        V[x] = dt;
        break;
      case 0x0A:
        V[x] = waitkey(); // TODO
        break;
      case 0x15:
        dt = V[x];
        break;
      case 0x18:
        st = V[x];
        break;
      case 0x1E:
        I += V[x];
        break;
      case 0x29:
        I = V[x] / 5; // TODO: ver se isso esta certo
        break;
      case 0x33:
        mem[I + 2] = V[x] % 10;
        mem[I + 1] = V[x] % 100 - mem[I + 2];
        mem[I] = V[x] - mem[I + 1] - mem[I + 2];
        break;
      case 0x55:
        for (uint8_t i = 0; i <= x; i++)
          mem[I++] = V[i];
        break;
      case 0x65:
        for (uint8_t i = 0; i <= x; i++)
          V[i] = mem[I++];
        break;
      }
    break;
  }
}


