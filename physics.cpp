#include "physics.hpp"

#include <stdio.h>

#include "imgui.h"
#include "imguiRenderGL.h"
#include "file_utils.hpp"

#include "vector3.hpp"

#pragma warning(disable: 4996)

namespace physics
{
  static int mouseX = 0, mouseY = 0, mouseButtons = 0;
  static int g_width, g_height;
  unique_ptr<RenderWindow> g_renderWindow;

  GLuint g_vbo;
  GLuint g_shaderProgram, g_vertexShader, g_fragmentShader;

#ifdef _WIN32
  string g_base = "/projects/physics/";
#else
  string g_base = "/Users/dooz/projects/physics/";
#endif

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
    imguiBeginFrame(mouseX, g_height - 1 - mouseY, mouseButtons, 0);

    static const int paramSize = 200;
    static int paramScroll = 0;
    imguiBeginScrollArea("Params", 0, g_height - paramSize, g_width, paramSize, &paramScroll);

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
    glOrtho(0, g_width, 0, g_height, -1.0f, 1.0f);

    imguiRenderGLDraw();

    glDisable(GL_BLEND);
  }

  //---------------------------------------------------------------------------------
  static void RenderFrame()
  {
    glClearColor(0 / 255.0f, 0x2b / 255.0f, 0x36 / 255.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PreRenderUi();

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

    PostRenderUi();

    g_renderWindow->display();
  }
}


int main(int argc, char** argv)
{
  using namespace physics;
#ifdef _WIN32
  g_width = GetSystemMetrics(SM_CXFULLSCREEN);
  g_height = GetSystemMetrics(SM_CYFULLSCREEN);
#else
  auto displayId = CGMainDisplayID();
  g_width = CGDisplayPixelsWide(displayId);
  g_height = CGDisplayPixelsHigh(displayId);
#endif

  sf::ContextSettings settings;
  g_renderWindow.reset(new RenderWindow(sf::VideoMode(8 * g_width / 10, 8 * g_height / 10), "...", sf::Style::Default, settings));
  g_renderWindow->setVerticalSyncEnabled(true);

  glViewport(0, 0, g_width, g_height);

  GLuint res = glewInit();
  if (res != GLEW_OK)
    return 1;

  if (!Init())
    return 1;

  imguiRenderGLInit((g_base + "gfx/04b_24_.ttf").c_str());

  bool done = false;
  while (g_renderWindow->isOpen() && !done)
  {
    Event event;
    while (g_renderWindow->pollEvent(event))
    {
      switch (event.type)
      {
      case Event::KeyReleased:
        if (event.key.code == sf::Keyboard::Escape)
          done = true;
        break;

      case Event::MouseMoved:
        mouseX = event.mouseMove.x;
        mouseY = event.mouseMove.y;
      break;

      case Event::MouseButtonPressed:
        if (event.mouseButton.button == sf::Mouse::Left)
          mouseButtons |= 1;
        else if (event.mouseButton.button == sf::Mouse::Right)
          mouseButtons |= 2;
      break;

      case Event::MouseButtonReleased:
      if (event.mouseButton.button == sf::Mouse::Left)
        mouseButtons &= ~1;
      else if (event.mouseButton.button == sf::Mouse::Right)
        mouseButtons &= ~2;
      break;
      }
    }

/*
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
*/
    RenderFrame();
  }

  g_renderWindow.reset();

  return 0;
}
