#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include <cstdint>

/*
 * Structs below are offered to the Platform Layer.
 * I'm trying to use portable types here. Nothing from base.h.
 */

typedef struct WindowParams {
  int width;
  int height;
} WindowParams;

typedef struct Memory {
  uint64_t size;
  void* memory;
} Memory;

typedef struct Input {
  bool lmbState;
  bool rmbState;
  bool mmbState;
  int mouseX;
  int mouseY;
  float mouseSensitivity;
} Input;


/*
 * Function declarations that are implemented by the platform layer.
 */

void* DEBUGReadEntireFile(char* fileName, uint64_t* size);
uint64_t DEBUGWriteEntireFile(char* fileName, void* data, uint64_t size);
void DEBUGFreeFileMemory(void* memory);

void UpdateAndRender(WindowParams* win, Memory* memory, Input* input); 

#endif