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

int currentState = num_of_states - 1; // this is the state that will replace a certain cellon mouse click
//std::cout << "Selected State: " << currentState << std::endl;

bool drawing = false;
bool drawn = false;

bool running = false;
bool step = false;

//main loop
int quit = 0;

std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now(); /*nano time*/
double ns = 1000000000.0 / 60.0;
double delta = 0;

while(!quit)
{
  //get mouse location
  SDL_GetMouseState(&mousex, &mousey);
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now(); /*nano time*/
  delta += (now - lastTime).count() / ns;
  lastTime = now;
  while(delta >= 1)
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
          {
            drawing = false;
            drawn = false;
          }
          break;
        case SDL_KEYUP:
          switch(e.key.keysym.sym)
          {
            case SDLK_SPACE:
              running = !running;
              std::cout << ((running) ? "Running" : "Stopped") << std::endl;
              break;
            case SDLK_s:
              step = true;
              break;
            case SDLK_c:
              //std::cout << "Clearing Screen..." << std::endl;
              for(int i = 0; i < cell_w; i++)
              {
                memset(nextgrid[i], 0, sizeof(int) * cell_h);
                memset(grid[i], 0, sizeof(int) * cell_h);
              }
              break;
            case SDLK_r:
              //std::cout << "Randomly Initializing..." << std::endl;
              for(int x = 0; x < cell_w; x++)
              {
                for(int y =  0; y < cell_h; y++)
                {
                  nextgrid[x][y] = (rand() % num_of_states);
                }
              }
              break;
            case SDLK_RIGHT:
            case SDLK_UP:
              currentState = currentState + 1;
              if(currentState >= num_of_states) currentState = 0;
              //std::cout << "Selected State: " << currentState << std::endl;
              break;
            case SDLK_LEFT:
            case SDLK_DOWN:
              currentState = currentState - 1;
              if(currentState < 0) currentState = num_of_states - 1;
              //std::cout << "Selected State: " << currentState << std::endl;
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
    //calculate the grid location
    int pgridx = gridx;
    int pgridy = gridy;
    gridx = mousex / cell_size;
    gridy = mousey / cell_size;
    if(pgridx != gridx || pgridy != gridy) drawn = false;
    //std::cout << gridx << ", " << gridy << "(" << ((drawing)?"y":"n") << ")" << std::endl;
    if(drawing && !drawn)
    {
      //this is here cause we are assuming 2 states for now
      nextgrid[gridx][gridy] = currentState;
      drawn = true;
    }
    for(int x = 0; x < cell_w; x++)
    {
      for(int y =  0; y < cell_h; y++)
      {
        grid[x][y] = nextgrid[x][y];
      }
    }
    //calculate next grid
    if(running || step)
    {
      for(int x = 0; x < cell_w; x++)
      {
        for(int y =  0; y < cell_h; y++)
        {
          //calculate neighbor numbers
          int* neighbor_counts = (int*)malloc(sizeof(int) * num_of_states);//2 will be replaced by thenumber of states in the future
          memset(neighbor_counts, 0, sizeof(int) * num_of_states);

          int kxoff = -(kernal_width / 2);
          int kyoff = -(kernal_height / 2);
          for(int kx = 0; kx < kernal_width; kx++)
          {
            for(int ky = 0; ky < kernal_height; ky++)
            {
              int ix = (((x + kx + kxoff) % cell_w) + cell_w) % cell_w; //probably unnecessary parentheses but it makes the order clear
              int iy = (((y + ky + kyoff) % cell_h) + cell_h) % cell_h;
              if(ix == x && iy == y) continue;//dont count cell we are looking at
              neighbor_counts[grid[ix][iy]]++;
            }
          }

          lua_getglobal(lua, "process");
          lua_pushinteger(lua, grid[x][y]);//current state argument
          lua_createtable(lua, num_of_states, 0); //neighbor_counts argument
          for(int i = 0; i < num_of_states; i++)//2 will be replaced by num of states
          {
            lua_pushinteger(lua, neighbor_counts[i]);
            lua_rawseti(lua, -2, i);
          }

          if(checkLua(lua, lua_pcall(lua, 2, 1, 0)))
          {
            //top of the stack has the new state
            if(lua_isnumber(lua, -1))
            {
              //makes sure the return value is a valid state
              int new_state = (int)(lua_tonumber(lua, -1));
              nextgrid[x][y] = ((new_state % num_of_states) + num_of_states) % num_of_states;
              lua_pop(lua, 1);
            }
            else
            {
              std::cout << "Error reading result from process function!" << std::endl;
            }
          }
          else
          {
            std::cout << "uhoh bad time" << std::endl;
          }
        }
      }
    }
    delta--;
  }

  //do rendering
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer);
  //SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  for(int x = 0; x < cell_w; x++)
  {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    //draw vertical line
    SDL_RenderDrawLine(renderer, x * cell_size, 0, x * cell_size, win_height);
    for(int y =  0; y < cell_h; y++)
    {
      //draw line only once
      if(x == 0)
      {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderDrawLine(renderer, 0, y * cell_size, win_width, y * cell_size);
      }
      //draw cell if the cell is not in state 0 will change when adding multiple states
      if(grid[x][y] != 0)
      {
        SDL_Rect cell = {x * cell_size, y * cell_size, cell_size, cell_size};
        SDL_SetRenderDrawColor(renderer, (int)(0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))),
                                         (int)(0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))),
                                         (int)(0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))),
                                         0xFF);
        SDL_RenderFillRect(renderer, &cell);
      }
    }
  }

  //draw circle around cursor for state
  SDL_SetRenderDrawColor(renderer, (int)(0xFF * (1 - ((double)currentState / (num_of_states - 1)))),
                                   (int)(0xFF * (1 - ((double)currentState / (num_of_states - 1)))),
                                   (int)(0xFF * (1 - ((double)currentState / (num_of_states - 1)))),
                                   0xFF);
  drawCircle(renderer, mousex, mousey, 5);

  SDL_RenderPresent(renderer);
  //finish rendering
  if(step) step = false;
}

lua_close(lua);
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
SDL_Quit();
