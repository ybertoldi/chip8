
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>


#include <ncurses.h>
#include <curses.h>

#define TODO(s)                                                                \
  printf("TODO: %s\n", s);                                                     \
  exit(0);

#define MEMSIZE 4096
#define PROGRAMSTART 0x200
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define STACK_SIZE 64
#define DISPLAY(x, y) display[(x) + (y) * DISPLAY_WIDTH]

// memory and display
uint8_t mem[MEMSIZE];
uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
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

FILE *logfile;

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

  // clear display and program area
  for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    display[i] = 0;
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

int get_key(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return -1;
  }
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
        //fprintf(logfile,"CLS\n");
        for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) display[i] = 0;
        clear();
        break;

      case 0x00EE:
        //fprintf(logfile,"RET (stack[%d]=%d)\n", sp, stack[sp]);
        if (sp == 0){
          pc = 0;
        } else {
          pc = stack[--sp];
        }
        break;

      default:
        //fprintf(logfile,"SYS addr (=%d)\n", opcode & 0x0FFF);
        pc = opcode & 0x0FFF;
        break;
    }
    break;

  case 0x1: // JP addr
    //fprintf(logfile," JP addr = %d\n",opcode & 0x0FFF );
    pc = opcode & 0x0FFF;
    break;

  case 0x2: // CALL addr
    //fprintf(logfile," CALL addr = %d; stack[%d] = %d\n", opcode & 0x0FFF, sp, pc);
    addr = opcode & 0x0FFF;
    stack[sp++] = pc;
    pc = addr;
    break;

  case 0x3: // SE Vx, byte
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    //fprintf(logfile," SE V%d, %d\n",x,k);
    if (V[x] == k) pc += 2;
    break;

  case 0x4: // SNE Vx, byte
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    //fprintf(logfile," SNE V%d, %d\n",x,k);
    if (V[x] != k) pc += 2;
    break;

  case 0x5: // SE Vx, Vy
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    //fprintf(logfile," SE V%d, V%d\n",x,y);
    if (V[x] == V[y]) pc += 2;
    break;

  case 0x6: // LD Vx, byte
    x = (opcode & 0x0F00) >> 8;
    //fprintf(logfile," LD V%d, %d\n", x, opcode & 0x00FF);
    V[x] = opcode & 0x00FF;
    break;

  case 0x7: // ADD Vx, byte
    x = (opcode & 0x0F00) >> 8;
    //fprintf(logfile," ADD V%d, %d\n", x, opcode & 0x00FF);
    V[x] += opcode & 0x00FF;
    break;

  case 0x8:
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    switch (opcode & 0x000F) {
    case 0x0: // LD Vx, Vy
    //fprintf(logfile," LD V%d, V%d\n", x, y);
      V[x] = V[y];
      break;
    case 0x1: // OR Vx, Vy
    //fprintf(logfile," OR V%d, V%d\n", x, y);
      V[x] |= V[y];
      break;
    case 0x2: // AND Vx, Vy
    //fprintf(logfile," AND V%d, V%d\n", x, y);
      V[x] &= V[y];
      break;
    case 0x3: // XOR Vx, Vy
    //fprintf(logfile," XOR V%d, V%d\n", x, y);
      V[x] ^= V[y];
      break;
    case 0x4: // ADD Vx, Vy
    //fprintf(logfile," ADD V%d, V%d\n", x, y);
      sum = (uint16_t) V[x] + (uint16_t) V[y];
      V[0xF] = sum > 0xFF;
      V[x] = sum & 0xFF;
      break;
    case 0x5: // SUB Vx, Vy
    //fprintf(logfile," SUB V%d, V%d\n", x, y);
      V[0xF] = V[x] >= V[y];
      V[x] -= V[y];
      break;
    case 0x6: // SHR Vx, {, Vy}
    //fprintf(logfile," SHR V%d, {, V%d}\n", x, y);
      V[0xF] = V[x] & 0x1;
      V[x] >>= 1;
      break;
    case 0x7: // SUBN Vx, Vy
    //fprintf(logfile," SUBN V%d, V%d\n", x, y);
      V[0xF] = V[y] >= V[x];
      V[x] = V[y] - V[x];
      break;
    case 0xE:                       // SHL Vx
    //fprintf(logfile," SHL V%d\n", x);
      V[0xF] = (V[x] & 0x80) >> 7;
      V[x] <<=  1;
      break;
    }
    break;

  case 0x9: // SNE Vx, Vy
    //fprintf(logfile," SNE V%d, V%d\n", x, y);
    x = (opcode & 0x0F00) >> 8;
    y = (opcode & 0x00F0) >> 4;
    if (V[x] != V[y])
      pc += 2;
    break;

  case 0xA: // LD I, addr
    //fprintf(logfile," LD I, %d\n", opcode & 0x0FFF);
    addr = opcode & 0x0FFF;
    I = addr;
    break;

  case 0xB: // JP V0, addr
    //fprintf(logfile," JP V0, %d\n", opcode & 0x0FFF);
    addr = opcode & 0x0FFF;
    pc = addr + V[0];
    jump = 1;
    break;

  case 0xC: // RND Vx, byte
    x = (opcode & 0x0F00) >> 8;
    k = opcode & 0x00FF;
    //fprintf(logfile," RND V%d, %d\n", x, k);
    V[x] = (rand() % 256) & k;
    break;

  case 0xD:
    x = V[(opcode & 0x0F00) >> 8];
    y = V[(opcode & 0x00F0) >> 4];
    n = opcode & 0x000F;
    //fprintf(logfile," DRAW %d, %d, %d\n", x, y, n);

    V[0xF] = 0;
    for (int i = 0; i < n; i++) {
      sprite = mem[I + i]; // line of 8 pixels
      for (int j = 0; j < 8; j++) {
        xpos = (x + j) % DISPLAY_WIDTH;
        ypos = (y + i) % DISPLAY_HEIGHT;

        bit = (sprite >> (7 - j)) & 0x1;
        if (bit && DISPLAY(xpos, ypos))
          V[0xF] = 1;

        DISPLAY(xpos, ypos) = bit ^ DISPLAY(xpos, ypos);
        move(ypos + 1, xpos + 1);
        printw("%s", DISPLAY(xpos, ypos) ? "█" : " ");
      }
    }
    break;

  case 0xE:
    x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
    case 0x9E:
      //fprintf(logfile,"SKP V%d (=%d)\n", x, V[x]);
      if (keys[V[x]])
        pc += 2;
      break;
    case 0xA1:
      //fprintf(logfile,"SKNP V%d (=%d)\n", x, V[x]);
      if (!keys[V[x]])
        pc += 2;
      break;
    }
    break;

  case 0xF:
    x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
    case 0x07:
      //fprintf(logfile,"LD V%d, DT (=%d)\n", x, dt);
      V[x] = dt;
      break;
    case 0x0A:
      //fprintf(logfile,"WAITK V%d, K\n", x);
      while ((c = get_key(getch())) == -1)
          ;
      V[x] = c;
      //fprintf(logfile," (=%d)\n", c);
      break;
    case 0x15:
      dt = V[x];
      //fprintf(logfile,"LD DT, V%d (=%d)\n", x, V[x]);
      break;
    case 0x18:
      st = V[x];
      //fprintf(logfile,"LD ST, V%d (=%d)\n", x, V[x]);
      break;
    case 0x1E:
      I += V[x];
      //fprintf(logfile,"ADD I, V%d (=%d)\n", x, V[x]);
      break;
    case 0x29:
      I = V[x] * 5; // TODO: ver se isso esta certo
      //fprintf(logfile,"ADD F, V%d (=%d)\n", x, V[x]);
      break;
    case 0x33:
      //fprintf(logfile,"LD B, V%d (=%d)\n", x, V[x]);
      mem[I] = V[x] / 100;
      mem[I + 1] = (V[x] / 10) % 10;
      mem[I + 2] = V[x] % 10;
      break;
    case 0x55:
      //fprintf(logfile,"LD [I], V%d\n", x);
      for (uint8_t i = 0; i <= x; i++)
        mem[I++] = V[i];
      break;
    case 0x65:
      //fprintf(logfile,"LD V%d, [I]\n", x);
      for (uint8_t i = 0; i <= x; i++)
        V[i] = mem[I++];
      break;
    }
    break;
  }
  fflush(logfile);
}


int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  int iters = 0;
  int c;
  uint16_t opcode;
  V[0] = 0;
  V[1] = 0;
  logfile = fopen("log.log", "w");
  setvbuf(logfile, NULL, _IOLBF, 1000);

  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE); // Set stdscr to non-blocking

  chip8_init();
  chip8_load(argv[1]);

  while (1) {

    for (int i = 0; i < 16; i++) keys[i] = 0;
    while ((c = get_key(getch())) != -1) keys[c] = 1;
    opcode = mem[pc] << 8 | mem[pc + 1];
    pc += 2;
    read_opcode(opcode);
    refresh();

    if (dt > 0) dt--;
    if (st > 0) st--;
    usleep(20000);
  }
}

int main2(int argc, char *argv[]) {
  setlocale(LC_ALL, "");
  int iters = 0;
  int c;
  V[0] = 0;
  V[1] = 0;
  logfile = fopen("instructions", "w");
  setvbuf(logfile, NULL, _IOLBF, 1000);
  chip8_load(argv[1]);
  for (int i = PROGRAMSTART; i+1 < MEMSIZE; i+=2){
    read_opcode(mem[i] << 8 | mem[i+1]);
  }

  

  return 0;
}

// int main1(){
//   int iters = 0;
//   int c;
//   V[0] = 0;
//   V[1] = 0;
// 
//   initscr();
//   cbreak();
//   noecho();
//   chip8_init();
//   while (1) {
//     refresh();
//     for (int i = 0; i < 16; i++) keys[i] = 0; // limpa o teclado
//     while ((c = getch()) != ERR){
//       if (get_key(c) != -1)
//         keys[get_key(c)] = 1;
//     }
//       
//     for (int i = 0; i < DISPLAY_HEIGHT; i++) {
//       move(i + 1, 1);
//       for (int j = 0; j < DISPLAY_WIDTH; j++) {
//         printw("%c", (DISPLAY(j, i)) ? '#' : ' ');
//       }
//     }
// 
//     I = 5 * (c - 'a');
//     read_opcode(0xD015);
//     iters += 1;
//     if (V[0] + 15 > DISPLAY_WIDTH){
//       V[0] = 0; V[1] += 6;
//     } else {
//       V[0] += 9;
//     }
//   }
// 
//   return 0;
// }
