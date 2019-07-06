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

typedef enum AssetType {
  ASSET_NONE,
  ASSET_MODEL3D_OBJ,
  ASSET_MODEL3D_GLTF,
  ASSET_FONT_TTF,
} AssetType;

typedef struct Asset {
  AssetType type;
  uint32 size;
  void* memory;
} Asset;

typedef struct AssetTable {
  uint32 assetsCount;
  Asset assets[ASSETS_MAX_COUNT];
  Memory* storageMemory;
} AssetTable;

typedef struct Mesh {
  char* name;

  uint8 strideVertices;
  uint8 strideIndices;
  uint8 strideNormals;
  uint8 strideColors;
  uint8 strideUVs;

  uint32 countVertices;
  uint32 countIndices;
  uint32 countNormals;
  uint32 countColors;
  uint32 countUVs;

  float* vertices;
  uint32* indices;
  float* normals;
  float* verticesColors;
  float* UVs;
} Mesh;


internal void
InitMemoryBlock(MemoryBlock* mb)
{
  Assert(mb->size > 0U);
  if (!mb->valid)
  {
    mb->memory = malloc(mb->size);
    mb->cursor = mb->memory;
    mb->valid = true;
  }
}