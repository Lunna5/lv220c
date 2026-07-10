//
// Created by lunna on 10/7/26.
//

#ifndef LV220C_LMATH_H
#define LV220C_LMATH_H
#include <stdbool.h>
#include <stdint.h>
#define EPSILON 0.001f

typedef int32_t fixed32;
typedef int16_t fixed16;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    Vector3 normal;
    float dist;
} Plane;

Vector3 vec_sub(Vector3 a, Vector3 b);
float vec_dot(Vector3 a, Vector3 b);
Vector3 vec_cross(Vector3 a, Vector3 b);
Vector3 vec_normalize(Vector3 v);
Vector3 vec_scale(Vector3 v, float s);
Vector3 vec_add(Vector3 a, Vector3 b);

Plane plane_from_points(Vector3 p1, Vector3 p2, Vector3 p3);
bool intersect_3_planes(Plane p1, Plane p2, Plane p3, Vector3* out_point);

// NDS math
fixed32 float_to_fix32_ds(float f);
#endif //LV220C_LMATH_H
