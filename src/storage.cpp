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

typedef struct Memory {
  MemoryBlock transient;
  MemoryBlock persistent;
} Memory;

typedef struct NodesIndex {
  // TODO(michalc): should this be a MemoryBlock?
  MemoryBlock nodesMemory;
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
  memcpy((Node*)ni->nodesMemory.memory + ni->nodesCount * sizeof(Node), n, sizeof(Node));
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
  for (Node* cursor = (Node*)ni->nodesMemory.memory; cursor < ((Node*)ni->nodesMemory.memory + ni->nodesCount * sizeof(Node)); cursor++)
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
  for (Node* cursor = (Node*)ni->nodesMemory.memory; cursor < ((Node*)ni->nodesMemory.memory + ni->nodesCount * sizeof(Node)); cursor++)
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