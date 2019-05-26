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
  gluPerspective(45, ratio, 1, 1000);

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
      std::cout << camerax << ", " << cameray << ", " << cameraz << std::endl;
    }

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

  //reset transformations
  glLoadIdentity();
  //set the camera
  gluLookAt(camerax        , cameray        , cameraz,
            camerax + lookx, cameray + looky, cameraz + lookz,
            upx            , upy            , upz);

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

  //draw pointer for mouse
  drawCircle(0, 0, 0, 5, 20);

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
}

//float* matrixMultiply(float* a, float* b)
//{
//
//}

//for with a button press
void processMotion(int x, int y)
{
  mousex = x;
  mousey = y;

  //float proj[16];
  //float view[16];
  //glGetFloatv(GL_PROJECTION_MATRIX, proj);
  //glGetFloatv(GL_MODELVIEW_MATRIX, view);

  //float 

  mouseGlobalX = camerax + cameraz - mousex;
  mouseGlobalY = cameray + ((win_height / win_width) * cameraz) - mousey;
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

  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

  //register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutIdleFunc(renderScene); //when there is nothing to be processed call this function

  //keyboard and mouse events
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeysUp);
  glutSpecialFunc(processSpecialKeys);
  glutIgnoreKeyRepeat(1);
  glutSpecialUpFunc(processSpecialKeysUp);

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
