#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <map_transformer.h>
#include <v220_decoder.h>
#include <lmath.h>

#define COLOR_RESET   "\x1b[0m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_CYAN    "\x1b[36m"

// Test helper to verify if two vectors are close enough
bool vec_equals(Vector3 a, Vector3 b) {
    return fabs(a.x - b.x) < EPSILON &&
           fabs(a.y - b.y) < EPSILON &&
           fabs(a.z - b.z) < EPSILON;
}

// Test function for plane creation and intersection
void test_plane_math() {
    printf(COLOR_CYAN "Running test_plane_math()...\n" COLOR_RESET);

    // Plane 1: x = 1 (Normal: 1, 0, 0; Dist: 1)
    Vector3 p1_a = {1.0f, -1.0f, 1.0f};
    Vector3 p1_b = {1.0f, -1.0f, -1.0f};
    Vector3 p1_c = {1.0f, 1.0f, -1.0f};
    Plane plane1 = plane_from_points(p1_a, p1_b, p1_c);
    
    assert(fabs(plane1.normal.x - 1.0f) < EPSILON);
    assert(fabs(plane1.normal.y - 0.0f) < EPSILON);
    assert(fabs(plane1.normal.z - 0.0f) < EPSILON);
    assert(fabs(plane1.dist - 1.0f) < EPSILON);

    // Plane 2: y = 1 (Normal: 0, 1, 0; Dist: 1)
    Vector3 p2_a = {1.0f, 1.0f, 1.0f};
    Vector3 p2_b = {1.0f, 1.0f, -1.0f};
    Vector3 p2_c = {-1.0f, 1.0f, -1.0f};
    Plane plane2 = plane_from_points(p2_a, p2_b, p2_c);
    
    assert(fabs(plane2.normal.x - 0.0f) < EPSILON);
    assert(fabs(plane2.normal.y - 1.0f) < EPSILON);
    assert(fabs(plane2.normal.z - 0.0f) < EPSILON);
    assert(fabs(plane2.dist - 1.0f) < EPSILON);

    // Plane 3: z = 1 (Normal: 0, 0, 1; Dist: 1)
    Vector3 p3_a = {-1.0f, -1.0f, 1.0f};
    Vector3 p3_b = {1.0f, -1.0f, 1.0f};
    Vector3 p3_c = {1.0f, 1.0f, 1.0f};
    Plane plane3 = plane_from_points(p3_a, p3_b, p3_c);

    assert(fabs(plane3.normal.x - 0.0f) < EPSILON);
    assert(fabs(plane3.normal.y - 0.0f) < EPSILON);
    assert(fabs(plane3.normal.z - 1.0f) < EPSILON);
    assert(fabs(plane3.dist - 1.0f) < EPSILON);

    // Intersection of plane 1, 2, 3 should be (1, 1, 1)
    Vector3 intersection;
    bool success = intersect_3_planes(plane1, plane2, plane3, &intersection);
    assert(success);
    assert(vec_equals(intersection, (Vector3){1.0f, 1.0f, 1.0f}));

    printf(COLOR_GREEN "test_plane_math() passed!\n" COLOR_RESET);
}

// Test conversion of planes to a brush and then to triangles
void test_planes_to_triangles_conversion() {
    printf(COLOR_CYAN "Running test_planes_to_triangles_conversion()...\n" COLOR_RESET);

    MapFace faces[6];
    MapBrush brush;
    brush.faces = faces;
    brush.face_count = 6;

    // Top face (Y = 1): Normal (0, 1, 0), dist = 1
    faces[0].p1 = (Vector3){1.0f, 1.0f, 1.0f};
    faces[0].p2 = (Vector3){1.0f, 1.0f, -1.0f};
    faces[0].p3 = (Vector3){-1.0f, 1.0f, -1.0f};

    // Bottom face (Y = -1): Normal (0, -1, 0), dist = 1
    faces[1].p1 = (Vector3){-1.0f, -1.0f, -1.0f};
    faces[1].p2 = (Vector3){1.0f, -1.0f, -1.0f};
    faces[1].p3 = (Vector3){1.0f, -1.0f, 1.0f};

    // Front face (Z = 1): Normal (0, 0, 1), dist = 1
    faces[2].p1 = (Vector3){-1.0f, -1.0f, 1.0f};
    faces[2].p2 = (Vector3){1.0f, -1.0f, 1.0f};
    faces[2].p3 = (Vector3){1.0f, 1.0f, 1.0f};

    // Back face (Z = -1): Normal (0, 0, -1), dist = 1
    faces[3].p1 = (Vector3){1.0f, -1.0f, -1.0f};
    faces[3].p2 = (Vector3){-1.0f, -1.0f, -1.0f};
    faces[3].p3 = (Vector3){-1.0f, 1.0f, -1.0f};

    // Right face (X = 1): Normal (1, 0, 0), dist = 1
    faces[4].p1 = (Vector3){1.0f, -1.0f, 1.0f};
    faces[4].p2 = (Vector3){1.0f, -1.0f, -1.0f};
    faces[4].p3 = (Vector3){1.0f, 1.0f, -1.0f};

    // Left face (X = -1): Normal (-1, 0, 0), dist = 1
    faces[5].p1 = (Vector3){-1.0f, -1.0f, -1.0f};
    faces[5].p2 = (Vector3){-1.0f, -1.0f, 1.0f};
    faces[5].p3 = (Vector3){-1.0f, 1.0f, 1.0f};

    // Build the brush geometry (computes planes, finds vertices, and sorts them)
    build_brush_geometry(&brush);

    // Verify each of the 6 faces has 4 vertices
    for (int i = 0; i < 6; i++) {
        printf("Face %d vertex count: %d\n", i, faces[i].vertex_count);
        assert(faces[i].vertex_count == 4);

        // Verify all vertices lie on the face's plane
        for (int j = 0; j < 4; j++) {
            float dist_to_plane = vec_dot(faces[i].plane.normal, faces[i].vertices[j]) - faces[i].plane.dist;
            assert(fabs(dist_to_plane) < EPSILON);
        }

        // Verify winding order is counter-clockwise relative to the normal
        for (int j = 0; j < 4; j++) {
            Vector3 v0 = faces[i].vertices[j];
            Vector3 v1 = faces[i].vertices[(j + 1) % 4];
            Vector3 v2 = faces[i].vertices[(j + 2) % 4];
            Vector3 edge1 = vec_sub(v1, v0);
            Vector3 edge2 = vec_sub(v2, v1);
            Vector3 cross = vec_cross(edge1, edge2);
            float dot_val = vec_dot(cross, faces[i].plane.normal);
            assert(dot_val > -EPSILON);
        }
    }

    // Mock original Face data from decoder for UV coordinate calculation
    Face mock_faces[6];
    for (int i = 0; i < 6; i++) {
        mock_faces[i].u.x = 1.0f; mock_faces[i].u.y = 0.0f; mock_faces[i].u.z = 0.0f; mock_faces[i].u.offset = 0.0f;
        mock_faces[i].v.x = 0.0f; mock_faces[i].v.y = 1.0f; mock_faces[i].v.z = 0.0f; mock_faces[i].v.offset = 0.0f;
        mock_faces[i].u_scale = 1.0f;
        mock_faces[i].v_scale = 1.0f;
        mock_faces[i].rotation = 0.0f;
        snprintf(mock_faces[i].texture, sizeof(mock_faces[i].texture), "textures/test");
    }

    // Initialize triangle container
    TriangleMapData triangle_data;
    init_triangle_map_data(&triangle_data);

    // Triangulate each face
    for (int i = 0; i < 6; i++) {
        triangle_face(&faces[i], &triangle_data, &mock_faces[i]);
    }

    // A cube with 6 quad faces should generate exactly 12 triangles (2 per quad)
    printf("Total triangles generated: %d\n", triangle_data.triangle_count);
    assert(triangle_data.triangle_count == 12);

    // Verify correctness of UV calculations for some vertices
    // e.g. for a vertex v, u = v.x, v = v.y
    for (int i = 0; i < triangle_data.triangle_count; i++) {
        DSTriangle *tri = &triangle_data.triangles[i];
        assert(fabs(tri->uv0.u - tri->v0.x) < EPSILON);
        assert(fabs(tri->uv0.v - tri->v0.y) < EPSILON);
        assert(fabs(tri->uv1.u - tri->v1.x) < EPSILON);
        assert(fabs(tri->uv1.v - tri->v1.y) < EPSILON);
        assert(fabs(tri->uv2.u - tri->v2.x) < EPSILON);
        assert(fabs(tri->uv2.v - tri->v2.y) < EPSILON);
    }

    free(triangle_data.triangles);
    printf(COLOR_GREEN "test_planes_to_triangles_conversion() passed!\n" COLOR_RESET);
}

int main() {
    printf(COLOR_BOLD "=== Lunna Engine CSG Conversion Tests ===\n" COLOR_RESET);
    
    test_plane_math();
    test_planes_to_triangles_conversion();

    printf(COLOR_BOLD COLOR_GREEN "\nALL TESTS PASSED SUCCESSFULLY!\n" COLOR_RESET);
    return 0;
}
