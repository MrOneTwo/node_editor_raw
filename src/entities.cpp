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

typedef struct Node
{
  char label[128];
  NodeID id;
  // TODO(michalc): pack into a vector?
  float x, y;
  Mesh mesh;
} Node;
