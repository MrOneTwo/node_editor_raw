#include "node_editor.h"
#include "base.h"

#include "entities.cpp"

#include <GL/glew.h>
#include "cglm/cglm.h"

#include <cstring>

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

typedef struct MemoryBlock {
  uint64 size;
  bool32 valid;
  void* head;
  void* tail;
} MemoryBlock;

typedef struct NodesIndex {
  // TODO(michalc): should this be a MemoryBlock?
  MemoryBlock nodesMemory;
  uint32 nodesCount;
} NodesIndex;


internal void
InitMemoryBlock(Memory* memory, MemoryBlock* mb, uint64 offset, uint64 size)
{
  Assert(size < memory->size);

  mb->head = (uint8*)memory->memory + offset;
  mb->tail = mb->head;
  mb->size = size;
  mb->valid = true;
}

internal void
SaveNode(NodesIndex* ni, Node* n)
{
  memcpy((Node*)ni->nodesMemory.tail + ni->nodesCount * sizeof(Node), n, sizeof(Node));
  ni->nodesCount++;
}

internal void
AddNode(NodesIndex* ni, char* label, NodeID id, float x, float y, Mesh mesh)
{
  Node node = {};

  strncpy_s(node.label, ArrayCount(node.label), label, strlen(label));
  node.id = id;
  node.x = x;
  node.y = y;
  node.mesh = mesh;

  // TODO(michalc): what if we reached the limit? Then we have to look for a spot where ID == -1

  if (ni->nodesCount * sizeof(Node) < ni->nodesMemory.size)
  {
    SaveNode(ni, &node);
  }
}

// TODO(michalc); probably labels should be in a separate index since for better nodes finding (more performant)
internal Node*
FindNodeByLabel(NodesIndex* ni, char* label)
{
  for (Node* cursor = (Node*)ni->nodesMemory.head; cursor < ((Node*)ni->nodesMemory.head + ni->nodesCount * sizeof(Node)); cursor++)
  {
    if (strcmp(label, cursor->label) == 0)
    {
      return cursor;
    }
  }
  return NULL;
}

/*
 * Removing Node means settings the ID to -1.
 * Those Nodes won't be drawn/used.
 */
internal void
RemoveNodeByLabel(NodesIndex* ni, char* label)
{
  for (Node* cursor = (Node*)ni->nodesMemory.head; cursor < ((Node*)ni->nodesMemory.head + ni->nodesCount * sizeof(Node)); cursor++)
  {
    if (strcmp(label, cursor->label) == 0)
    {
      cursor->id = (NodeID)-1;
    }
  }
}

internal Node*
FindNodeByID()
{

}

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
  MemoryBlock nodesMemory = {};

  InitMemoryBlock(memory, &nodesMemory, 0, Megabytes(4));
}