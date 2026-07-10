#include <lun_file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lmath.h>
#include <v220_decoder.h>
#include <map_transformer.h>

#define MAX_LUN_TEXTURES 256
#define MAX_LUN_VERTICES 32000
#define MAX_LUN_FACES 16000
#define MAX_LUN_ENTITIES 1024

typedef struct {
    LunTexture textures[MAX_LUN_TEXTURES];
    int texture_count;

    LunVertex vertices[MAX_LUN_VERTICES];
    int vertex_count;

    LunFace faces[MAX_LUN_FACES];
    int face_count;

    LunEntity entities[MAX_LUN_ENTITIES];
    int entity_count;
} LunBuilder;

uint16_t get_or_add_texture(LunBuilder *builder, const char* texture_name) {
    for (int i = 0; i < builder->texture_count; i++) {
        if (strcmp(builder->textures[i].name, texture_name) == 0) {
            return (uint16_t)i;
        }
    }

    if (builder->texture_count >= MAX_LUN_TEXTURES) {
        fprintf(stderr, "Error: You have reached the maximum number of textures allowed (%d).\n", MAX_LUN_TEXTURES);
        exit(-1);
    }

    int id = builder->texture_count;
    strncpy(builder->textures[id].name, texture_name, 63);
    builder->textures[id].name[63] = '\0';
    builder->texture_count++;
    return (uint16_t)id;
}

uint16_t get_or_add_vertex(LunBuilder *builder, Vector3 v) {
    fixed32 fx = float_to_fix32_ds(v.x);
    fixed32 fy = float_to_fix32_ds(v.y);
    fixed32 fz = float_to_fix32_ds(v.z);

    for (int i = 0; i < builder->vertex_count; i++) {
        if (builder->vertices[i].x == fx
            && builder->vertices[i].y == fy
            && builder->vertices[i].z == fz) {
            return (uint16_t)i;
        }
    }

    if (builder->vertex_count >= MAX_LUN_VERTICES) {
        fprintf(stderr, "Error: You have reached the maximum number of vertices allowed (%d).\n", MAX_LUN_VERTICES);
        exit(-1);
    }

    int id = builder->vertex_count;
    builder->vertices[id].x = fx;
    builder->vertices[id].y = fy;
    builder->vertices[id].z = fz;
    builder->vertex_count++;
    return (uint16_t)id;
}

void convert_entity(Entity *entity, LunEntity *dest) {
    memset(dest, 0, sizeof(LunEntity)); // Avoid garbage on unused fields

    for (int i = 0; i < entity->property_count; i++) {
        Property *prop = &entity->properties[i];

        if (strcmp(prop->key, "classname") == 0) {
            char *delimiter = strchr(prop->value, ';');

            if (delimiter != NULL) {
                dest->type = (uint8_t)atoi(delimiter + 1);
            } else {
                dest->type = (uint8_t)255;
            }
        } else if (strcmp(prop->key, "origin") == 0) {
            float ox = 0, oy = 0, oz = 0;
            sscanf(prop->value, "%f %f %f", &ox, &oy, &oz);

            dest->x = float_to_fix32_ds(ox);
            dest->y = float_to_fix32_ds(oy);
            dest->z = float_to_fix32_ds(oz);
        } else if (strcmp(prop->key, "angle") == 0) {
            dest->angle = (int16_t)atoi(prop->value);
        }
    }
}

bool export_map_to_lun(const char *filepath, Map *src_map) {
    LunBuilder builder = {0};

    for (int i = 0; i < src_map->entity_count; i++) {
        Entity *entity = &src_map->entities[i];

        if (builder.entity_count >= MAX_LUN_ENTITIES) {
            fprintf(stderr, "Error: You have reached the maximum number of entities allowed (%d).\n", MAX_LUN_ENTITIES);
            return false;
        }

        convert_entity(entity, &builder.entities[builder.entity_count]);
        builder.entity_count++;

        for (int j = 0; j < entity->brush_count; j++) {
            Brush *src_brush = &entity->brushes[j];

            MapBrush csg_brush;
            csg_brush.face_count = src_brush->face_count;
            csg_brush.faces = malloc(sizeof(MapFace) * src_brush->face_count);
            if (!csg_brush.faces) {
                fprintf(stderr, "Error: Out of memory trying to allocate brush faces.\n");
                return false;
            }

            for (int f = 0; f < src_brush->face_count; f++) {
                Face *src_face = &src_brush->faces[f];
                MapFace *dst_face = &csg_brush.faces[f];

                dst_face->p1 = src_face->p1;
                dst_face->p2 = src_face->p2;
                dst_face->p3 = src_face->p3;
            }

            build_brush_geometry(&csg_brush);

            for (int f = 0; f < csg_brush.face_count; f++) {
                MapFace *csg_face = &csg_brush.faces[f];
                Face *src_face = &src_brush->faces[f];

                if (csg_face->vertex_count < 3) {
                    continue;
                }

                TriangleMapData triangle_data;
                init_triangle_map_data(&triangle_data);

                triangle_face(csg_face, &triangle_data, src_face);

                uint16_t tex_id = get_or_add_texture(&builder, src_face->texture);

                for (int t = 0; t < triangle_data.triangle_count; t++) {
                    DSTriangle *tri = &triangle_data.triangles[t];

                    if (builder.face_count >= MAX_LUN_FACES) {
                        fprintf(stderr, "Error: You have reached the maximum number of faces allowed (%d).\n", MAX_LUN_FACES);
                        free(triangle_data.triangles);
                        free(csg_brush.faces);
                        return false;
                    }

                    LunFace *dest_face = &builder.faces[builder.face_count];
                    dest_face->texture_id = tex_id;

                    dest_face->vertex_indices[0] = get_or_add_vertex(&builder, tri->v0);
                    dest_face->vertex_indices[1] = get_or_add_vertex(&builder, tri->v1);
                    dest_face->vertex_indices[2] = get_or_add_vertex(&builder, tri->v2);

                    dest_face->u[0] = (fixed16)(tri->uv0.u * 16.0f);
                    dest_face->v[0] = (fixed16)(tri->uv0.v * 16.0f);

                    dest_face->u[1] = (fixed16)(tri->uv1.u * 16.0f);
                    dest_face->v[1] = (fixed16)(tri->uv1.v * 16.0f);

                    dest_face->u[2] = (fixed16)(tri->uv2.u * 16.0f);
                    dest_face->v[2] = (fixed16)(tri->uv2.v * 16.0f);

                    builder.face_count++;
                }

                free(triangle_data.triangles);
            }

            free(csg_brush.faces);
        }
    }

    // Write binary Lun file
    LunHeader header;
    memcpy(header.magic, LUNNA_MAGIC_BYTES, 4);
    header.version = VERSION;
    header.num_textures = (uint32_t)builder.texture_count;
    header.num_vertices = (uint32_t)builder.vertex_count;
    header.num_faces = (uint32_t)builder.face_count;
    header.num_entities = (uint32_t)builder.entity_count;

    header.offset_textures = (uint32_t)sizeof(LunHeader);
    header.offset_vertices = header.offset_textures + (header.num_textures * (uint32_t)sizeof(LunTexture));
    header.offset_faces = header.offset_vertices + (header.num_vertices * (uint32_t)sizeof(LunVertex));
    header.offset_entities = header.offset_faces + (header.num_faces * (uint32_t)sizeof(LunFace));

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing.\n", filepath);
        return false;
    }

    if (fwrite(&header, sizeof(LunHeader), 1, file) != 1) {
        fclose(file);
        return false;
    }

    if (header.num_textures > 0) {
        if (fwrite(builder.textures, sizeof(LunTexture), header.num_textures, file) != header.num_textures) {
            fclose(file);
            return false;
        }
    }

    if (header.num_vertices > 0) {
        if (fwrite(builder.vertices, sizeof(LunVertex), header.num_vertices, file) != header.num_vertices) {
            fclose(file);
            return false;
        }
    }

    if (header.num_faces > 0) {
        if (fwrite(builder.faces, sizeof(LunFace), header.num_faces, file) != header.num_faces) {
            fclose(file);
            return false;
        }
    }

    if (header.num_entities > 0) {
        if (fwrite(builder.entities, sizeof(LunEntity), header.num_entities, file) != header.num_entities) {
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}