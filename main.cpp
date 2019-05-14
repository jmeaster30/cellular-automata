#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <ctime>
#include <chrono>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

int main(int argc, char** argv)
{
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
  {
    printf("ERROR %s: SDL Initialization Failed!\n", SDL_GetError());
    return -1;
  }

  std::string win_title = "Cellular Automata";

  int win_width = 800;
  int win_height = 600;
  int cell_size = 20;

  int cell_w = win_width / cell_size;
  int cell_h = win_height / cell_size;

  //grid[x][y]
  //where 0 <= x <= cell_w
  //where 0 <= y <= cell_h
  char** grid = (char**)malloc(sizeof(char*) * cell_w);
  for(int i = 0; i < cell_w; i++)
  {
    grid[i] = (char*)malloc(sizeof(char) * cell_h);
    memset(grid[i], 0, sizeof(char) * cell_h);
  }


  //initialize the window
  SDL_Window* window = SDL_CreateWindow(win_title.c_str(),
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        win_width, win_height, 0);
  if(!window)
  {
    printf("ERROR %s: SDL Window Creation Failed!\n", SDL_GetError());
    SDL_Quit();
    return -1;
  }

  //initialize the renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(!renderer)
  {
    printf("Error %s: SDL Renderer Creation Failed!", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }
  //mouse x and y
  int mousex = 0;
  int mousey = 0;
  //grid location of mouse
  int gridx = 0;
  int gridy = 0;

  bool drawing = false;
  bool drawn = false;

  //main loop
  int quit = 0;
  while(!quit)
  {
    //grab inputs
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
      switch(e.type)
      {
        case SDL_QUIT://close window
          quit = 1;
          break;
        case SDL_MOUSEBUTTONDOWN:
          if(e.button.button == SDL_BUTTON_LEFT)//on left click
            drawing = true;
          break;
        case SDL_MOUSEBUTTONUP:
          if(e.button.button == SDL_BUTTON_LEFT)
            drawing = false;
          break;
        default:
          break;
      }
    }
    //grab mouse location and calculate the grid location
    SDL_GetMouseState(&mousex, &mousey);
    int pgridx = gridx;
    int pgridy = gridy;
    gridx = mousex / cell_size;
    gridy = mousey / cell_size;
    if(pgridx != gridx || pgridy != gridy) drawn = false;
    //std::cout << gridx << ", " << gridy << "(" << ((drawing)?"y":"n") << ")" << std::endl;
    if(drawing && !drawn)
    {
      grid[gridx][gridy] = !grid[gridx][gridy];
      drawn = true;
    }
    //do rendering
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    for(int x = 0; x < cell_w; x++)
    {
      //draw vertical line
      SDL_RenderDrawLine(renderer, x * cell_size, 0, x * cell_size, win_height);
      for(int y =  0; y < cell_h; y++)
      {
        //draw line only once
        if(x == 0) SDL_RenderDrawLine(renderer, 0, y * cell_size, win_width, y * cell_size);
        if(grid[x][y] != '\0')
        {
          SDL_Rect cell = {x * cell_size, y * cell_size, cell_size, cell_size};
          SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
          SDL_RenderFillRect(renderer, &cell);
        }
      }
    }

    SDL_RenderPresent(renderer);
    //finish rendering
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
