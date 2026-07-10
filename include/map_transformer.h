#ifndef LV220C_MAP_TRANSFORMER_H
#define LV220C_MAP_TRANSFORMER_H

#define MAX_VERTS_PER_FACE 32
#include "v220_decoder.h"

typedef struct {
    Vector3 p1, p2, p3;
    Plane plane;

    Vector3 vertices[MAX_VERTS_PER_FACE];
    int vertex_count;
} MapFace;

typedef struct {
    MapFace *faces;
    int face_count;
} MapBrush;

typedef struct {
    Vector3 vertex;
    float angle;
} SortVertex;

typedef struct {
    float u, v;
} TexCoord;

typedef struct {
    Vector3 v0, v1, v2;
    TexCoord uv0, uv1, uv2;
} DSTriangle;

typedef struct {
    int triangle_count;
    int triangle_capacity;
    DSTriangle *triangles;
} TriangleMapData;

int compare_vertices(const void* a, const void* b);
void build_brush_geometry(MapBrush *brush);
void init_triangle_map_data(TriangleMapData *data);
void triangle_face(MapFace *face, TriangleMapData *data, Face *original_face_data);

#endif //LV220C_MAP_TRANSFORMER_H
