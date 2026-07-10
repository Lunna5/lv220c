#include <stdbool.h>
#include <math.h>
#include <lmath.h>
#include <stdlib.h>

#include <map_transformer.h>
#include <v220_decoder.h>

bool is_point_inside_brush(Vector3 point, MapBrush *brush) {
    for (int i = 0; i < brush->face_count; i++) {
        Plane *plane = &brush->faces[i].plane;

        float distance = vec_dot(plane->normal, point) - plane->dist;

        if (distance > EPSILON) {
            return false;
        }
    }

    return true;
}

void add_unique_vertex(MapFace *face, Vector3 v) {
    for (int i = 0; i < face->vertex_count; i++) {
        Vector3 existing = face->vertices[i];

        if (fabs(existing.x - v.x) < EPSILON &&
            fabs(existing.y - v.y) < EPSILON &&
            fabs(existing.z - v.z) < EPSILON) {
            return;
        }
    }

    if (face->vertex_count < MAX_VERTS_PER_FACE) {
        face->vertices[face->vertex_count++] = v;
    }
}

int compare_vertices(const void *a, const void *b) {
    float angle_a = ((SortVertex *)a)->angle;
    float angle_b = ((SortVertex *)b)->angle;

    if (angle_a < angle_b) return -1;
    if (angle_a > angle_b) return 1;
    return 0;
}

void sort_face_vertices_winding(MapFace *face) {
    if (face->vertex_count < 3) return;

    Vector3 centroid = {0, 0, 0};

    for (int i = 0; i < face->vertex_count; i++) {
        centroid = vec_add(centroid, face->vertices[i]);
    }

    centroid = vec_scale(centroid, 1.0f / face->vertex_count);

    Vector3 u_axis = vec_normalize(vec_sub(face->vertices[0], centroid));
    Vector3 v_axis = vec_cross(face->plane.normal, u_axis);

    SortVertex sort_array[MAX_VERTS_PER_FACE];

    for (int i = 0; i < face->vertex_count; i++) {
        sort_array[i].vertex = face->vertices[i];
        Vector3 dir = vec_sub(face->vertices[i], centroid);

        float u_proj = vec_dot(dir, u_axis);
        float v_proj = vec_dot(dir, v_axis);

        sort_array[i].angle = atan2f(v_proj, u_proj);
    }

    qsort(sort_array, face->vertex_count, sizeof(SortVertex), compare_vertices);

    for (int i = 0; i < face->vertex_count; i++) {
        face->vertices[i] = sort_array[i].vertex;
    }
}

void build_brush_geometry(MapBrush *brush) {
    int num = brush->face_count;

    for (int i = 0; i < num; i++) {
        MapFace *face = &brush->faces[i];
        face->plane = plane_from_points(face->p1, face->p2, face->p3);
        face->vertex_count = 0;
    }

    for (int i = 0;i < num - 2;i++) {
        for (int j = i + 1; j < num - 1; j++) {
            for (int k = j + 1; k < num; k++) {
                Vector3 intersection_point;

                Plane p1 = brush->faces[i].plane;
                Plane p2 = brush->faces[j].plane;
                Plane p3 = brush->faces[k].plane;

                if (intersect_3_planes(p1, p2, p3, &intersection_point)) {
                    if (is_point_inside_brush(intersection_point, brush)) {
                        add_unique_vertex(&brush->faces[i], intersection_point);
                        add_unique_vertex(&brush->faces[j], intersection_point);
                        add_unique_vertex(&brush->faces[k], intersection_point);
                    }
                }
            }
        }
    }

    for (int i = 0; i < brush->face_count; i++) {
        if (brush->faces[i].vertex_count >= 3) {
            sort_face_vertices_winding(&brush->faces[i]);
        } else {
            brush->faces[i].vertex_count = 0;
        }
    }
}

void init_triangle_map_data(TriangleMapData *data) {
    data->triangle_count = 0;
    data->triangle_capacity = 1024;
    data->triangles = malloc(data->triangle_capacity * sizeof(DSTriangle));
}

TexCoord calculate_uv(Vector3 vertex, Face *face) {
    TexCoord text_coord;

    Vector3 u_axis = {
        face->u.x,
        face->u.y,
        face->u.z
    };

    Vector3 v_axis = {
        face->v.x,
        face->v.y,
        face->v.z
    };

    float u_proj = vec_dot(vertex, u_axis);
    float v_proj = vec_dot(vertex, v_axis);

    text_coord.u = (u_proj / face->u_scale) + face->u.offset;
    text_coord.v = (v_proj / face->v_scale) + face->v.offset;

    return text_coord;
}

void triangle_face(MapFace *face, TriangleMapData *data, Face *original_face_data) {
    if (face->vertex_count < 3) return;

    Vector3 pivot = face->vertices[0];
    TexCoord uv_pivot = calculate_uv(pivot, original_face_data);

    for (int i = 1; i < face->vertex_count - 1; i++) {
        if (data->triangle_count >= data->triangle_capacity) {
            data->triangle_capacity *= 2;
            data->triangles = realloc(data->triangles, data->triangle_capacity * sizeof(DSTriangle));
        }

        DSTriangle *triangle = &data->triangles[data->triangle_count];

        triangle->v0 = pivot;
        triangle->v1 = face->vertices[i];
        triangle->v2 = face->vertices[i + 1];

        triangle->uv0 = uv_pivot;
        triangle->uv1 = calculate_uv(face->vertices[i], original_face_data);
        triangle->uv2 = calculate_uv(face->vertices[i + 1], original_face_data);

        data->triangle_count++;
    }
}
