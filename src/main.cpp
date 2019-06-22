/*
 * mc
 */

#include <GL/glew.h>

#define HAVE_M_PI  // SDL_stdinc.h has M_PI and it collides with Windows M_PI
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <signal.h>
#include <math.h>
//#include <x86intrin.h>
#include <intrin.h>

#include "cglm/cglm.h"

#include "shaders.h"

#include "base.h"
#include "storage.cpp"

#include "file.cpp"
#include "img.h"
#include "img.cpp"

#include "render.cpp"
#include "loadAsset.cpp"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO  // includes stdio.h
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gles2.h"

#define CUSTOM_3D_SPACE_MAX_X   1000.0f
#define CUSTOM_3D_SPACE_MAX_Y   1000.0f
#define CUSTOM_3D_SPACE_MAX_Z   1000.0f
#define CUSTOM_3D_SPACE_MIN_X  -1000.0f
#define CUSTOM_3D_SPACE_MIN_Y  -1000.0f
#define CUSTOM_3D_SPACE_MIN_Z  -1000.0f

#define NK_MAX_VERTEX_MEMORY    512 * 1024
#define NK_MAX_ELEMENT_MEMORY   128 * 1024


typedef struct Camera
{
  vec3 pos;
  float yaw;
  float roll;
  float pitch;
} Camera;

typedef struct Controls
{
  bool32 lmbState;
  bool32 rmbState;
  bool32 mmbState;
  uint16 mouseX;
  uint16 mouseY;
  float mouseSensitivity;
} Controls;

typedef struct State
{
  bool32 drawGUI;
  GLuint drawMode;
  bool32 running;
} State;

typedef struct GLAtom
{
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  GLuint texture;
} GLAtom;

#define INIT_WINDOW_WIDTH    1200
#define INIT_WINDOW_HEIGHT   900

typedef struct WindowParams
{
  uint16 width;
  uint16 height;
} WindowParams;

global_variable Camera camera = {};
global_variable Controls controls = {};
global_variable Controls controlsPrev = {};


global_variable uint64 perfCountFrequency;


void
SignalHandler(int signal)
{
  if (signal == SIGINT || SIGTERM) {
    SDL_Log("Received signal %d", signal);
    exit(EXIT_SUCCESS);
  }
}

float vertices[] = {
   // positions           // colors           // uvs
   200.0f,  120.0f,  0.0f,   0.0f, 1.0f, 1.0f,   1.0f, 1.0f,
   200.0f, -120.0f,  0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
  -200.0f, -120.0f,  0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
  -200.0f,  120.0f,  0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
};

GLint indices[] = {
  0, 1, 3,
  1, 2, 3
};


global_variable GLAtom glAtom = {};


void
SetGeoForRendering()
{
  /*
   * VAO - combines VBOs into one object.
   */
  glGenVertexArrays(
    1,       // vertex array object names count to generate
    &glAtom.vao// array of object names
  );
  // Make this VAO the active one (remember OpenGL is a state machine).
  // You can have different VBO and VAO active. This bind has only one argument
  // because VAO can have only one role - it's more of a wrapper.
  glBindVertexArray(glAtom.vao);

  /*
   * VBO - represents a single attribute. Those aren't usually rendered
   * directly.
   */
  glGenBuffers(
    1,               // buffer object names count to generate
    &glAtom.vbo      // array of object names
  );

  // Make this VBO the active one (remember OpenGL is a state machine).
  // You can have different VBO and VAO active. First argument specifies
  // the role of this buffer.
  glBindBuffer(GL_ARRAY_BUFFER, glAtom.vbo);
  // describe the data in the buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


  /*
   * EBO
   */
  glGenBuffers(
    1,
    &glAtom.ebo
  );
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glAtom.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), (GLvoid*)indices, GL_STATIC_DRAW);

  /*
   * TEXTURES
   */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenTextures(1, &glAtom.texture);
  glBindTexture(GL_TEXTURE_2D, glAtom.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
               FONT_TEST_IMAGE_WIDTH, FONT_TEST_IMAGE_HEIGHT,
               0, GL_RED, GL_UNSIGNED_BYTE,
               image);
  glGenerateMipmap(GL_TEXTURE_2D);
  // stbi_image_free(image);
}

/*
 * Compute the new camera position in radial coordinates.
 *
 * This function computes a position offset vector based on the yaw, pitch, roll.
 * It sets the position - means that it doesn't take in the account the original position.
 */
void
UpdateViewRotation(Camera* cam, mat4 view)
{
  vec3 trgt = {0.0f, 0.0f, 0.0f};
  vec3 camUpVector = {0.0f, 1.0f, 0.0f};
  float r = 1.0f;
  UNUSED(r);
  camera.pos[0] = cosf(camera.yaw) * cosf(camera.pitch);
  camera.pos[1] = sinf(camera.pitch) * sinf(camera.yaw);
  camera.pos[2] = cosf(camera.pitch) * sinf(camera.yaw);
  glm_lookat(camera.pos, trgt, camUpVector, view);
}


int
main(int argc, char *argv[])
{
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  Memory mem = {};
  if (!mem.isInitialized)
  {
    mem.isInitialized = true;
    mem.transientMemorySize = Megabytes(8);
    mem.persistentMemorySize = Megabytes(128);
    // TODO(michalc): need a better/cross platform malloc?
    mem.transientMemory = malloc(mem.transientMemorySize);
    mem.transientTail = mem.transientMemory;
    mem.persistentMemory = malloc(mem.persistentMemorySize);
    mem.persistentTail = mem.persistentMemory;
  }
  AssetTable assetTable = {};
  assetTable.storageMemory = &mem;

  LoadAsset("./resources/cube.obj", &assetTable, ASSET_MODEL3D_OBJ);
  Model3D modelOBJ = {};
  RetriveOBJ(0, &assetTable, &modelOBJ);

  char* fontPath = "./resources/fonts/OpenSans-Regular.ttf";
  LoadFont(fontPath);
  GetTextImage("All is well");

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    SDL_Log("Failed to SDL_Init");
    exit(EXIT_FAILURE);
  }

  GetDisplayInformation();
  perfCountFrequency = SDL_GetPerformanceFrequency();

  // Need to set some SDL specific things for OpenGL
  if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8)) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_Window* window = SDL_CreateWindow(
    "Node editor",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT,
    SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI
  );

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  if (glContext == NULL) {
    SDL_Log("%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  glViewport(0, 0, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

  glewExperimental = GL_TRUE;
  glewInit();  // glew will look up OpenGL functions
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  //glEnable(GL_CULL_FACE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // NUKLEAR init
  nk_context* nkCtx;
  nkCtx = nk_sdl_init(window);
  nk_font_atlas *atlas;
  nk_sdl_font_stash_begin(&atlas);
  nk_sdl_font_stash_end();

  controls.mouseSensitivity = 0.01f;

  // model matrix holds, translation, scaling, rotation
  mat4 model = {};
  glm_mat4_identity(model);
  glm_scale_uni(model, 0.7f);

  // view matrix represents point of view in the scene - camera
  camera.pos[0] = 0.00f;
  camera.pos[1] = 0.00f;
  camera.pos[2] = 0.30f;
  camera.yaw = M_PI/2.0f;
  camera.pitch = 0.0f;
  camera.roll = 0.0f;
  mat4 view = {};
  UpdateViewRotation(&camera, view);

  // projection holds the ortho/persp + frustrum
  mat4 projection;
#ifdef USING_PERSPECTIVE
  glm_perspective(90.0f,
                  1.0f,
                  CUSTOM_3D_SPACE_MIN_Z,
                  CUSTOM_3D_SPACE_MAX_Z,
                  projection);
#else
  glm_ortho(CUSTOM_3D_SPACE_MIN_X, CUSTOM_3D_SPACE_MAX_X,
            CUSTOM_3D_SPACE_MIN_Y, CUSTOM_3D_SPACE_MAX_Y,
            CUSTOM_3D_SPACE_MIN_Z, CUSTOM_3D_SPACE_MAX_Z,
            projection);
#endif


  SetGeoForRendering();
  GLuint vertexShader;
  GLuint fragmentShader;
  CompileShader(VS_texture, FS_default, &vertexShader, &fragmentShader);
  GLuint shaderProgram = SetShader(vertexShader, fragmentShader);

  // TODO (mc): fix this... it doesn't work

  GLint modelLoc = GetUniformLoc(shaderProgram, "model");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);

  GLint viewLoc = GetUniformLoc(shaderProgram, "view");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float *)view);

  GLint projectionLoc = GetUniformLoc(shaderProgram, "projection");
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float *)projection);

  // Linking vertex buffer with the attribute (saved in VAO).
  glBindVertexArray(glAtom.vao);
  glBindBuffer(GL_ARRAY_BUFFER, glAtom.vbo);

  GLint posAttrib = glGetAttribLocation(shaderProgram, "a_position");
  glEnableVertexAttribArray(posAttrib);
  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,
                        8*sizeof(float), (void*)0);

  GLint colorAttrib = glGetAttribLocation(shaderProgram, "a_color");
  glEnableVertexAttribArray(colorAttrib);
  glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                        8*sizeof(float), (void*)(3*sizeof(float)));

  GLint uvsAttrib = glGetAttribLocation(shaderProgram, "a_texCoords");
  glEnableVertexAttribArray(uvsAttrib);
  glVertexAttribPointer(uvsAttrib, 2, GL_FLOAT, GL_FALSE,
                        8*sizeof(float), (void*)(6*sizeof(float)));

  // Unbinding to be sure it's a clean slate before actual loop.
  // Doing that mostly to understand what's going on.
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);



  //
  // Set things UP.
  //
  ////////////////////////////////////////
  //
  // Prepare and LOOP.
  //


  uint8 framerateTarget = 60;
  float frametimeTarget = 1.0f/framerateTarget;


  WindowParams windowParameters = {};
  windowParameters.width = INIT_WINDOW_WIDTH;
  windowParameters.height = INIT_WINDOW_HEIGHT;
  State appState = {};
  appState.drawGUI = true;
  appState.drawMode = GL_FILL;
  appState.running = true;
  SDL_Event event;

  int m = 0;

  uint64 lastCounter = SDL_GetPerformanceCounter();
  uint64 lastCycleCount = __rdtsc();


  while (appState.running) {
    nk_input_begin(nkCtx);

    // EVENTS
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT: {
          appState.running = false;
        } break;

        case SDL_WINDOWEVENT: {
        } break;

        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE: {
              appState.running = false;
            } break;

            case SDLK_g: {
              appState.drawGUI = !appState.drawGUI;
            } break;

            case SDLK_r: {
              glClearColor(1.0, 0.0, 0.0, 1.0);
              glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
              SDL_GL_SwapWindow(window);
            } break;

            case SDLK_t: {
              glClearColor(0.0, 0.0, 0.0, 1.0);
              glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
              appState.drawMode = ((m++)%2) ? GL_FILL : GL_LINE;
              glPolygonMode(GL_FRONT_AND_BACK, appState.drawMode);
              glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
              SDL_GL_SwapWindow(window);
            } break;

            case SDLK_w: {
            } break;

            default: break;
          }
        } break;  // SDL_KEYDOWN

        case SDL_MOUSEMOTION: {
        } break;

        case SDL_MOUSEBUTTONDOWN: {
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT: {
              controls.lmbState = true;
            } break;
            case SDL_BUTTON_MIDDLE: {
              controls.mmbState = true;
            } break;
            case SDL_BUTTON_RIGHT: {
              controls.rmbState = true;
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEBUTTONUP: {
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT: {
              controls.lmbState = false;
            } break;
            case SDL_BUTTON_MIDDLE: {
              controls.mmbState = false;
            } break;
            case SDL_BUTTON_RIGHT: {
              controls.rmbState = false;
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEWHEEL: {
        } break;

        default: break;
      }  // switch (event.type)

      nk_sdl_handle_event(&event);
    }  // while(SDL_PollEvent(&event)

    nk_input_end(nkCtx);


    ////////////////////////////////////////
    //
    // Input
    //

    {
      bool32 updateView = false;

      SDL_GetMouseState((int*)&(controls.mouseX),
                        (int*)&(controls.mouseY));

      if (controls.mmbState)
      {
        camera.pos[0] += controls.mouseSensitivity * (controls.mouseX - controlsPrev.mouseX);
        camera.pos[1] += controls.mouseSensitivity * (controls.mouseY - controlsPrev.mouseY);
        updateView = true;
      }

      if (updateView)
      {
        glm_translate(view, camera.pos);
        GLint viewLoc = GetUniformLoc(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float *)view);
      }
    }

    controlsPrev = controls;

    ////////////////////////////////////////
    //
    // Timing
    //

    // TODO(mc): think which real64 can be changed to real32
    uint64 endCounter = SDL_GetPerformanceCounter();
    uint64 counterElapsed = endCounter - lastCounter;
    real64 msPerFrame = (((1000.0f * (real64)counterElapsed) /
                          (real64)perfCountFrequency));
    real64 fps = (real64)perfCountFrequency / (real64)counterElapsed;
    lastCounter = endCounter;

    uint64 endCycleCount = __rdtsc();
    uint64 cyclesElapsed = endCycleCount - lastCycleCount;
    real64 mcpf = ((real64)cyclesElapsed); // / (1000.0f * 1000.0f));
    lastCycleCount = endCycleCount;


    ////////////////////////////////////////
    //
    // Render

    SDL_GetWindowSize(window, (int*)(&(windowParameters.width)), (int*)(&(windowParameters.height)));
    glViewport(0, 0, windowParameters.width, windowParameters.height);

    glClearColor(0.0, 0.0, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(glAtom.vao);
    glBindTexture(GL_TEXTURE_2D, glAtom.texture);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glAtom.ebo);
    // The Vertex Attributes need to be set and for that VBO need to be binded.
    glBindBuffer(GL_ARRAY_BUFFER, glAtom.vbo);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "a_position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,
                          8*sizeof(float), (void*)0);

    GLint colorAttrib = glGetAttribLocation(shaderProgram, "a_color");
    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                          8*sizeof(float), (void*)(3*sizeof(float)));

    GLint uvsAttrib = glGetAttribLocation(shaderProgram, "a_texCoords");
    glEnableVertexAttribArray(uvsAttrib);
    glVertexAttribPointer(uvsAttrib, 2, GL_FLOAT, GL_FALSE,
                          8*sizeof(float), (void*)(6*sizeof(float)));

    glUseProgram(shaderProgram);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // check NK_API int nk_window_is_closed(struct nk_context*, const char*);
    if (appState.drawGUI)
    {
      static nk_flags window_flags = 0;
      window_flags = NK_WINDOW_BORDER |
                     NK_WINDOW_MOVABLE |
                     NK_WINDOW_SCALABLE |
                     NK_WINDOW_CLOSABLE |
                     NK_WINDOW_MINIMIZABLE |
                     NK_WINDOW_TITLE;
      if (nk_begin(nkCtx, "Debug", nk_rect(10, 10, 400, 200), window_flags))
      {
        nk_layout_row_dynamic(nkCtx, 20, 2);
        nk_label(nkCtx, "FPS (ms per frame)", NK_TEXT_LEFT);
        nk_labelf(nkCtx, NK_TEXT_RIGHT,
                  "%.3f (%.3f)", fps, msPerFrame);
        nk_label(nkCtx, "Mili cycles per frame", NK_TEXT_LEFT);
        nk_labelf(nkCtx, NK_TEXT_RIGHT,
                  "%.3f", mcpf);
        nk_layout_row_dynamic(nkCtx, 20, 2);
        nk_label(nkCtx, "Camera pos", NK_TEXT_LEFT);
        nk_labelf(nkCtx, NK_TEXT_RIGHT,
                  "(%.3f, %.3f, %.3f)",
                  camera.pos[0], camera.pos[1], camera.pos[2]);
        nk_layout_row_dynamic(nkCtx, 20, 2);
        nk_label(nkCtx, "Camera spherical (P,R,Y)", NK_TEXT_LEFT);
        nk_labelf(nkCtx, NK_TEXT_RIGHT,
                  "(%.3f, %.3f, %.3f)",
                  camera.pitch, camera.roll, camera.yaw);
      }
      nk_end(nkCtx);

      nk_sdl_render(NK_ANTI_ALIASING_ON, NK_MAX_VERTEX_MEMORY, NK_MAX_ELEMENT_MEMORY);
    }

    SDL_GL_SwapWindow(window);

    SDL_Delay(10);

    /*
    // TODO(mc): add sleep for the rest of fixed frame time
    if (there is more time in this frame computing) {
      sleep
    }
    else {
      drop the target framerate
    }
    */

  }   // <---- END OF running WHILE

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &glAtom.vbo);
  glDeleteBuffers(1, &glAtom.ebo);

  SDL_Quit();
  return 0;
}