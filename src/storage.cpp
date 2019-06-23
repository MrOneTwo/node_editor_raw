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
  bool32 isInitialized;

  uint64 transientMemorySize;
  void* transientMemory;
  void* transientTail;

  uint64 persistentMemorySize;
  void* persistentMemory;
  void* persistentTail;
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

typedef struct Model3D {
  char* name;
  float* vertices;
  uint32* indices;
  float* normals;
  float* verticesColors;
  float* uvs;
} Model3D;

