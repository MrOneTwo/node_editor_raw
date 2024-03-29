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
#include <cstring>

#include "cglm/cglm.h"

#include "shaders.h"

#include "base.h"
#include "entities.cpp"
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


typedef struct Camera {
  vec3 pos;
  float yaw;
  float roll;
  float pitch;
} Camera;

typedef struct Controls {
  bool32 lmbState;
  bool32 rmbState;
  bool32 mmbState;
  uint16 mouseX;
  uint16 mouseY;
  float mouseSensitivity;
} Controls;

typedef struct State {
  bool32 drawGUI;
  GLuint drawMode;
  bool32 running;
} State;

typedef struct GLAtom {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  GLuint texture;
} GLAtom;

#define INIT_WINDOW_WIDTH    1200
#define INIT_WINDOW_HEIGHT   900

typedef struct WindowParams {
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

/*
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
*/

float vertices[] = {
  -200.000000f, -105.876236f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -185.611237f, -120.000000f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -192.805618f, -118.107780f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -198.072281f, -112.938126f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   200.000000f, -105.876236f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   185.611237f, -120.000000f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   192.805618f, -118.107780f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   198.072281f, -112.938126f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -200.000000f,  105.876236f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -185.611237f,  120.000000f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -192.805618f,  118.107780f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
  -198.072281f,  112.938126f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   200.000000f,  105.876236f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   185.611237f,  120.000000f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   192.805618f,  118.107780f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
   198.072281f,  112.938126f, 0.0f, 0.2f, 0.2f, 0.2f, 0.0f, 0.0f,
};

GLint indices[] = {
  4, 7, 3,
  0, 4, 3,
  10, 9, 14,
  12, 11, 15,
  11, 10, 14,
  3, 7, 6,
  2, 6, 1,
  6, 5, 1,
  2, 3, 6,
  11, 14, 15,
  4, 0, 12,
  9, 13, 14,
  11, 12, 8,
  12, 0, 8,
};


global_variable GLAtom glAtom = {};


void
GenerateBuffers()
{
  /*
   * VAO - combines VBOs into one object.
   */
  glGenVertexArrays(
    1,       // vertex array object names count to generate
    &glAtom.vao// array of object names
  );
  /*
   * VBO - represents a single attribute. Those aren't usually rendered
   * directly.
   */
  glGenBuffers(
    1,               // buffer object names count to generate
    &glAtom.vbo      // array of object names
  );
  /*
   * EBO
   */
  glGenBuffers(
    1,
    &glAtom.ebo
  );
  /*
   * TEXTURES
   */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenTextures(1, &glAtom.texture);
}

void
SetGeoForRendering(Mesh m)
{
  // Make this VAO the active one (remember OpenGL is a state machine).
  // You can have different VBO and VAO active. This bind has only one argument
  // because VAO can have only one role - it's more of a wrapper.
  glBindVertexArray(glAtom.vao);

  // Make this VBO the active one (remember OpenGL is a state machine).
  // You can have different VBO and VAO active. First argument specifies
  // the role of this buffer.
  glBindBuffer(GL_ARRAY_BUFFER, glAtom.vbo);
  // describe the data in the buffer. size is the total size of the buffer.
  glBufferData(GL_ARRAY_BUFFER,
               m.countVertices * sizeof(m.vertices[0]) * 8, // 8 is the number of elements: verts, colors, uvs in a line
               m.vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glAtom.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               m.countIndices * sizeof(m.indices[0]),
               (GLvoid*)(m.indices), GL_STATIC_DRAW);

  glBindTexture(GL_TEXTURE_2D, glAtom.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
               FONT_TEST_IMAGE_WIDTH, FONT_TEST_IMAGE_HEIGHT,
               0, GL_RED, GL_UNSIGNED_BYTE,
               image);
  // glGenerateMipmap(GL_TEXTURE_2D);
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

  // TODO: this memory block needs a struct that describes it.
  MemoryBlock nodesStorage = {};
  InitMemoryBlock(&nodesStorage, Megabytes(2));
  NodesIndex nodesIndex = {};
  // TODO(michalc): should it use the MemoryBlock struct? For now it only points to the raw memory
  nodesIndex.nodesMemory = nodesStorage;

  // Temporary test crap...

  Mesh node1Mesh = {};

  node1Mesh.vertices = vertices;
  node1Mesh.indices = (uint32*)indices;
  node1Mesh.strideVertices = 8*sizeof(float);
  node1Mesh.strideColors = 8*sizeof(float);
  node1Mesh.strideIndices = 8*sizeof(float);
  node1Mesh.strideUVs = 8*sizeof(float);
  node1Mesh.countVertices = 16U;
  node1Mesh.countIndices = 42U;
  node1Mesh.countColors = 16U;

  AddNode(&nodesIndex, "First node", 0, 100, 100, node1Mesh);

  Node* node1 = FindNodeByLabel(&nodesIndex, "First node");

  // ...end of temporary test crap.


  Memory mem = {};
  // TODO(michalc): need a better/cross platform malloc?
  InitMemoryBlock(&mem.transient, Megabytes(8));
  InitMemoryBlock(&mem.persistent, Megabytes(128));

  AssetTable assetTable = {};
  assetTable.storageMemory = &mem;

  if (LoadAsset("./resources/cube.obj", &assetTable, ASSET_MODEL3D_OBJ) == 0)
  {
    Mesh modelOBJ = {};
    RetriveOBJ(0, &assetTable, &modelOBJ);
  }

  char* fontPath = "./resources/fonts/OpenSans-Regular.ttf";
  LoadFont(fontPath);
  GetTextImage("All is well");


  SDL_Window* window = NULL;
  SDL_GLContext glContext = {};
  nk_context* nkCtx = NULL;

  // Setup: SDL, GLEW, Nuklear.
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
      SDL_Log("Failed to SDL_Init");
      exit(EXIT_FAILURE);
    }

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

    GetDisplayInformation();
    perfCountFrequency = SDL_GetPerformanceFrequency();

    window = SDL_CreateWindow(
      "Node editor",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI
    );

    glContext = SDL_GL_CreateContext(window);
    if (glContext == NULL) {
      SDL_Log("%s", SDL_GetError());
      exit(EXIT_FAILURE);
    }
    glViewport(0, 0, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    // Setup GLEW.
    glewExperimental = GL_TRUE;
    glewInit();  // glew looks up OpenGL functions.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Nuklear GUI init.
    nkCtx = nk_sdl_init(window);
    nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();
  }


  controls.mouseSensitivity = 1.0f;

  // model matrix holds, translation, scaling, rotation
  mat4 model = {};
  glm_mat4_identity(model);
  //glm_scale_uni(model, 0.7f);

  // view matrix represents point of view in the scene - camera
  camera.pos[0] = 0.00f;
  camera.pos[1] = 0.00f;
  camera.pos[2] = 0.30f;
  camera.yaw = (float)(M_PI/2.0f);
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


  GenerateBuffers();
  SetGeoForRendering(node1->mesh);
  // Create shader objects.
  GLuint SO_VS;
  GLuint SO_FS;
  CompileShader(VS_texture, FS_default, &SO_VS, &SO_FS);
  GLuint shaderProgram = LinkShaderObjects(SO_VS, SO_FS);

  GLint modelLoc = GetUniformLoc(shaderProgram, "model");
  GLint viewLoc = GetUniformLoc(shaderProgram, "view");
  GLint projectionLoc = GetUniformLoc(shaderProgram, "projection");

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float *)model);
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float *)view);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (float *)projection);

  GLint posAttrib = glGetAttribLocation(shaderProgram, "a_position");
  GLint colorAttrib = glGetAttribLocation(shaderProgram, "a_color");
  GLint uvsAttrib = glGetAttribLocation(shaderProgram, "a_texCoords");

  glEnableVertexAttribArray(posAttrib);
  glEnableVertexAttribArray(colorAttrib);
  glEnableVertexAttribArray(uvsAttrib);

  glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,
                        node1->mesh.strideVertices, (void*)0);
  glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                        node1->mesh.strideColors, (void*)(3*sizeof(float)));
  glVertexAttribPointer(uvsAttrib, 2, GL_FLOAT, GL_FALSE,
                        node1->mesh.strideUVs, (void*)(6*sizeof(float)));

  // Unbinding to be sure it's a clean slate before actual loop.
  // Doing that mostly to understand what's going on.
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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
  appState.drawGUI = false;
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
      vec3 delta = GLM_VEC3_ZERO_INIT;

      SDL_GetMouseState((int*)&(controls.mouseX), (int*)&(controls.mouseY));

      if (controls.mmbState)
      {
        delta[0] = controls.mouseSensitivity * (controls.mouseX - controlsPrev.mouseX);
        delta[1] = -(controls.mouseSensitivity * (controls.mouseY - controlsPrev.mouseY));
        camera.pos[0] += delta[0];
        camera.pos[1] += delta[1];
        updateView = true;
      }

      if (updateView)
      {
        glm_translate(view, delta);
        GLint viewLoc = GetUniformLoc(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float *)view);
      }

      controlsPrev = controls;
    }

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

    // TODO(michalc): the problem is that drawing the geometry has to be after drawing the GUI.
    // Otherwise the geometry rendering doesn't update. That means that the geometry overlaps the UI
    // which is less than ideal.

    SDL_GetWindowSize(window, (int*)(&(windowParameters.width)), (int*)(&(windowParameters.height)));
    glViewport(0, 0, windowParameters.width, windowParameters.height);

    glClearColor(0.0, 0.0, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // check NK_API int nk_window_is_closed(struct nk_context*, const char*);
    if (appState.drawGUI)
    {
      static nk_flags windowFlags = 0;
      windowFlags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
                    NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE |
                    NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE;
      if (nk_begin(nkCtx, "Debug", nk_rect(10, 10, 400, 200), windowFlags))
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

    SetGeoForRendering(node1->mesh);

    glBindTexture(GL_TEXTURE_2D, glAtom.texture);

    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "a_position");
    GLint colorAttrib = glGetAttribLocation(shaderProgram, "a_color");
    GLint uvsAttrib = glGetAttribLocation(shaderProgram, "a_texCoords");

    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(colorAttrib);
    glEnableVertexAttribArray(uvsAttrib);

    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,
                          node1->mesh.strideVertices, (void*)0);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                          node1->mesh.strideColors, (void*)(3*sizeof(float)));
    glVertexAttribPointer(uvsAttrib, 2, GL_FLOAT, GL_FALSE,
                          node1->mesh.strideUVs, (void*)(6*sizeof(float)));

    glDrawElements(GL_TRIANGLES, 3*14, GL_UNSIGNED_INT, 0);

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
