//
// Created by lunna on 10/7/26.
//
#include <lv220c.h>

void init_map(Map *map) {
    map->entity_count = 0;
    map->entity_capacity = 4;
    map->entities = malloc(sizeof(Entity) * map->entity_capacity);
}

int add_entity(Map *map) {
    if (map->entity_count >= map->entity_capacity) {
        map->entity_capacity *= 2;
        map->entities = (Entity*) realloc(map->entities, map->entity_capacity * sizeof(Entity));
    }

    Entity *entity = &map->entities[map->entity_count];
    entity->property_count = 0;
    entity->property_capacity = 4;
    entity->properties = malloc(sizeof(Property) * entity->property_capacity);

    entity->brush_count = 0;
    entity->brush_capacity = 2;
    entity->brushes = malloc(sizeof(Brush) * entity->brush_capacity);

    return map->entity_count++;
}

int add_brush(Entity *entity) {
    if (entity->brush_count >= entity->brush_capacity) {
        entity->brush_capacity *= 2;
        entity->brushes = realloc(entity->brushes, entity->brush_capacity * sizeof(Brush));
    }

    Brush *brush = &entity->brushes[entity->brush_count];
    brush->face_count = 0;
    brush->face_capacity = 6;
    brush->faces = malloc(sizeof(Face) * brush->face_capacity);

    return entity->brush_count++;
}

void free_map(Map *map) {
    for (int i = 0; i < map->entity_count; i++) {
        Entity *entity = &map->entities[i];
        for (int j = 0; j < entity->brush_count; j++) {
            free(entity->brushes[j].faces);
        }

        free(entity->brushes);
        free(entity->properties);
    }

    free(map->entities);
}

char* trim_whitespace(char* str) {
    while(isspace((unsigned char)*str)) str++;
    return str;
}

void decode_map_file(FILE *map_file, Map *map) {
    char line[MAX_LINE_LENGTH];

    enum { PARSE_GLOBAL, PARSE_ENTITY, PARSE_BRUSH } state = PARSE_GLOBAL;

    int current_entity_idx = -1;
    int current_brush_idx = -1;

    init_map(map);

    while (fgets(line, sizeof(line), map_file)) {
        char *trimmed = trim_whitespace(line);

        if (trimmed[0] == '\0' || strncmp(trimmed, "//", 2) == 0) continue;

        switch (state) {
            case PARSE_GLOBAL:
                if (trimmed[0] == '{') {
                    state = PARSE_ENTITY;
                    current_entity_idx = add_entity(map);
                }
                break;

            case PARSE_ENTITY:
                if (trimmed[0] == '}') {
                    state = PARSE_GLOBAL;
                } else if (trimmed[0] == '{') {
                    state = PARSE_BRUSH;
                    Entity *entity = &map->entities[current_entity_idx];
                    current_brush_idx = add_brush(entity);
                } else if (trimmed[0] == '"') {
                    Entity *entity = &map->entities[current_entity_idx];

                    if (entity->property_count >= entity->property_capacity) {
                        entity->property_capacity *= 2;
                        entity->properties = realloc(entity->properties, entity->property_capacity * sizeof(Property));
                    }

                    Property *property = &entity->properties[entity->property_count];

                    if (sscanf(trimmed, "\"%[^\"]\" \"%[^\"]\"", property->key, property->value) == 2) {
                        entity->property_count++;
                    }
                }
                break;
            case PARSE_BRUSH:
                if (trimmed[0] == '}') {
                    state = PARSE_ENTITY;
                } else if (trimmed[0] == '(') {
                    Entity *entity = &map->entities[current_entity_idx];
                    Brush *brush = &entity->brushes[current_brush_idx];

                    if (brush->face_count >= brush->face_capacity) {
                        brush->face_capacity *= 2;
                        brush->faces = realloc(brush->faces, brush->face_capacity * sizeof(Brush));
                    }

                    Face *f = &brush->faces[brush->face_count];

                    int parsed = sscanf(trimmed,
                        "( %f %f %f ) ( %f %f %f ) ( %f %f %f ) %63s [ %f %f %f %f ] [ %f %f %f %f ] %f %f %f",
                        &f->p1.x, &f->p1.y, &f->p1.z, &f->p2.x, &f->p2.y, &f->p2.z, &f->p3.x, &f->p3.y, &f->p3.z,
                        f->texture, &f->u.x, &f->u.y, &f->u.z, &f->u.offset,
                        &f->v.x, &f->v.y, &f->v.z, &f->v.offset, &f->rotation, &f->u_scale, &f->v_scale
                    );

                    if (parsed == 21) {
                        brush->face_count++;
                    }

                }
                break;
        }
    }
}
