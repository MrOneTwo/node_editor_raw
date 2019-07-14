/*
 * Storage module.
 *
 * The idea is to allocate the memory in advance and avoid constant malloc/free.
 *
 * The assets get read and saved in the transient memory. As a generic byte data.
 * Then the data gets interpretted and copied into the persistent memory.
 * AssetTable is an index of the assets. Asset itself stores a pointer to the
 * persistent memory where the asset lives. Asset also tells the type of the
 * asset. Using the type you can cast an address of the asset to the asset type
 * struct pointer and retrive specific parts of the asset.
 */

#define ASSETS_MAX_COUNT 128

typedef struct MemoryBlock {
  // valid as in allocated and... valid...
  bool32 valid;
  uint64 size;
  void* memory;
  // cursor points to the end of the used memory + 1. That means it can be used to
  // write into the memory.
  void* cursor;
} MemoryBlock;

typedef struct Memory {
  MemoryBlock transient;
  MemoryBlock persistent;
} Memory;

typedef struct NodesIndex {
  Node* nodesMemory;
  uint32 nodesCount;
} NodesIndex;

typedef struct AssetTable {
  uint32 assetsCount;
  Asset assets[ASSETS_MAX_COUNT];
  Memory* storageMemory;
} AssetTable;


internal void
InitMemoryBlock(MemoryBlock* mb, uint32 size)
{
  Assert(size > 0U);
  if (!mb->valid)
  {
    mb->memory = malloc(size);
    mb->size = size;
    mb->cursor = mb->memory;
    mb->valid = true;
  }
}


internal void
SaveNode(NodesIndex* ni, Node* n)
{
  memcpy(ni->nodesMemory + ni->nodesCount * sizeof(Node), n, sizeof(Node));
  ni->nodesCount++;
}


internal void
AddNode(NodesIndex* ni, char* label, NodeID id, float x, float y, Mesh mesh)
{
  Node node = {};

  strncpy(node.label, label, strlen(label));
  node.id = id;
  node.x = x;
  node.y = y;
  node.mesh = mesh;

  SaveNode(ni, &node);
}


internal Node*
FindNodeByLabel(NodesIndex* ni, char* label)
{
  for (Node* cursor = ni->nodesMemory; cursor < (ni->nodesMemory + ni->nodesCount * sizeof(Node)); cursor++)
  {
    if (strcmp(label, cursor->label) == 0)
    {
      return cursor;
    }
    else
    {
      NULL;
    }
  }
}


internal Node*
FindNodeByID()
{

}