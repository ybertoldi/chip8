
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TODO(s) printf("TODO: %s\n", s); exit(0);

#define MEMSIZE 4096
#define PROGRAMSTART 0x200
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32



// memory and display
uint8_t mem[MEMSIZE];
uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];

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
  for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) 
    display[i] = 0;
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

void read_instr(uint16_t instr){
  switch (instr) {
    case 0x00E0:
      TODO("cls");
      return;
    case 0x00EE:
      TODO("RET");
      return;
  }

  switch (instr & 0x1) {
    case 1:
      TODO("JP to addres");
    case 2:
      TODO("CALL addres");
    case 3:
      TODO("SET Vx BYTE");
    case 4:
      TODO(s)
  
  }

}


