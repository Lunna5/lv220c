//
// Created by lunna on 10/7/26.
//

#ifndef LV220C_LV220C_H
#define LV220C_LV220C_H

#define MAX_LINE_LENGTH 512

#include <stdio.h>
#include <lmath.h>

typedef struct {
    float x, y, z, offset;
} UV_Axis;

typedef struct {
    Vector3 p1, p2, p3;
    char texture[64];
    UV_Axis u, v;
    float rotation, u_scale, v_scale;
} Face;

typedef struct {
    int face_count;
    int face_capacity;
    Face *faces;
} Brush;

typedef struct {
    char key[64];
    char value[256];
} Property;

typedef struct {
    int property_count;
    int property_capacity;
    Property *properties;

    int brush_count;
    int brush_capacity;
    Brush *brushes;
} Entity;

typedef struct {
    int entity_count;
    int entity_capacity;
    Entity *entities;
} Map;

void init_map(Map *map);

int add_entity(Map *map);

int add_brush(Entity *entity);

void free_map(Map *map);

void decode_map_file(FILE *map_file, Map *map);

#endif //LV220C_LV220C_H
