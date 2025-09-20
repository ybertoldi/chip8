teste8: chip8.c
	gcc -o teste chip8.c -g -lSDL2 -Wall

sdl_ex:  teste_sdl.c
	gcc -o sdl_ex teste_sdl.c -g -lSDL2 
