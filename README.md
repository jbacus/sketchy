# Sketchy: The Generative 3D Modeler

**Project Goal**: To create a next-generation, AI-driven 3D modeling application capable of translating natural language and conceptual sketches into fully editable, parametric 3D models.

## Architecture

This project is built on a **Hierarchical Orchestration Pattern** powered by Google's Agent Development Kit (ADK) and the Gemini family of models. The system comprises a **PM-Agent** for task management and specialized domain agents (Kernel, Graphics, Modeling, I/O, etc.) for code generation.

### Key Technologies

- **Orchestration**: Vertex AI Agent Builder (ADK)
- **Intelligence**: Gemini Models (via Vertex AI)
- **Core Code**: High-Performance C++ (Kernel) and Rust (Graphics/Engine)
- **Data/Memory**: Vertex AI Vector Search (Shared Knowledge Base)
- **CI/CD**: Google Cloud Build

## Development Status

- **Phase**: Phase 1: Core Geometric Kernel (Winged-Edge Implementation)
- **Current Focus**: Stabilizing agent communication protocols and the initial C++ Winged-Edge data structure implementation.

## Project Structure

```
/Sketchy
├── .agent_config/          # Agent orchestration configuration
│   ├── agent_registry.json
│   ├── task_handoff_schema.json
│   └── feedback_schema.yaml
├── src/
│   ├── core_kernel/        # C++ geometric kernel (Kernel-Agent)
│   ├── graphics/           # Rust rendering pipeline (Graphics-Agent)
│   ├── ui_frontend/        # User interface (UXUI-Agent)
│   └── api_backend/        # API services (API-Agent)
├── tests/                  # Unit and integration tests
│   ├── unit/
│   └── integration/
├── docs/                   # Architecture and design documents
├── orchestrator/           # PM-Agent orchestration scripts
└── roadmap.json            # Development roadmap and task tracking
```

## Agent System

### Core Engineering Agents
- **Kernel-Agent**: C++/Rust Performance Engineer - Core geometric kernel, Winged-Edge topology
- **Graphics-Agent**: Graphics API Expert - OpenGL/Vulkan rendering, real-time viewport
- **Modeling-Agent**: Computational Geometry Specialist - Push/Pull, extrusion, transformations

### Support Agents
- **IO-Agent**: Data serialization and file format handling (OBJ, STL, DWG)
- **UXUI-Agent**: Frontend designer for application shell and user flows
- **API-Agent**: Scripting API bridge (Python/Lua integration)
- **Cloud-Agent**: Distributed systems and cloud collaboration

### Validation Agents
- **QA-Agent**: Automated test generation and coverage validation (>95% target)
- **Sentinel-Agent**: Security and performance auditing

## Getting Started (For Human Developers)

1. **Clone this repository**:
   ```bash
   git clone https://github.com/YourOrg/Sketchy.git
   cd Sketchy
   ```

2. **Install ADK** (refer to the documentation in `docs/`)

3. **Monitor the development**:
   - `roadmap.json` - Current phase and task status
   - `tasks/` directory - Delegated agent tasks
   - `tests/` directory - Validation status

4. **Build the project**:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

## Development Roadmap

- **Phase 0**: Foundation & Setup ✅
- **Phase 1**: Core Geometric Kernel 🔄 (Current)
- **Phase 2**: Core Tools Implementation
- **Phase 3**: Interoperability & File I/O
- **Phase 4**: In-App Scripting API
- **Phase 5**: Cloud Collaboration

## License

See [LICENSE](LICENSE) file.
