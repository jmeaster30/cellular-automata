#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <ctime>
#include <chrono>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

extern "C"
{
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

bool checkLua(lua_State* lua, int r)
{
  if(r != LUA_OK)
  {
    std::string err = lua_tostring(lua, -1);
    std::cout << err << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char** argv)
{
  std::string inputfile = "";
  if(argc < 2)
  {
    std::cout << "too few arguments: please specify a lua file that contains the settings and rules for the automata" << std::endl;
    return -1;
  }
  else if(argc > 2)
  {
    std::cout << "too many arguments: please specify a lua file that contains the settings and rules for the automata" << std::endl;
    return -1;
  }
  else
  {
    inputfile = argv[1];
  }

  std::string win_title = "Cellular Automata";

  int win_width = 800;
  int win_height = 600;
  int cell_size = 20;

  lua_State *lua = luaL_newstate();
  //luaL_openlibs(lua);

  if(checkLua(lua, luaL_dofile(lua, inputfile.c_str())))
  {
    std::cout << "Lua file loaded successfully!!!" << std::endl;
    lua_getglobal(lua, "win_title");
    if(lua_isstring(lua, -1))
      win_title = lua_tostring(lua, -1);
    else
      std::cout << "Error reading in variable 'win_title'!" << std::endl;

    lua_getglobal(lua, "win_width");
    if(lua_isnumber(lua, -1))
      win_width = (int)lua_tonumber(lua, -1);
    else
      std::cout << "Error reading in variable 'win_width'!" << std::endl;

    lua_getglobal(lua, "win_height");
    if(lua_isnumber(lua, -1))
      win_height = (int)lua_tonumber(lua, -1);
    else
      std::cout << "Error reading in variable 'win_height'!" << std::endl;

    lua_getglobal(lua, "cell_size");
    if(lua_isnumber(lua, -1))
      cell_size = (int)lua_tonumber(lua, -1);
    else
      std::cout << "Error reading in variable 'cell_size'!" << std::endl;
  }
  else
  {
    std::cout << "There was a problem loading the lua file :(" << std::endl;
  }


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

  //initialize SDL stuff
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
  {
    std::cout << "ERROR " << SDL_GetError() << ": SDL Initialization Failed!" << std::endl;
    return -1;
  }

  //initialize the window
  SDL_Window* window = SDL_CreateWindow(win_title.c_str(),
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        win_width, win_height, 0);
  if(!window)
  {
    std::cout << "ERROR " << SDL_GetError() << ": SDL Window Creation Failed!" << std::endl;
    lua_close(lua);
    SDL_Quit();
    return -1;
  }

  //initialize the renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(!renderer)
  {
    std::cout << "ERROR " << SDL_GetError() << ": SDL Renderer Creation Failed!" << std::endl;
    lua_close(lua);
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

  bool running = false;

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
        case SDL_KEYUP:
          if(e.key.keysym.sym == SDLK_SPACE)
            running = !running;
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

  lua_close(lua);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
