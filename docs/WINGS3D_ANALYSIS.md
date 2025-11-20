# Wings 3D Architecture Analysis & Lessons for Sketchy

## Overview

**Wings 3D** (https://github.com/dgud/wings) is a mature subdivision surface modeler that uses the Winged-Edge data structure. Analyzing its architecture provides valuable insights for our Sketchy agentic 3D modeler.

## Wings 3D Architecture

### Technology Stack

- **Primary Language**: Erlang (94.9%)
- **Native Extensions**: C (2.6%) - performance-critical operations
- **Graphics**: OpenGL + GLSL shaders
- **UI Framework**: wxWidgets (cross-platform)

**Why Erlang?**
- Functional programming paradigm
- Excellent concurrency support
- Fault tolerance and robustness
- Hot code reloading for plugin system

### Repository Structure

```
wings/
├── src/                      # Core Erlang source
│   ├── wings_we.erl         # Winged-Edge data structure ⭐
│   ├── wings_we_build.erl   # WE construction/validation
│   ├── wings_we_util.erl    # Topology utilities
│   ├── wings_vertex.erl     # Vertex operations
│   ├── wings_edge.erl       # Edge operations
│   ├── wings_face.erl       # Face operations
│   ├── wings_extrude_face.erl
│   ├── wings_extrude_edge.erl
│   ├── wings_subdiv.erl     # Subdivision surfaces
│   └── ...
├── e3d/                      # 3D geometry primitives
├── c_src/                    # Native C extensions
├── plugins_src/              # Plugin system
└── shaders/                  # GLSL shaders
```

---

## Key Design Patterns from Wings 3D

### 1. **Element-Level Operation Modules** ⭐

Wings 3D separates operations by topology element type:

- `wings_vertex.erl` - Vertex-specific tools
- `wings_edge.erl` - Edge-specific tools
- `wings_face.erl` - Face-specific tools

**Lesson for Sketchy:**
Create separate operation namespaces in our Modeling-Agent:
```cpp
namespace sketchy::modeling {
    namespace vertex_ops { /* dissolve, connect, etc. */ }
    namespace edge_ops { /* loop, bevel, etc. */ }
    namespace face_ops { /* extrude, inset, etc. */ }
}
```

---

### 2. **Dedicated Extrusion Modules**

Wings 3D has separate files for different extrusion types:
- `wings_extrude_face.erl` - Face extrusion (Push/Pull)
- `wings_extrude_edge.erl` - Edge extrusion

**Our Implementation (P1-T3):**
We should create similar specialization:
```
src/core_kernel/modeling/
├── extrude_face.h/cpp     # Uses MEV + MEF Euler operators
├── extrude_edge.h/cpp     # Edge-based extrusion
├── inset.h/cpp            # Face inset operation
└── bevel.h/cpp            # Edge beveling
```

---

### 3. **Winged-Edge Core Structure**

Wings 3D modules:
- `wings_we.erl` - Core WE data structure
- `wings_we_build.erl` - Construction & validation
- `wings_we_util.erl` - Utility operations

**Comparison with Sketchy:**

| Feature | Wings 3D | Sketchy (Current) |
|---------|----------|-------------------|
| **Language** | Erlang | C++ |
| **WE Storage** | Functional data structures | `shared_ptr` graph |
| **Validation** | Separate build module | `validate()` method |
| **Construction** | Explicit builder | Euler operators (MVSF, MEV, MEF) |

**Sketchy Advantage:**
Our Euler operator approach provides **atomic topological modifications** that Wings 3D constructs manually. This gives us stronger guarantees of validity.

---

### 4. **Subdivision Surfaces** (Future Phase)

Wings 3D has `wings_subdiv.erl` for Catmull-Clark subdivision.

**Roadmap for Sketchy:**
- Phase 2-3: Implement subdivision operators
- Use Euler operators as foundation
- Consider OpenSubdiv library integration

---

### 5. **Plugin Architecture**

Wings 3D's `plugins_src/` directory enables extensibility without core modifications.

**Sketchy Integration (Phase 4 - API-Agent):**
```
src/api_backend/
├── plugin_api.h          # C++ plugin interface
├── python_bindings.cpp   # Pybind11 integration
└── lua_bindings.cpp      # Lua scripting
```

Expose Euler operators via scripting:
```python
# Python plugin example
kernel = sketchy.WingedEdgeKernel()
v1 = kernel.mvsf(Point3D(0, 0, 0))
face = kernel.get_faces()[0]
kernel.mev(v1, Point3D(1, 0, 0), face)
```

---

### 6. **Rendering Pipeline**

Wings 3D separates:
- `wings_draw.erl` - Drawing operations
- `wings_render.erl` - Rendering pipeline
- `wings_frame.erl` - Viewport management

**For Sketchy Graphics-Agent (P1-T2):**
```
src/graphics/
├── viewport.h/cpp        # Camera and viewport management
├── renderer.h/cpp        # OpenGL rendering pipeline
├── shader_manager.h/cpp  # GLSL shader compilation
└── mesh_renderer.h/cpp   # Render WE mesh
```

---

## Architectural Lessons Applied to Sketchy

### ✅ Already Implemented Well

1. **Modular Kernel** - Our `src/core_kernel/` and `src/kernel/` separation mirrors Wings' modular approach
2. **Euler Operators** - Superior to Wings' manual construction
3. **Testing** - 56 unit tests vs Wings' less formal testing
4. **Documentation** - Comprehensive docs in `docs/`

### 🔄 To Improve (Based on Wings 3D)

1. **Operation Organization**
   - Create element-specific operation namespaces (vertex_ops, edge_ops, face_ops)
   - Separate extrusion into dedicated modules

2. **Builder Pattern**
   - Add `MeshBuilder` class for complex geometry
   - Complement Euler operators with higher-level builders

3. **Plugin System** (Phase 4)
   - Design plugin API inspired by Wings' extensibility
   - Python/Lua bindings for Euler operators

4. **Performance Critical Paths**
   - Consider SIMD for vector operations (Wings uses C for this)
   - Profile and optimize hot paths in topology queries

---

## Specific Implementation Recommendations

### For P1-T3: Push/Pull Tool (Modeling-Agent)

Study `wings_extrude_face.erl` approach, but implement using Euler operators:

```cpp
// Recommended structure based on Wings 3D patterns
namespace sketchy::modeling::face_ops {

class FaceExtruder {
public:
    FaceExtruder(WingedEdgeKernel& kernel) : kernel_(kernel) {}

    // Main extrusion operation (inspired by Wings, implemented with Euler ops)
    std::vector<Face*> extrude(
        Face* face,
        double distance,
        const Vec3& direction
    ) {
        // 1. Get face boundary vertices
        auto vertices = kernel_.getFaceVertices(face);

        // 2. Use MEV to create offset vertices
        std::vector<Vertex*> new_vertices;
        for (auto* v : vertices) {
            Vec3 offset_pos = v->coords + direction * distance;
            // MEV creates edge from old vertex to new vertex
            auto edge = kernel_.mev(v, offset_pos, face);
            new_vertices.push_back(edge->v2);
        }

        // 3. Use MEF to create side faces
        std::vector<Face*> side_faces;
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t next = (i + 1) % vertices.size();
            // MEF connects new vertices, creating side face
            auto edge = kernel_.mef(new_vertices[i], new_vertices[next], face);
            side_faces.push_back(edge->f2);
        }

        return side_faces;
    }

private:
    WingedEdgeKernel& kernel_;
};

} // namespace sketchy::modeling::face_ops
```

---

## Performance Insights from Wings 3D

### Wings 3D Optimization Strategies

1. **C Extensions** (2.6% of codebase)
   - Used for performance-critical math operations
   - OpenGL integration

2. **Functional Data Structures**
   - Erlang's persistent data structures enable undo/redo
   - Copy-on-write for state management

### Sketchy Performance Strategy

1. **Keep C++ Core** ✅
   - Already optimal for performance-critical topology ops
   - No overhead of functional language runtime

2. **Consider**:
   - **SIMD** for vector/matrix operations (Eigen library)
   - **Spatial indexing** for large meshes (octree, BVH)
   - **Multithreading** for batch operations (OpenMP)

---

## Comparison Table: Wings 3D vs Sketchy

| Aspect | Wings 3D | Sketchy |
|--------|----------|---------|
| **Language** | Erlang + C | C++ + Python/Lua (planned) |
| **WE Implementation** | Manual construction | Euler operators ⭐ |
| **Concurrency** | Native (Erlang) | Future (C++ threads) |
| **Plugin System** | Mature | Planned (Phase 4) |
| **UI Framework** | wxWidgets | React + Three.js (web-based) |
| **Graphics** | OpenGL | OpenGL/WebGL |
| **Testing** | Informal | Comprehensive (56 tests) ⭐ |
| **Subdivision** | Built-in | Planned (Phase 2-3) |
| **Undo/Redo** | Natural (functional) | To implement |
| **Target Platform** | Desktop | Web + Desktop ⭐ |

---

## Action Items for Sketchy Development

### Immediate (P1-T3: Push/Pull Tool)

1. ✅ Study Wings' `wings_extrude_face.erl` algorithm
2. ✅ Implement using our Euler operators (MEV + MEF)
3. ✅ Create `face_ops` namespace
4. ✅ Add unit tests for extrusion

### Short-term (Phase 2)

1. Implement edge loop selection (`wings_edge_loop.erl` equivalent)
2. Add bevel operation (`wings_bevel.erl`)
3. Implement inset tool
4. Add subdivision surfaces

### Long-term (Phase 4-5)

1. Design plugin API inspired by Wings 3D
2. Add Python bindings for Euler operators
3. Implement undo/redo system
4. Add scripting console (like Wings' command line)

---

## Conclusion

**Wings 3D's 20+ years of development** provide valuable lessons:

### What Sketchy Does Better
1. ✅ **Euler operators** - More rigorous than manual construction
2. ✅ **Modern C++17** - Better performance than Erlang
3. ✅ **Comprehensive testing** - 56 automated tests
4. ✅ **Web-first architecture** - React + Three.js

### What We Should Adopt from Wings
1. 🔄 **Modular operation organization** (vertex/edge/face ops)
2. 🔄 **Dedicated extrusion modules**
3. 🔄 **Plugin architecture** (Phase 4)
4. 🔄 **Subdivision surfaces** (Phase 2-3)

### Strategic Advantage

Sketchy's **agentic development model** + **Euler operator foundation** + **web-based architecture** positions it as a **next-generation 3D modeler** that learns from Wings 3D's maturity while leveraging modern technologies.

---

## References

- Wings 3D Repository: https://github.com/dgud/wings
- Winged-Edge Paper: Baumgart, B. (1975). "A Polyhedron Representation for Computer Vision"
- Euler Operators: Mäntylä, M. (1988). "An Introduction to Solid Modeling"

**Next Step**: Implement Push/Pull tool using insights from Wings 3D's face extrusion, powered by our Euler operators.
