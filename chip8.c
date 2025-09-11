
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
uint8_t V[16];

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
    TODO("sys, cls, ret")
    break;

  case 0x1:
    pc = opcode & 0x0FFF;
    break;

  case 0x2:
    addr = opcode & 0x0FFF;
    TODO("call addr");
    break;

  case 0x3:
    x = opcode & 0x0F00 >> 8;
    TODO("SET Vx BYTE");
    break;

  case 0x4:
    x = opcode & 0x0F00 >> 8;
    TODO("SNE Vx, byte");
    break;

  case 0x5:
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    TODO("Se Vx, Vy");
    break;

  case 0x6:
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    TODO("LD Vx, by");
    break;

  case 0x7:
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    TODO("ADD Vx, byte");
    break;

  case 0x8:
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    switch (opcode & 0x000F) {
      TODO("operations between registers")    
    }
    break;

  case 0x9:
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    TODO("SNE Vx, Vy");
    break;

  case 0xA:
    addr = opcode & 0x0FFF;
    TODO("LD I, addr");
    break;

  case 0xB:
    addr = opcode & 0x0FFF;
    TODO("JP V0, addr");
    break;

  case 0xC:
    x = opcode & 0x0F00 >> 8;
    k = opcode & 0x00FF;
    TODO("RND Vx, byte");
    break;

  case 0xD:
    x = opcode & 0x0F00 >> 8;
    y = opcode & 0x00F0 >> 4;
    n = opcode & 0x000F;
    TODO("DRW Vx, Vy, nibble");
    break;

  case 0xE:
    x = opcode & 0x0F00 >> 8;
    switch (opcode & 0x000F) {
      TODO("SKP, SKPN");
    }
    break;

  case 0xF:
    x = opcode & 0x0F00 >> 8;
    switch (opcode & 0x00FF) {
      TODO("input, time and special registers");
    }
    break;
  }
}


