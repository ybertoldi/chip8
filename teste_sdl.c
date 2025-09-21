#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_keyboard.h>

#define PIXEL_SIZE 15
#define WINDOW_HEIGHT (PIXEL_SIZE * 32)
#define WINDOW_WIDTH  (PIXEL_SIZE * 64)
#define FPS (1000 / 60)


typedef struct {
  SDL_Window *window;
  SDL_Surface *surface;
} Display;

Display sdl_initialize(uint32_t flags);
void log_surface(SDL_Surface *);

SDL_Event evt;
bool running = true;
void read_event(){
    while (SDL_PollEvent(&evt)) {
      switch (evt.type) {
        case SDL_QUIT: 
          exit(0);
          break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
          printf("%s\n", SDL_GetKeyName(evt.key.keysym.sym));
          break;

        default:
          break;
      }
    }
}

int main() {
  Display display = sdl_initialize(SDL_INIT_VIDEO);

  while (running) {
    SDL_FillRect(display.surface,NULL, 0xFF0000);
    read_event();

    SDL_Rect rect = {.h = PIXEL_SIZE, .w = PIXEL_SIZE};
    for (int i = 0; i < 32; i++){
      for (int j = 0; j < 64; j++){
        rect.x = (j * (PIXEL_SIZE));
        rect.y = (i * (PIXEL_SIZE));

        SDL_FillRect(display.surface, &rect, 0xFFFFFF);
      }
    }

    SDL_UpdateWindowSurface(display.window);
    SDL_Delay(FPS);
  }

  return 0;
}

//////////////// foward declared functions //////////////////
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


void log_surface(SDL_Surface *surface){
  uint32_t* pixels = (uint32_t*) surface->pixels;

  FILE *log = fopen("pixels.txt", "w");
  fprintf(log, "width: %d, height: %d\n", surface->w, surface->h);

  for (int i = 0; i < surface->h; i++){
    fprintf(log, "\n");
    for (int j = 0; j < surface->w; j++)
      fprintf(log, "%08x ", pixels[j + (i * surface->w)]);
  }
  fclose(log);
}
