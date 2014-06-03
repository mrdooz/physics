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
  unique_ptr<sf::Window> g_renderWindow;

  GLuint g_vbo, g_ibo;
  GLuint g_shaderProgram, g_vertexShader, g_fragmentShader;
  vector<u32> g_indices;


#ifdef _WIN32
  string g_base = "/projects/physics/";
#else
  string g_base = "/Users/dooz/projects/physics/";
#endif

  void CheckOpenGLError(const char* stmt, const char* fname, int line)
  {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
      printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
      abort();
    }
  }

#define _DEBUG

#ifdef _DEBUG
    #define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

  void CheckCompat()
  {
    GLint maxRectTextureSize;
    GLint myMaxTextureUnits;
    GLint myMaxTextureSize;
    const GLubyte * strVersion;
    const GLubyte * strExt;
    float myGLVersion;
    GLboolean isVAO, isTexLOD, isColorTable, isFence, isShade;
    GLboolean isTextureRectangle;
    strVersion = glGetString (GL_VERSION); // 1
    sscanf((char *)strVersion, "%f", &myGLVersion);
    strExt = glGetString (GL_EXTENSIONS); // 2
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &myMaxTextureUnits); // 3
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &myMaxTextureSize); // 4
    isVAO = gluCheckExtension ((const GLubyte*)"GL_APPLE_vertex_array_object",strExt); // 5
    isFence = gluCheckExtension ((const GLubyte*)"GL_APPLE_fence", strExt); // 6
    isShade = gluCheckExtension ((const GLubyte*)"GL_ARB_shading_language_100", strExt); // 7
    isColorTable =
        gluCheckExtension ((const GLubyte*)"GL_SGI_color_table", strExt) ||
        gluCheckExtension ((const GLubyte*)"GL_ARB_imaging", strExt); // 8
    isTexLOD = gluCheckExtension ((const GLubyte*)"GL_SGIS_texture_lod", strExt) || (myGLVersion >= 1.2); // 9
    isTextureRectangle = gluCheckExtension ((const GLubyte*)"GL_EXT_texture_rectangle", strExt);
    if (isTextureRectangle)
      glGetIntegerv (GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT, &maxRectTextureSize);
    else
      maxRectTextureSize = 0;
  }

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

    GL_CHECK(glAttachShader(g_shaderProgram, g_vertexShader));
    GL_CHECK(glAttachShader(g_shaderProgram, g_fragmentShader));

    GL_CHECK(glLinkProgram(g_shaderProgram));
    return true;
  }

  //---------------------------------------------------------------------------------
  bool Init()
  {

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
        g_indices.push_back(verts.size());
        verts.push_back(p + sphereCenter);
      }
    }

    GL_CHECK(glDisableClientState(GL_NORMAL_ARRAY));
    GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
    GL_CHECK(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
    GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
    GL_CHECK(glEnableClientState(GL_INDEX_ARRAY));

    GL_CHECK(glGenBuffers(1, &g_vbo));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, g_vbo));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vector3), verts.data(), GL_STATIC_DRAW));

    GL_CHECK(glGenBuffers(1, &g_ibo));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_indices.size() * sizeof(u32), g_indices.data(), GL_STATIC_DRAW));

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
  void Perspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
  {
    // replacement for gluPerspective
    GLdouble h = tan(fovy/2 * PI / 180) * zNear;
    GLdouble w = aspect * h;
    glFrustum(-w, w, -h, h, zNear, zFar);
  }

  //---------------------------------------------------------------------------------
  static void RenderFrame()
  {
    glClearColor(0 / 255.0f, 0x2b / 255.0f, 0x36 / 255.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //PreRenderUi();

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    Perspective(45.0f,(GLfloat)g_width / (GLfloat)g_height, 1, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // render stuff
    GL_CHECK(glUseProgram(g_shaderProgram));

    GL_CHECK(glPointSize(2));


    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, g_vbo));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
//      GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
//    GL_CHECK(glVertexPointer(3, GL_FLOAT, sizeof(Vector3), 0));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo));
//    GL_CHECK(glDrawArrays(GL_POINTS, 0, 10000));

    GL_CHECK(glDrawElements(GL_POINTS, 10000, GL_UNSIGNED_BYTE, 0));

    GL_CHECK(glUseProgram(0));

    //PostRenderUi();

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

  g_width = (int)(0.8f * g_width);
  g_height = (int)(0.8f * g_height);

  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 0;
  g_renderWindow.reset(new sf::Window(sf::VideoMode(g_width, g_height), "...", sf::Style::Default, settings));
  g_renderWindow->setVerticalSyncEnabled(true);

  CheckCompat();

  glViewport(0, 0, g_width, g_height);

  GLuint res = glewInit();
  if (res != GLEW_OK)
    return 1;

  if (!Init())
    return 1;

  //imguiRenderGLInit((g_base + "gfx/04b_24_.ttf").c_str());

  bool done = false;
  while (!done)
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
