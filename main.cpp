#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES

#include <cmath>
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

//camera variables
int fov = 45;
int near = 1;
int far = 2000;

float camerax = 320;
float cameray = 240;
float cameraz = -500;

float dx = 0;
float dy = 0;
float dz = 0;

float cameraMoveSpeed = 1.0f;
float cameraZoomSpeed = 2.0f;

float lookx = 0;
float looky = 0;
float lookz = 1;

float upx = 0;
float upy = 1;
float upz = 0;

//timing variables
std::chrono::steady_clock::time_point lastTime;
double ns = 1000000000.0 / 60.0;
double delta = 0;

int mousex = 0;
int mousey = 0;

int mouseGridX = -1;
int mouseGridY = -1;

float mouseGlobalX = 0.0f;
float mouseGlobalY = 0.0f;
float mouseGlobalZ = 0.0f;

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
  gluPerspective(fov, ratio, near, far);

  glMatrixMode(GL_MODELVIEW);
}

void drawCircle(float cx, float cy, float cz, float rad, int res)
{

  float twopi = 2 * M_PI;
  float step = twopi / res;
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(cx, cy, cz);
    for(float a = 0; a < twopi; a += step)
    {
      float x = cx + rad * cos(a);
      float y = cy + rad * sin(a);
      glVertex3f(x, y, cz);
    }
    glVertex3f(cx + rad, cy, cz);
  glEnd();
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
    //camera location
    if(dx || dy || dz)
    {
      camerax += dx;
      cameray += dy;
      cameraz += dz;
      if(cameraz > 0) cameraz = 0;
      //std::cout << camerax << ", " << cameray << ", " << cameraz << std::endl;
    }

    //if drawing && ! drawn
    if(drawing && !drawn)
    {
      if(mouseGridX != -1 && mouseGridY != -1)
        nextgrid[mouseGridX][mouseGridY] = currentState;
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
      step = false;
    }
    //finsih update
    delta--;
  }

  //render
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //reset transformations
  glLoadIdentity();
  //set the camera
  gluLookAt(camerax        , cameray        , cameraz,
            camerax + lookx, cameray + looky, cameraz + lookz,
            upx            , upy            , upz);

   //draw pointer for mouse
  glColor3f((0xFF * (1 - ((double)currentState / (num_of_states - 1)))) / 255.0f,
            (0xFF * (1 - ((double)currentState / (num_of_states - 1)))) / 255.0f,
            (0xFF * (1 - ((double)currentState / (num_of_states - 1)))) / 255.0f);
  drawCircle(mouseGlobalX, mouseGlobalY, 0, 5, 20);

  //draw grid
  glColor3f(0.0f, 0.0f, 0.0f);
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

  //draw cells
  glBegin(GL_QUADS);
  for(int x = 0; x < cell_w; x++)
  {
    for(int y = 0; y < cell_h; y++)
    {
      glColor3f((0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))) / 255.0f,
                (0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))) / 255.0f,
                (0xFF * (1 - ((double)grid[x][y] / (num_of_states - 1)))) / 255.0f);
      glVertex3f(x * cell_size, y * cell_size, 0);
      glVertex3f(x * cell_size, (y + 1) * cell_size, 0);
      glVertex3f((x + 1) * cell_size, (y + 1) * cell_size, 0);
      glVertex3f((x + 1) * cell_size, y * cell_size, 0);
    }
  }
  glEnd();

  glutSwapBuffers();
}

//get crtl, shfft, alt with int mod = glutGetModifiers();
void processNormalKeys(unsigned char key, int x, int y)
{
  //std::cout << "Normal key event: " << (int)key << std::endl;
  switch(key){
    case 27:
      exit(0);
      break;
    case 32:
      running = !running;
      std::cout << ((running) ? "Running" : "Stopped") << std::endl;
      break;
    case 'e':
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
    case 'w':
      dy = cameraMoveSpeed;
      break;
    case 'a':
      dx = cameraMoveSpeed;
      break;
    case 's':
      dy = -cameraMoveSpeed;
      break;
    case 'd':
      dx = -cameraMoveSpeed;
      break;
    default:
      break;
  }
  key = x + y;
}

void processNormalKeysUp(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'w':
    case 's':
      dy = 0;
      break;
    case 'a':
    case 'd':
      dx = 0;
      break;
    default:
      break;
  }
  key = x + y;
}

void processSpecialKeys(int key, int x, int y)
{
  //std::cout << "special key event" << std::endl;
  switch(key)
  {
    case GLUT_KEY_RIGHT:
    case GLUT_KEY_UP:
      currentState = currentState + 1;
      if(currentState >= num_of_states) currentState = 0;
      drawn = false;
      break;
    case GLUT_KEY_LEFT:
    case GLUT_KEY_DOWN:
      currentState = currentState - 1;
      if(currentState < 0) currentState = num_of_states - 1;
      drawn = false;
      break;
    default:
      break;
  }
  key = x + y;
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
        //std::cout << "Scroll " << ((button == 3) ? "Up" : "Down") << std::endl;
        cameraz += ((button == 3) ? -cameraZoomSpeed : cameraZoomSpeed);
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
        //dz = 0;
      }
      break;
    default:
      break;
  }
  button = x + y;
}

void processMotion(int x, int y)
{
  mousex = x;
  mousey = y;

  //calculates the mouse global coordinate location
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winx, winy;
  GLdouble worx, wory, worz;
  GLdouble worx1, wory1, worz1;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  winx = (float)x;
  winy = (float)viewport[3] - (float)y - 1;

  gluUnProject(winx, winy, 0.0f, modelview, projection, viewport, &worx, &wory, &worz);
  gluUnProject(winx, winy, 1.0f, modelview, projection, viewport, &worx1, &wory1, &worz1);

  float f = worz / (worz1 - worz);
  mouseGlobalX = worx - f * (worx1 - worx);
  mouseGlobalY = wory - f * (wory1 - wory);

  if((mouseGlobalX >= 0 && mouseGlobalX < cell_w * cell_size) &&
     (mouseGlobalY >= 0 && mouseGlobalY < cell_h * cell_size))
  {
    mouseGridX = floor(mouseGlobalX / cell_size);
    mouseGridY = floor(mouseGlobalY / cell_size);
  }
  else
  {
    mouseGridX = -1;
    mouseGridY = -1;
  }
  drawn = false;
  //std::cout << winx << ", " << winy << "(" << mouseGlobalX << ", " << mouseGlobalY << ", " << worz << ")" << std::endl;
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
    ns = 1000000000.0 / ups;

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

    if(lua_getglobal(lua, "cells_wide") != 0)
    {
      if(lua_isnumber(lua, -1))
        cell_w = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'cells_wide'!" << std::endl;
    }
    else
    {
      cell_w = win_width / cell_size;
    }

    if(lua_getglobal(lua, "cells_high") != 0)
    {
      if(lua_isnumber(lua, -1))
        cell_h = (int)lua_tonumber(lua, -1);
      else
        std::cout << "Error reading in variable 'cells_high'!" << std::endl;
    }
    else
    {
      cell_h = win_height / cell_size;
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

  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

  //register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutIdleFunc(renderScene); //when there is nothing to be processed call this function

  far = cell_w * cell_size * 4;
  //set camera
  if(win_height == 0) win_height = 1;
  float ratio = 1.0 * win_width / win_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, win_width, win_height);
  gluPerspective(fov, ratio, near, far);

  glMatrixMode(GL_MODELVIEW);

  camerax = (cell_w * cell_size) / 2.0f;
  cameray = (cell_h * cell_size) / 2.0f;
  cameraz = cell_w * cell_size * -0.9f;

  //keyboard and mouse events
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeysUp);
  glutSpecialFunc(processSpecialKeys);
  glutIgnoreKeyRepeat(1);

  glutMouseFunc(processMouse);
  glutMotionFunc(processMotion);
  glutPassiveMotionFunc(processMotion);

  glEnable(GL_DEPTH_TEST);

  //enter GLUT event processing cycle
  lastTime = std::chrono::steady_clock::now();
  glutMainLoop();

  lua_close(lua);
  return 0;
}
