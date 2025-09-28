#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void print_opcode(uint16_t opcode) {
  uint8_t instr = (opcode & 0xF000) >> 12;
  switch (instr) {
    case 0x0:
      switch (opcode & 0x0FFF) {
        case 0x00E0: printf("CLS\n"); break;
        case 0x00EE: printf("RET\n"); break;
        default: printf("SYS addr %d\n", opcode & 0x0FFF); 
      }
      break;

    case 0x1: printf("JP addr = %d\n",opcode & 0x0FFF ); break;
    case 0x2: printf("CALL addr %d\n", opcode & 0x0FFF); break;
    case 0x3: printf("SE V%d, %d\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF);  break;
    case 0x4: printf("SNE V%d, %d\n",(opcode & 0x0F00) >> 8,opcode & 0x00FF); break;
    case 0x5: printf("SE V%d, V%d\n",(opcode & 0x0F00) >> 8,(opcode & 0x00F0) >> 4);  break;
    case 0x6: printf("LD V%d, %d\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
    case 0x7: printf("ADD V%d, %d\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;

    case 0x8: {
        int x = (opcode & 0x0F00) >> 8;
        int y = (opcode & 0x00F0) >> 4;
        switch (opcode & 0x000F) {
          case 0x0: printf("LD V%d, V%d\n", x, y); break;
          case 0x1: printf("OR V%d, V%d\n", x, y); break;
          case 0x2: printf("AND V%d, V%d\n", x, y); break;
          case 0x3: printf("XOR V%d, V%d\n", x, y); break;
          case 0x4: printf("ADD V%d, V%d\n", x, y); break;
          case 0x5: printf("SUB V%d, V%d\n", x, y); break;
          case 0x6: printf("SHR V%d, {, V%d}\n", x, y); break;
          case 0x7: printf("SUBN V%d, V%d\n", x, y); break;
          case 0xE: printf("SHL V%d\n", x); break;
        }
        break;
    }

    case 0x9: printf("SNE V%d, V%d\n", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4); break;
    case 0xA: printf("LD I, %d\n", opcode & 0x0FFF); break;
    case 0xB: printf("JP V0, %d\n", opcode & 0x0FFF); break;
    case 0xC: printf("RND V%d, %d\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF); break;
    case 0xD: printf("DRAW %d, %d, %d\n", (opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4,  opcode & 0x000F); break;

    case 0xE:{
      int x = (opcode & 0x0F00) >> 8;
      switch (opcode & 0x00FF) {
        case 0x9E: printf("SKP V%d\n", x); break;
        case 0xA1: printf("SKNP V%d\n", x); break;
      }
      break;
    }

    case 0xF: {
      int x = (opcode & 0x0F00) >> 8;
      switch (opcode & 0x00FF) {
        case 0x07: printf("LD V%d, DT\n", x); break;
        case 0x0A: printf("WAITK V%d, K\n", x); break;
        case 0x15: printf("LD DT, V%d\n", x); break;
        case 0x18: printf("LD ST, V%d", x); break;
        case 0x1E: printf("ADD I, V%d\n", x); break;
        case 0x29: printf("ADD F, V%d\n", x); break;
        case 0x33: printf("LD B, V%d\n", x); break;
        case 0x55: printf("LD [I], V%d\n", x); break;
        case 0x65: printf("LD V%d, [I]\n", x); break;
      }
      break;
    }
  }
}

#define MEMSIZE 4096
#define PROGRAMSTART 0x200
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 64
int rom_load(char *filepath, uint8_t *buf) {
  FILE *romfile;
  int romsize;

  if ((romfile = fopen(filepath, "rb")) == NULL) {
    perror("failed to open romfile\n");
    return -1;
  }

  fseek(romfile, 0, SEEK_END);
  romsize = ftell(romfile);
  rewind(romfile);

  if (romsize > MEMSIZE - PROGRAMSTART) {
    printf("romsize too big\n");
    return -1;
  }

  if (fread(buf, sizeof(uint8_t), romsize, romfile) < romsize) {
    perror("failed to read rom\n");
    return -1;
  }

  fclose(romfile);
  return romsize;
}

int main(int argc, char *argv[]){
  int romsize;
  uint8_t buf[MEMSIZE];

  if (argc != 2){
    printf("usage: %s <rom>\n", argv[0]);
    exit(1);
  }

  if ((romsize = rom_load(argv[1], buf)) < 0) return 1;
  
  for (int i = 0; i < romsize; i += 2)
    print_opcode(buf[i] << 8 | buf[i+1]);

  return 0;
}
