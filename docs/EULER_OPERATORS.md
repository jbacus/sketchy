# Euler Operators for Winged-Edge Topology

## Overview

This document describes the **Euler Operator** implementation in the Sketchy geometric kernel. Euler operators are atomic topological modification operations that guarantee the preservation of topological validity when building or modifying 3D solid models.

## Task: P1-T1-SUB1

**Agent**: Kernel-Agent
**Goal**: Implement Winged-Edge Data Structure with Euler Operators
**Location**: `src/kernel/winged_edge.h`, `src/kernel/winged_edge.cpp`
**Status**: ✅ **COMPLETED**

## What are Euler Operators?

Euler operators are named after the Euler-Poincaré formula for polyhedra:

```
V - E + F = 2(S - G)
```

Where:
- **V** = Number of Vertices
- **E** = Number of Edges
- **F** = Number of Faces
- **S** = Number of Solids (shells)
- **G** = Genus (number of "holes" or handles)

Each Euler operator modifies the topology while maintaining this invariant, ensuring the model remains geometrically valid.

## Implemented Euler Operators

### 1. MVSF - Make Vertex, Solid, Face

**Purpose**: Creates the initial seed for building geometry
**Effect**: V+1, S+1, F+1
**Use Case**: Starting a new model

```cpp
auto vertex = kernel.mvsf(Point3D(0, 0, 0));
```

**Creates**:
- 1 new vertex at the specified position
- 1 new face (initially unbounded)
- 1 implicit solid

**Result**: The foundation for building geometry

---

### 2. MEV - Make Edge, Vertex

**Purpose**: Extends the boundary of a face by adding a new edge and vertex
**Effect**: V+1, E+1
**Use Case**: Growing a face outline, creating open polylines

```cpp
auto edge = kernel.mev(from_vertex, Point3D(1, 0, 0), face);
auto new_vertex = edge->v2;
```

**Creates**:
- 1 new vertex at the specified coordinates
- 1 new edge connecting `from_vertex` to the new vertex
- Edge is assigned to the specified face

**Topology Update**:
- Updates winged-edge connectivity (prev/next pointers)
- Maintains face boundary loop

---

### 3. MEF - Make Edge, Face

**Purpose**: Closes a loop by connecting two vertices, splitting one face into two
**Effect**: E+1, F+1
**Use Case**: Completing polygons, subdividing faces

```cpp
auto edge = kernel.mef(vertex1, vertex2, face);
```

**Creates**:
- 1 new edge connecting the two vertices
- 1 new face (the original face is split)

**Topology Update**:
- The new edge separates the two faces
- `edge->f1` = original face
- `edge->f2` = newly created face
- Both faces have valid boundary loops

**Critical for**: Face extrusion, push/pull operations

---

### 4. KEF - Kill Edge, Face

**Purpose**: Removes an edge and merges two adjacent faces
**Effect**: E-1, F-1
**Use Case**: Inverse of MEF, simplifying geometry

```cpp
auto merged_face = kernel.kef(edge);
```

**Removes**:
- The specified edge
- One of the two adjacent faces (merged into the other)

**Returns**: Pointer to the surviving face

**Topology Update**:
- All edges that referenced the killed face are updated to reference the surviving face
- Winged-edge connectivity is rewired around the removed edge

---

### 5. KFMRH - Kill Face, Make Ring Hole

**Purpose**: Creates a hole in a face (genus modification)
**Effect**: F-1, G+1
**Use Case**: Creating holes, handles in surfaces

```cpp
kernel.kfmrh(hole_face, outer_face);
```

**Effect**:
- Removes the inner face
- Updates all edges on the hole boundary to reference the outer face
- Creates topological hole (increases genus)

---

## Winged-Edge Connectivity

Each edge stores **4 "wing" pointers** for fast topological navigation:

```cpp
struct Edge {
    std::shared_ptr<Vertex> v1, v2;         // Endpoints
    std::shared_ptr<Face> f1, f2;            // Adjacent faces

    // Wing pointers (for circular navigation)
    std::shared_ptr<Edge> p1_f1;  // Previous edge at v1 on face f1
    std::shared_ptr<Edge> n1_f1;  // Next edge at v1 on face f1
    std::shared_ptr<Edge> p2_f2;  // Previous edge at v2 on face f2
    std::shared_ptr<Edge> n2_f2;  // Next edge at v2 on face f2
};
```

### Naming Convention

- `p` = **previous** edge
- `n` = **next** edge
- `1` = relative to vertex **v1**
- `2` = relative to vertex **v2**
- `f1`/`f2` = on face **f1** or **f2**

### Why Winged-Edge?

Compared to other topological structures:

| Structure | Vertex→Edge | Edge→Face | Face→Vertices | Memory | Operations |
|-----------|-------------|-----------|---------------|---------|------------|
| **Winged-Edge** | O(1) | O(1) | O(n) | Medium | **Optimal for modeling** |
| Half-Edge | O(1) | O(1) | O(n) | High | Good for traversal |
| Face-Vertex | O(n) | O(n) | O(1) | Low | Limited operations |

**Winged-Edge excels at**:
- Edge-based operations (MEF, KEF)
- Face splitting/merging
- Extrusion and sweep operations
- Maintaining manifold properties

---

## Topology Navigation

The kernel provides efficient queries:

### Get Incident Edges
```cpp
auto edges = kernel.getIncidentEdges(vertex);
```
Returns all edges connected to a vertex (using wing pointers).

### Get Incident Faces
```cpp
auto faces = kernel.getIncidentFaces(vertex);
```
Returns all faces adjacent to a vertex.

### Get Face Boundary
```cpp
auto edges = kernel.getFaceBoundary(face);
```
Returns ordered list of edges forming the face boundary.

### Get Face Vertices
```cpp
auto vertices = kernel.getFaceVertices(face);
```
Returns ordered list of vertices on the face boundary.

---

## Validation & Manifold Checking

### Topological Validation

```cpp
bool valid = kernel.validate();
```

Checks:
- All vertex→edge references are valid
- All edge→vertex references exist in vertex list
- All face→edge references are valid
- Edge endpoints belong to the referenced vertices

### Manifold Property

```cpp
bool manifold = kernel.isManifold();
```

Verifies:
- Each edge is adjacent to exactly 2 faces (or 1 for boundary edges)
- No non-manifold vertices (vertices where the incident faces don't form a disk topology)

---

## Building Complex Geometry

### Example: Triangle

```cpp
WingedEdgeKernel kernel;

// Step 1: Create initial vertex and face
auto v1 = kernel.mvsf(Point3D(0, 0, 0));
auto face = kernel.getFaces()[0];

// Step 2: Add second vertex
auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
auto v2 = e1->v2;

// Step 3: Add third vertex
auto e2 = kernel.mev(v2, Point3D(0.5, 1, 0), face);
auto v3 = e2->v2;

// Step 4: Close the triangle (splits face)
auto e3 = kernel.mef(v3, v1, face);

// Result: 3 vertices, 3 edges, 2 faces
```

### Example: Quadrilateral

```cpp
auto v1 = kernel.mvsf(Point3D(0, 0, 0));
auto face = kernel.getFaces()[0];

auto e1 = kernel.mev(v1, Point3D(1, 0, 0), face);
auto v2 = e1->v2;

auto e2 = kernel.mev(v2, Point3D(1, 1, 0), face);
auto v3 = e2->v2;

auto e3 = kernel.mev(v3, Point3D(0, 1, 0), face);
auto v4 = e3->v2;

auto e4 = kernel.mef(v4, v1, face);

// Result: 4 vertices, 4 edges, 2 faces
```

---

## Test Coverage

### Unit Tests: 24 Tests, 100% Pass Rate

**MVSF Tests** (2 tests)
- ✅ Creates vertex and face
- ✅ Multiple calls produce unique IDs

**MEV Tests** (4 tests)
- ✅ Creates edge and vertex correctly
- ✅ Builds line strings
- ✅ Throws on null vertex
- ✅ Throws on null face

**MEF Tests** (3 tests)
- ✅ Splits face correctly
- ✅ Throws on same vertex
- ✅ Throws on null inputs

**KEF Tests** (2 tests)
- ✅ Removes edge and merges faces
- ✅ Throws on null edge

**Navigation Tests** (3 tests)
- ✅ Get incident edges
- ✅ Get face boundary
- ✅ Get face vertices

**Validation Tests** (4 tests)
- ✅ Empty kernel validates
- ✅ Validates after MVSF
- ✅ Validates after MEV
- ✅ Validates after complex operations

**Manifold Tests** (1 test)
- ✅ Manifold check for simple configurations

**ID Lookup Tests** (3 tests)
- ✅ Get vertex by ID
- ✅ Get edge by ID
- ✅ Get face by ID

**Construction Tests** (2 tests)
- ✅ Build triangle
- ✅ Build quad

**Location**: `tests/unit/test_euler_operators.cpp`

---

## Integration with Modeling Operations

### Push/Pull (Extrusion)

The Euler operators enable face extrusion:

1. **MEV** around face boundary to create offset vertices
2. **MEF** to create side faces connecting old and new vertices
3. Maintain manifold topology throughout

### Move Tool

Vertex positions can be modified while maintaining all topological connections intact (due to pointer-based structure).

### Rotate Tool

Similar to Move - geometric transformations don't affect topology.

---

## Memory Management

- **Ownership**: `WingedEdgeKernel` owns all vertices, edges, and faces via `std::shared_ptr`
- **References**: Cross-references between entities use `shared_ptr` (allows multiple references)
- **Lifetime**: Objects are automatically deallocated when the kernel is destroyed

**Why `shared_ptr` instead of `unique_ptr`?**

In Winged-Edge topology, edges, faces, and vertices all reference each other. `shared_ptr` allows:
- Multiple owners (e.g., an edge is referenced by 2 vertices and 2 faces)
- Automatic cleanup when all references are gone
- No manual memory management or dangling pointers

---

## Future Euler Operators

Additional operators to be implemented for advanced modeling:

- **KVFS** - Kill Vertex, Face, Solid (inverse of MVSF)
- **KEMR** - Kill Edge, Make Ring (creates hole in face)
- **LMEV** - Loop Make Edge Vertex (for creating handles)
- **LKFEV** - Loop Kill Face Edge Vertex

---

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| **MVSF** | O(1) | Creates single vertex/face |
| **MEV** | O(1) | Creates edge, updates pointers |
| **MEF** | O(1) | Splits face, creates edge |
| **KEF** | O(E) | Must update all edges referencing killed face |
| **Incident Edges** | O(k) | k = valence of vertex |
| **Face Boundary** | O(n) | n = edges in face |
| **Validate** | O(V + E + F) | Full topology check |

---

## References

1. Mäntylä, M. (1988). *An Introduction to Solid Modeling*. Computer Science Press.
2. Baumgart, B. (1975). "A Polyhedron Representation for Computer Vision". *AFIPS National Computer Conference*.
3. Euler, L. (1758). "Elementa doctrinae solidorum". *Novi Commentarii academiae scientiarum Petropolitanae*.

---

## Acceptance Criteria

✅ **All criteria met for P1-T1-SUB1:**

1. ✅ Winged-Edge data structure implemented with proper connectivity
2. ✅ Each edge stores pointers to 2 vertices, 2 faces, and 4 neighboring edges
3. ✅ MVSF, MEV, MEF, KEF operators implemented and functional
4. ✅ Topological validation functions (validate, isManifold)
5. ✅ Navigation queries (incident edges/faces, face boundaries)
6. ✅ 24 unit tests, 100% pass rate
7. ✅ Memory managed with C++ smart pointers
8. ✅ Demonstration program showing all operators

**Status**: ✅ **READY FOR MODELING-AGENT (Push/Pull Tool Implementation)**
