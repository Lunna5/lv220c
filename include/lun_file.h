//
// Created by lunna on 10/7/26.
//

#ifndef LV220C_LUN_FILE_H
#define LV220C_LUN_FILE_H

#include <stdint.h>

typedef int32_t fixed32;
typedef int16_t fixed16;

#define LUNNA_MAGIC_BYTES "LUNN"
#define VERSION 1

typedef struct {
    char magic[4];
    uint32_t version;

    uint32_t num_textures;
    uint32_t num_vertices;
    uint32_t num_faces;
    uint32_t num_entities;

    uint32_t offset_textures;
    uint32_t offset_vertices;
    uint32_t offset_faces;
    uint32_t offset_entities;
} LunHeader;

typedef struct {
    char name[64];
} LunTexture;

typedef struct {
    fixed32 x, y, z;
} LunVertex;

typedef struct {
    uint16_t vertex_indices[3];

    fixed16 u[3];
    fixed16 v[3];

    uint16_t texture_id;
} LunFace;

#include <stdbool.h>
#include <v220_decoder.h>

typedef struct {
    uint8_t type;
    uint8_t padding; // Padding for security on ARM9 architecture
    int16_t angle;
    fixed32 x, y, z;

    uint16_t param1;
    uint16_t param2;
} LunEntity;

bool export_map_to_lun(const char *filepath, Map *src_map);

#endif //LV220C_LUN_FILE_H
