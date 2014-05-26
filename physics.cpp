#include "physics.hpp"

#include <stdio.h>

#include <GL/glew.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <GL/GLU.h>
#else
#include <OpenGL/glu.h>
#include <unistd.h>
#endif


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imguiRenderGL.h"
#include "file_utils.hpp"

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "sdl2.lib")
#pragma comment(lib, "glew32.lib")
#endif

#include "vector3.hpp"

namespace physics
{
  SDL_Window* displayWindow;

  static int mouseX = 0, mouseY = 0, mouseButtons = 0;
  static int g_width, height;

  GLuint g_vbo;
  GLuint g_shaderProgram, g_vertexShader, g_fragmentShader;

  string g_base = "/Users/dooz/projects/physics/";

  //---------------------------------------------------------------------------------
  GLuint loadShader(const char* filename, GLuint type)
  {
    GLuint shader = glCreateShader(type);
    vector<char> buf;
    if (!LoadTextFile((g_base+filename).c_str(), &buf))
      return -1;

    const char* data = buf.data();
    glShaderSource(shader, 1, &data, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
      GLint length;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
      vector<char> buf(length);
      glGetShaderInfoLog(shader, length, &length, buf.data());
      printf("%s", buf.data());
    }

    return shader;
  }

  //---------------------------------------------------------------------------------
  bool InitShaders()
  {
    g_shaderProgram   = glCreateProgram();
    g_vertexShader    = loadShader("simple.vert.glsl", GL_VERTEX_SHADER);
    g_fragmentShader  = loadShader("simple.frag.glsl", GL_FRAGMENT_SHADER);

    if (g_vertexShader == -1 || g_fragmentShader == -1)
      return false;

    glAttachShader(g_shaderProgram, g_vertexShader);
    glAttachShader(g_shaderProgram, g_fragmentShader);

    glLinkProgram(g_shaderProgram);
    return true;
  }

  //---------------------------------------------------------------------------------
  bool Init()
  {
    glGenBuffers(1, &g_vbo);

    vector<Vector3> verts;

    float sphereRadius = 5;
    Vector3 sphereCenter(0,0,0);

    while (verts.size() < 10000)
    {
      float x = -1 + 2 * (rand() / (float)RAND_MAX);
      float y = -1 + 2 * (rand() / (float)RAND_MAX);
      float z = -1 + 2 * (rand() / (float)RAND_MAX);

      Vector3 p(x,y,z);
      p *= sphereRadius;

      if (Length(p) < sphereRadius)
      {
        verts.push_back(p + sphereCenter);
      }
    }

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vector3), verts.data(), GL_STATIC_DRAW);

    return InitShaders();
  }

  //---------------------------------------------------------------------------------
  static void PreRenderUi()
  {
    imguiBeginFrame(mouseX, height - 1 - mouseY, mouseButtons, 0);

    static const int paramSize = 200;
    static int paramScroll = 0;
    imguiBeginScrollArea("Params", 0, height - paramSize, g_width, paramSize, &paramScroll);

    static float xPos = 0.0f;
    imguiSlider("xPos", &xPos, -1000, 1000.0f, 1);

    static float yPos = 20.0f;
    imguiSlider("yPos", &yPos, -1000, 1000.0f, 1);

    static float zPos = 1000.0f;
    imguiSlider("zPos", &zPos, 1, 1000.0f, 1);

    char buf[256];
    sprintf(buf, "mouse: %d, %d", mouseX, mouseY);
    imguiLabel(buf);
  }

  //---------------------------------------------------------------------------------
  static void PostRenderUi()
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_width, 0, height, -1.0f, 1.0f);

    imguiRenderGLDraw();

    glDisable(GL_BLEND);
  }

  //---------------------------------------------------------------------------------
  static void RenderFrame()
  {
    glClearColor(0 / 255.0f, 0x2b / 255.0f, 0x36 / 255.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //PreRenderUi();

    glLoadIdentity();
    //glMatrixMode(GL_PROJECTION);
    //gluPerspective(45.0f,(GLfloat) g_width /(GLfloat)height,0.1f,1000.0f);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

    // render stuff
    glUseProgram(g_shaderProgram);

    glPointSize(2);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_POINTS, 0, 10000);

    glUseProgram(0);

    //PostRenderUi();


    SDL_GL_SwapWindow(displayWindow);
  }
}


int main(int argc, char** argv)
{
  using namespace physics;
  g_width = 1024;
  height = 768;
  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO);
  displayWindow = SDL_CreateWindow(
    "SDL2/OpenGL Demo", 100, 100, g_width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  SDL_GLContext glcontext = SDL_GL_CreateContext(displayWindow);
  glViewport(0, 0, g_width, height);

  GLuint res = glewInit();
  if (res != GLEW_OK)
    return 1;

  if (!Init())
    return 1;

  imguiRenderGLInit((g_base + "gfx/04b_24_.ttf").c_str());

  bool quit = false;
  while (!quit)
  {
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        quit = true;
        break;

      case SDL_MOUSEMOTION:
        mouseX = event.motion.x;
        mouseY = event.motion.y;
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          mouseButtons |= 1;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT)
        {
          mouseButtons |= 2;
        }
        break;

      case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT)
        {
          mouseButtons &= ~1;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT)
        {
          mouseButtons &= ~2;
        }
        break;
      }
    }
    RenderFrame();
  }

  SDL_GL_DeleteContext(glcontext);
  SDL_Quit();

  return 0;
}
