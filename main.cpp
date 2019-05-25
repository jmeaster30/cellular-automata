#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <chrono>
#include <string>

#include <GL/glut.h>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_timer.h>

extern "C"
{
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

//Controllable variables

const char* win_title;//default: "Cellular Automata"

int ups;//default: 60

int win_width;//default: 640
int win_height;//default: 480

int cell_size;//default: 20
int cell_w;
int cell_h;

int** grid;
int** nextgrid;

int num_of_states;//default: 2

int kernal_width;//default: 3
int kernal_height;//default: 3

//sim variables
lua_State *lua;

int currentState = 0;

bool running = false;
bool step = false;

bool drawing = false;
bool drawn = false;

//timing variables
std::chrono::steady_clock::time_point lastTime;
double ns = 1000000000.0 / 60.0;
double delta = 0;

int mousex = 0;
int mousey = 0;

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

void changeSize(int w, int h)
{
  if(h == 0) h = 1;
  float ratio = 1.0 * w / h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, w, h);
  gluPerspective(45, ratio, 1, 1000);

  glMatrixMode(GL_MODELVIEW);
}

void renderScene(void)
{
  //update
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now(); /*nano time*/
  delta += (now - lastTime).count() / ns;
  lastTime = now;
  while(delta >= 1)
  {
    //do update
    //calculate grid location of the mouse


    //if drawing && ! drawn
    if(drawing && !drawn)
    {
      //nextgrid[gridx][gridy] = currentState;
      drawn = true;
    }
    //copy nextgrid->grid
    for(int x = 0; x < cell_w; x++)
    {
      for(int y = 0; y < cell_h; y++)
      {
        grid[x][y] = nextgrid[x][y];
      }
    }
    //calculate next grid
    //if running || step
    if(running || step)
    {

    }
    //finsih update
    delta--;
  }

  //render
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //set the camera
  glLoadIdentity();
  //draw grid
  for(int x = 0; x < cell_w + 1; x++)
  {
    for(int y = 0; y < cell_h + 1; y++)
    {
      glBegin(GL_LINES);
      if(x != cell_w)
      {
        glVertex3f(x * cell_size, y * cell_size, 0);
        glVertex3f((x + 1) * cell_size, y * cell_size, 0);
      }
      if(y != cell_h)
      {
        glVertex3f(x * cell_size, y * cell_size, 0);
        glVertex3f(x * cell_size, (y  + 1) * cell_size, 0);
      }
      glEnd();
    }
  }

  //color cells
  glColor3f(0.9f, 0.9f, 0.9f);
  glBegin(GL_TRIANGLES);
    glVertex3f(-0.5, -0.5, 0.0);
    glVertex3f(0.5, 0.0, 0.0);
    glVertex3f(0.0, 0.5, 0.0);
  glEnd();

  glutSwapBuffers();
}

//get crtl, shfft, alt with int mod = glutGetModifiers();
void processNormalKeys(unsigned char key, int x, int y)
{
  std::cout << "Normal key event: " << (int)key << std::endl;
  switch(key){
    case 27:
      exit(0);
      break;
    case 32:
      running = !running;
      std::cout << ((running) ? "Running" : "Stopped") << std::endl;
      break;
    case 's':
      step = true;
      break;
    case 'c':
      for(int i = 0; i < cell_w; i++)
      {
        memset(nextgrid[i], 0, sizeof(int) * cell_h);
        memset(grid[i], 0, sizeof(int) * cell_h);
      }
      break;
    case 'r':
      for(int x = 0; x < cell_w; x++)
      {
        for(int y =  0; y < cell_h; y++)
        {
          nextgrid[x][y] = (rand() % num_of_states);
        }
      }
      break;
    default:
      break;
  }
}

void processSpecialKeys(int key, int x, int y)
{
  std::cout << "special key event" << std::endl;
  switch(key)
  {
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_UP:
      currentState = currentState + 1;
      if(currentState >= num_of_states) currentState = 0;
      break;
    case GLUT_KEY_LEFT:
    case GLUT_KEY_DOWN:
      currentState = currentState - 1;
      if(currentState < 0) currentState = num_of_states - 1;
      break;
    default:
      break;
  }
}

void processSpecialKeysUp(int key, int x, int y)
{

}

void processMouse(int button, int state, int x, int y)
{
  switch(state)
  {
    case GLUT_DOWN:
      if(button == GLUT_LEFT_BUTTON)
      {
        drawing = true;
      }
      else if(button == 3 || button == 4)
      {
        std::cout << "Scroll " << ((button == 3) ? "Up" : "Down") << std::endl;
      }
      break;
    case GLUT_UP:
      if(button == GLUT_LEFT_BUTTON)
      {
        drawing = false;
        drawn = false;
      }
      else if(button == 3 || button == 4)
      {
        //std::cout << "Scroll " << ((button == 3) ? "Up" : "Down") << "Stopped" << std::endl;
      }
      break;
    default:
      break;
  }
}

//for with a button press
void processMotion(int x, int y)
{
  mousex = x;
  mousey = y;
  //std::cout << x << ", " << y << std::endl;
}

//for every other time
void processPassiveMotion(int x, int y)
{
  mousex = x;
  mousey = y;
  //std::cout << x << ", " << y << std::endl;
}

int main(int argc, char** argv)
{
  srand(time(0));
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

  win_title = "";//default: "Cellular Automata"

  ups = 0;//default: 60

  win_width = 0;//default: 640
  win_height = 0;//default: 480

  cell_size = 0;//default: 20

  num_of_states = 0;//default: 2

  kernal_width = 0;//default: 3
  kernal_height = 0;//default: 3

  lua = luaL_newstate();
  luaL_openlibs(lua);

  if(checkLua(lua, luaL_dofile(lua, inputfile.c_str())))
  {
    std::cout << "Lua file loaded successfully!!!" << std::endl;
    if(lua_getglobal(lua, "win_title") != 0)
    {
      if(lua_isstring(lua, -1))
        win_title = lua_tostring(lua, -1);
      else
        std::cout << "Error reading in variable 'win_title'!" << std::endl;
    }
    else
    {
      win_title = "Cellular Automata";
    }

    if(lua_getglobal(lua, "updates_per_second") != 0)
    {
      if(lua_isnumber(lua, -1))
        ups = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'updates_per_second'!" << std::endl;
    }
    else
    {
      ups = 60;
    }
    double ns = 1000000000.0 / ups;

    if(lua_getglobal(lua, "win_width") != 0)
    {
      if(lua_isnumber(lua, -1))
        win_width = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'win_width'!" << std::endl;
    }
    else
    {
      win_width = 640;
    }


    if(lua_getglobal(lua, "win_height") != 0)
    {
      if(lua_isnumber(lua, -1))
        win_height = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'win_height'!" << std::endl;
    }
    else
    {
      win_height = 480;
    }

    if(lua_getglobal(lua, "cell_size") != 0)
    {
      if(lua_isnumber(lua, -1))
        cell_size = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'cell_size'!" << std::endl;
    }
    else
    {
      cell_size = 20;
    }

    if(lua_getglobal(lua, "num_of_states") != 0)
    {
      if(lua_isnumber(lua, -1))
        num_of_states = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'num_of_states'!" << std::endl;
    }
    else
    {
      num_of_states = 2;
    }
    currentState = num_of_states - 1;

    if(lua_getglobal(lua, "kernal_width") != 0)
    {
      if(lua_isnumber(lua, -1))
        kernal_width = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'kernal_width'!" << std::endl;
    }
    else
    {
      kernal_width = 3;
    }

    if(lua_getglobal(lua, "kernal_height") != 0)
    {
      if(lua_isnumber(lua, -1))
        kernal_height = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'kernal_height'!" << std::endl;
    }
    else
    {
      kernal_height = 3;
    }
  }
  else
  {
    std::cout << "There was a problem loading the lua file :(" << std::endl;
  }

  cell_w = win_width / cell_size;
  cell_h = win_height / cell_size;

  //grid[x][y]
  //where 0 <= x <= cell_w
  //where 0 <= y <= cell_h
  grid = (int**)malloc(sizeof(int*) * cell_w);
  for(int i = 0; i < cell_w; i++)
  {
    grid[i] = (int*)malloc(sizeof(int) * cell_h);
    memset(grid[i], 0, sizeof(int) * cell_h);
  }

  //nextgrid[x][y]
  //where 0 <= x <= cell_w
  //where 0 <= y <= cell_h
  nextgrid = (int**)malloc(sizeof(int*) * cell_w);
  for(int i = 0; i < cell_w; i++)
  {
    nextgrid[i] = (int*)malloc(sizeof(int) * cell_h);
    memset(nextgrid[i], 0, sizeof(int) * cell_h);
  }

  //initialize glut and create the Window
  glutInit(&argc, argv);
  glutInitWindowPosition(-1, -1); //window position doesn't matter
  glutInitWindowSize(win_width, win_height);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow(win_title);

  //register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutIdleFunc(renderScene); //when there is nothing to be processed call this function

  //keyboard and mouse events
  glutKeyboardFunc(processNormalKeys);
  glutSpecialFunc(processSpecialKeys);
  glutIgnoreKeyRepeat(1);
  glutSpecialUpFunc(processSpecialKeysUp);

  glutMouseFunc(processMouse);
  glutMotionFunc(processMotion);
  glutPassiveMotionFunc(processPassiveMotion);

  glEnable(GL_DEPTH_TEST);

  //enter GLUT event processing cycle
  lastTime = std::chrono::steady_clock::now();
  glutMainLoop();

  lua_close(lua);
  return 0;
}
