#define HAVE_M_PI  // SDL_stdinc.h has M_PI and it collides with Windows M_PI
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <GL/glew.h>

#include <stdlib.h>

#define INIT_WINDOW_WIDTH    1200
#define INIT_WINDOW_HEIGHT   900

void
GetDisplayInformation()
{
  int8_t displayCount = 0, displayIndex = 0, modeIndex = 0;
  SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
  displayCount = SDL_GetNumVideoDisplays();
  SDL_GetDisplayMode(displayIndex, modeIndex, &mode);
  SDL_Log("Display count: %d\n"
          "      Display refresh rate: %d\n",
          displayCount,
          mode.refresh_rate);
}

int
main(int argc, char *argv[])
{
  SDL_Window* window = NULL;
  SDL_GLContext glContext = {};

  // Init.
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

    glewExperimental = GL_TRUE;
    glewInit();
  }

  SDL_Event event;

  while (true) {

    // EVENTS
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT: {
          goto EXIT;
        } break;

        case SDL_WINDOWEVENT: {
        } break;

        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym)
          {
            case SDLK_ESCAPE: {
              goto EXIT;
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
            } break;
            case SDL_BUTTON_MIDDLE: {
            } break;
            case SDL_BUTTON_RIGHT: {
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEBUTTONUP: {
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT: {
            } break;
            case SDL_BUTTON_MIDDLE: {
            } break;
            case SDL_BUTTON_RIGHT: {
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEWHEEL: {
        } break;

        default: break;
      }  // switch (event.type)
    }  // while(SDL_PollEvent(&event)

    /*
     * UPDATE AND RENDER
     */

  } // while(true)

EXIT:;
  // CLEANUP
}