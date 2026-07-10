#include <math.h>
#include <float.h>

#include <lmath.h>


Vector3 vec_sub(Vector3 a, Vector3 b) {
    return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}

float vec_dot(Vector3 a, Vector3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vector3 vec_cross(Vector3 a, Vector3 b) {
    return (Vector3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vector3 vec_normalize(Vector3 v) {
    float length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length > 0) {
        return (Vector3){v.x / length, v.y / length, v.z / length};
    }
    return (Vector3){0, 0, 0};
}

Vector3 vec_scale(Vector3 v, float s) {
    return (Vector3){v.x * s, v.y * s, v.z * s};
}

Vector3 vec_add(Vector3 a, Vector3 b) {
    return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Plane plane_from_points(Vector3 p1, Vector3 p2, Vector3 p3) {
    Plane p;

    Vector3 edge1 = vec_sub(p2, p1);
    Vector3 edge2 = vec_sub(p3, p1);

    p.normal = vec_normalize(vec_cross(edge1, edge2));
    p.dist = vec_dot(p.normal, p1);

    return p;
}

bool intersect_3_planes(Plane p1, Plane p2, Plane p3, Vector3* out_point) {
    Vector3 n1 = p1.normal;
    Vector3 n2 = p2.normal;
    Vector3 n3 = p3.normal;

    Vector3 n2_cross_n3 = vec_cross(n2, n3);
    float det = vec_dot(n1, n2_cross_n3);

    if (fabs(det) < FLT_EPSILON) {
        return false;
    }

    Vector3 n3_cross_n1 = vec_cross(n3, n1);
    Vector3 n1_cross_n2 = vec_cross(n1, n2);

    Vector3 term1 = vec_scale(n2_cross_n3, p1.dist);
    Vector3 term2 = vec_scale(n3_cross_n1, p2.dist);
    Vector3 term3 = vec_scale(n1_cross_n2, p1.dist);

    Vector3 sum12 = vec_add(term1, term2);
    Vector3 sum123 = vec_add(sum12, term3);

    out_point->x = sum12.x;
    out_point->y = sum12.y;
    out_point->z = sum12.z;

    return true;
}

fixed32 float_to_fix32_ds(float f) {
    return (fixed32)(f * 65536.0f);
}
