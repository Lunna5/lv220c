# Lunna Valve220 Map Compiler (lv220c)

Welcome to **lv220c** (Lunna Valve220 Map Compiler), a specialized map compilation tool built for **LunnaEngine**. 

This tool compiles standard **Valve 220 `.map`** files (the brush-based geometry format commonly used in level editors like TrenchBroom, Hammer, or Radiant) into a custom, highly optimized binary map format named **`.lun`**.

The `.lun` format is specifically tailored for resource-constrained architectures without hardware floating-point units (FPUs)—most notably the **Nintendo DS (ARM9)**. It converts standard float-based coordinate and texturing data into fixed-point representations and structures aligned for secure and efficient memory access.

---

## Features

- **Valve 220 Map Parsing:** Decodes entities, solid brushes, plane descriptions, and texture/UV mapping configurations from standard `.map` files.
- **CSG & Triangulation:** Reconstructs solid 3D brushes using plane intersections and triangulates the resulting polygonal faces.
- **Fixed-Point Conversion:** Pre-computes coordinates and texture coordinates (UVs) to target-optimized fixed-point values:
  - **3D Coordinates (`fixed32`):** Encoded in **Q15.16** fixed-point format (multiplied by `65536.0f`).
  - **UV Coordinates (`fixed16`):** Encoded in **Q11.4** fixed-point format (multiplied by `16.0f`) to fit Nintendo DS hardware texture rendering constraints.
- **ARM9 Optimization:** Pads data structures to maintain proper alignment boundary limits, preventing alignment traps and improving execution speed.
- **Unit Testing:** Built-in suite testing plane math, CSG brush rebuilding, triangulation winding, and binary serialization.

---

## Build Instructions

`lv220c` is written in standard C11 and built using CMake.

### Prerequisites

- A C11-compliant compiler (GCC, Clang, or MSVC)
- CMake 3.15 or newer
- Make or Ninja (or Visual Studio on Windows)

### Compilation Steps

1. Create a build directory and configure the project:
   ```bash
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

2. Compile the binaries:
   ```bash
   cmake --build .
   ```

3. Run the unit tests:
   ```bash
   ctest --output-on-failure
   # Or directly run:
   ./test_csg
   ```

---

## Usage

Run the compiled executable with the input map file.

```bash
lv220c [options] <map_file>
```

### Options
- `-o <output_file>`: Specify the output `.lun` file path. If not provided, it defaults to the input filename with the `.lun` extension.
- `-h`, `--help`: Show the command-line help message.

### Examples
Compile `level_01.map` into `level_01.lun`:
```bash
./lv220c maps/level_01.map
```

Compile `level_01.map` into a specific custom output path:
```bash
./lv220c -o build/maps/main_level.lun maps/level_01.map
```

---

## The `.lun` Binary File Format

The `.lun` format stores map data in five sequential sections:
1. **Header:** Contains magic bytes, version metadata, count registers, and file offsets.
2. **Textures:** Contiguous list of unique texture names.
3. **Vertices:** Unique 3D points in Q15.16 fixed-point format.
4. **Faces:** Triangles referencing the vertices with UV coordinates.
5. **Entities:** Game entity parameters, locations, and angles.

### Format Distribution & Layout Table

The following tables show exactly how data is laid out inside a `.lun` file.

#### 1. File Header (`LunHeader` — Size: 40 bytes)
Always resides at the start of the file (offset `0`).

| Byte Offset | Field Name | Data Type | Size (Bytes) | Description |
| :--- | :--- | :--- | :--- | :--- |
| `0` | `magic` | `char[4]` | 4 | Magic signature: `"LUNN"`. |
| `4` | `version` | `uint32_t` | 4 | Format version (currently `1`). |
| `8` | `num_textures` | `uint32_t` | 4 | Total number of textures in the Texture Section. |
| `12` | `num_vertices` | `uint32_t` | 4 | Total number of vertices in the Vertex Section. |
| `16` | `num_faces` | `uint32_t` | 4 | Total number of triangular faces in the Face Section. |
| `20` | `num_entities` | `uint32_t` | 4 | Total number of entities in the Entity Section. |
| `24` | `offset_textures`| `uint32_t` | 4 | Byte offset to the Texture Section. |
| `28` | `offset_vertices`| `uint32_t` | 4 | Byte offset to the Vertex Section. |
| `32` | `offset_faces` | `uint32_t` | 4 | Byte offset to the Face Section. |
| `36` | `offset_entities`| `uint32_t` | 4 | Byte offset to the Entity Section. |

---

#### 2. Texture Section (Size: `num_textures * 64` bytes)
Starts at `offset_textures`.

| Struct Member | Data Type | Size (Bytes) | Description |
| :--- | :--- | :--- | :--- |
| `name` | `char[64]` | 64 | Null-terminated texture path/name (max 63 chars + `\0`). |

---

#### 3. Vertex Section (Size: `num_vertices * 12` bytes)
Starts at `offset_vertices`. Contains 3D coordinates.

| Struct Member | Data Type | Size (Bytes) | Format | Description |
| :--- | :--- | :--- | :--- | :--- |
| `x` | `fixed32` (`int32_t`) | 4 | Q15.16 | X-coordinate (value * 65536.0). |
| `y` | `fixed32` (`int32_t`) | 4 | Q15.16 | Y-coordinate (value * 65536.0). |
| `z` | `fixed32` (`int32_t`) | 4 | Q15.16 | Z-coordinate (value * 65536.0). |

---

#### 4. Face Section (Size: `num_faces * 20` bytes)
Starts at `offset_faces`. Contains triangular face lists.

| Offset within Face | Field Name | Data Type | Size (Bytes) | Format | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0` | `vertex_indices`| `uint16_t[3]` | 6 | — | Indirection offsets targeting the Vertex array (v0, v1, v2). |
| `6` | `u` | `fixed16[3]` | 6 | Q11.4 | Horizontal texture coordinates (u0, u1, u2) (value * 16.0). |
| `12` | `v` | `fixed16[3]` | 6 | Q11.4 | Vertical texture coordinates (v0, v1, v2) (value * 16.0). |
| `18` | `texture_id` | `uint16_t` | 2 | — | Index pointing to the Texture array. |

---

#### 5. Entity Section (Size: `num_entities * 20` bytes)
Starts at `offset_entities`. Contains logical markers, spawn points, and custom triggers.

| Offset within Entity | Field Name | Data Type | Size (Bytes) | Format | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `0` | `type` | `uint8_t` | 1 | — | Numeric ID specifying entity class type. |
| `1` | `padding` | `uint8_t` | 1 | — | Safety padding byte to align 32-bit fields on ARM9 architecture. |
| `2` | `angle` | `int16_t` | 2 | — | Yaw angle / rotation direction. |
| `4` | `x` | `fixed32` (`int32_t`) | 4 | Q15.16 | Entity X position coordinate. |
| `8` | `y` | `fixed32` (`int32_t`) | 4 | Q15.16 | Entity Y position coordinate. |
| `12` | `z` | `fixed32` (`int32_t`) | 4 | Q15.16 | Entity Z position coordinate. |
| `16` | `param1` | `uint16_t` | 2 | — | Custom generic parameter 1. |
| `18` | `param2` | `uint16_t` | 2 | — | Custom generic parameter 2. |
