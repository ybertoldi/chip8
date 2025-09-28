emu: chip8.c
	gcc -o emu chip8.c -g -lSDL2

chdis: chip8_disassemble.c
	gcc -o chdis chip8_disassemble.c
