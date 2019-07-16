#include "node_editor.h"
#include "base.h"

#include <GL/glew.h>
#include "cglm/cglm.h"

#include "entities.cpp"


#define CUSTOM_3D_SPACE_MAX_X   1000.0f
#define CUSTOM_3D_SPACE_MAX_Y   1000.0f
#define CUSTOM_3D_SPACE_MAX_Z   1000.0f
#define CUSTOM_3D_SPACE_MIN_X  -1000.0f
#define CUSTOM_3D_SPACE_MIN_Y  -1000.0f
#define CUSTOM_3D_SPACE_MIN_Z  -1000.0f

typedef struct GLAtom {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  GLuint texture;
} GLAtom;

typedef struct Camera {
  vec3 pos;
  float yaw;
  float roll;
  float pitch;
} Camera;

global_variable GLAtom glAtom = {};
global_variable Camera camera = {};

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

void UpdateAndRender(WindowParams* win, Memory* memory, Input* input)
{
  
}