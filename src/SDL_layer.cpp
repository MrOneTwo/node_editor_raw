#define HAVE_M_PI  // SDL_stdinc.h has M_PI and it collides with Windows M_PI
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <GL/glew.h>

#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE

#include "base.h"
#include "node_editor.h"

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

void*
DEBUGReadEntireFile(char* fileName, uint64_t* size)
{
	Assert(fileName);

	void* result = 0;

  SDL_RWops* file = SDL_RWFromFile(fileName, "rb");
  if (file) {
	  uint64_t fileSize = SDL_RWsize(file);

	  if (fileSize > 0) {
	  	*size = fileSize;
		  void* fileData = malloc(fileSize);
		  size_t bytesRead = SDL_RWread(file, fileData, 1, fileSize);
		  if (bytesRead == fileSize) {
			  result = fileData;
		  }
		  else {
		  	// TODO(michalc): handle this
		  }
	  }
	  else {
	  	// TODO(michalc): handle this
	  }
  }
  else {
    // TODO(mc): handle this case.
  }

  if (result == 0) {
	  SDL_Log("Failed DEBUGReadEntireFile(): %s", fileName);
  }
  SDL_RWclose(file);
  return result;
}

uint64_t
DEBUGWriteEntireFile(char* fileName, void* data, uint64_t size)
{
	Assert(fileName);
	Assert(data);
	Assert(size > 0);

	size_t result = 0;

  SDL_RWops* file = SDL_RWFromFile(fileName, "wb");
  if (file) {
	  size_t bytesWritten = SDL_RWwrite(file, data, 1, size);
	  result = bytesWritten;
  }
  else {
	  SDL_Log("File %s not found", fileName);
  }

  if (result == 0) {
	  SDL_Log("Failed DEBUGWriteEntireFile(): %s", fileName);
  }
  SDL_RWclose(file);
  return (uint64_t)result;
}

void
DEBUGFreeFileMemory(void* memory)
{
	free(memory);
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

  WindowParams win = {};
  win.width = INIT_WINDOW_WIDTH;
  win.height = INIT_WINDOW_HEIGHT;
  SDL_GetWindowSize(window, &(win.width), &(win.height));

  Input inputPrev = {};
  Input input = {};
  SDL_GetMouseState(&input.mouseX, &input.mouseY);

  Memory mem = {};
  mem.size = 8 * 1024 * 1024;
  mem.memory = malloc(mem.size);

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
        	switch (event.window.event)
        	{
        		case SDL_WINDOWEVENT_RESIZED: {
	            SDL_GetWindowSize(window, &(win.width), &(win.height));
        		}
        		case SDL_WINDOWEVENT_SIZE_CHANGED: {
	            SDL_GetWindowSize(window, &(win.width), &(win.height));
        		}
        		default: break;
        	}
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
	        SDL_GetMouseState(&input.mouseX, &input.mouseY);
        } break;

        case SDL_MOUSEBUTTONDOWN: {
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT: {
              input.lmbState = true;
            } break;
            case SDL_BUTTON_MIDDLE: {
              input.mmbState = true;
            } break;
            case SDL_BUTTON_RIGHT: {
              input.rmbState = true;
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEBUTTONUP: {
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT: {
              input.lmbState = false;
            } break;
            case SDL_BUTTON_MIDDLE: {
              input.mmbState = false;
            } break;
            case SDL_BUTTON_RIGHT: {
              input.rmbState = false;
            } break;
            default: break;
          }
        } break;

        case SDL_MOUSEWHEEL: {
        } break;

        default: break;
      }  // switch (event.type)
    }  // while(SDL_PollEvent(&event)

    UpdateAndRender(&win, &mem, &input);

    SDL_GL_SwapWindow(window);
    SDL_Delay(10);

    inputPrev = input;

  } // while(true)

EXIT:;
  SDL_Quit();
  // CLEANUP
}