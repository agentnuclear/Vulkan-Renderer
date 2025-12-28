# Vulkan Renderer - A Learning-Focused Graphics Engine

A modern Vulkan-based rendering engine designed to demonstrate best practices in graphics programming. This project showcases the progression from basic Vulkan setup to advanced rendering techniques including PBR materials, GLTF asset loading, and compute shaders.

## Table of Contents

- [What You'll Learn](#what-youll-learn)
- [Features](#features)
- [Architecture Overview](#architecture-overview)
- [Getting Started](#getting-started)
- [Project Structure](#project-structure)
- [Understanding the Rendering Pipeline](#understanding-the-rendering-pipeline)
- [Key Concepts Demonstrated](#key-concepts-demonstrated)
- [Building the Project](#building-the-project)
- [Next Steps for Learners](#next-steps-for-learners)

## What You'll Learn

This project demonstrates the journey of building a production-quality Vulkan renderer from scratch. By exploring this codebase, you'll learn:

1. **Modern Vulkan API Usage**
   - Proper initialization using VkBootstrap
   - Swapchain management and presentation
   - Synchronization primitives (fences, semaphores)
   - Command buffer recording and submission

2. **Graphics Pipeline Construction**
   - Creating graphics and compute pipelines
   - Shader compilation (GLSL to SPIR-V)
   - Descriptor sets and layouts
   - Push constants for per-draw data

3. **Memory Management**
   - Using Vulkan Memory Allocator (VMA)
   - Buffer and image allocation strategies
   - Resource lifetime management with deletion queues

4. **Asset Pipeline**
   - GLTF 2.0 file loading and parsing
   - Texture loading and mipmapping
   - Material system with PBR workflow
   - Scene graph hierarchy

5. **Advanced Rendering Techniques**
   - Physically-Based Rendering (PBR)
   - Spherical harmonics lighting
   - Bindless texture rendering
   - Compute shader effects

## Features

### Core Rendering
- **Vulkan 1.x** - Modern low-level graphics API
- **Double-buffered rendering** - Smooth frame pacing with FRAME_OVERLAP=2
- **Deferred command recording** - Efficient multi-threaded rendering capability
- **ImGui integration** - Real-time debug statistics and controls

### Material System
- **PBR Materials** - Metallic/roughness workflow
- **Material Passes** - Opaque and transparent rendering
- **Material Instances** - Per-object material variations
- **Bindless Textures** - Dynamic texture arrays for efficiency

### Asset Loading
- **GLTF 2.0 Support** - Industry-standard 3D asset format
- **Embedded & External Textures** - Flexible texture loading
- **Hierarchical Scenes** - Node-based scene graph with transforms
- **Automatic Bounds Calculation** - For frustum culling

### Camera System
- **Free-look Camera** - Pitch/yaw rotation with mouse
- **WASD Movement** - Velocity-based camera controls
- **SDL2 Input Handling** - Cross-platform input processing

### Compute Shaders
- **Procedural Sky** - Star field generation with noise
- **Background Effects** - Runtime-switchable compute shaders
- **GPU-driven Rendering** - Offload work to compute pipelines

## Architecture Overview

The engine follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────┐
│           VulkanEngine (Core)               │
│  - Initialization & Lifecycle Management    │
│  - Main Render Loop                         │
└─────────────────────────────────────────────┘
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
┌──────────┐ ┌──────────┐ ┌──────────┐
│ Pipeline │ │  Images  │ │ Loader   │
│ Builder  │ │ & Memory │ │ (GLTF)   │
└──────────┘ └──────────┘ └──────────┘
        │           │           │
        └───────────┼───────────┘
                    ▼
        ┌────────────────────────┐
        │    Descriptor System    │
        │  - Allocator Pools      │
        │  - Layout Cache         │
        └────────────────────────┘
                    │
                    ▼
        ┌────────────────────────┐
        │    Scene Graph          │
        │  - Nodes & Transforms   │
        │  - Renderables          │
        │  - Draw Context         │
        └────────────────────────┘
```

### Key Components

| Component | Purpose | Location |
|-----------|---------|----------|
| **VulkanEngine** | Main engine orchestrator | `src/vk_engine.h/cpp` |
| **VkTypes** | Core type definitions | `src/vk_types.h` |
| **VkInitializers** | Vulkan structure helpers | `src/vk_initializers.h/cpp` |
| **VkDescriptors** | Descriptor management | `src/vk_descriptors.h/cpp` |
| **VkPipelines** | Pipeline creation utilities | `src/vk_pipelines.h/cpp` |
| **VkImages** | Image operations | `src/vk_images.h/cpp` |
| **VkLoader** | GLTF asset loading | `src/vk_loader.h/cpp` |
| **Camera** | Camera controls | `src/camera.h/cpp` |

## Getting Started

### Prerequisites

- **C++20 compatible compiler** (GCC 10+, Clang 11+, MSVC 2019+)
- **CMake 3.8+**
- **Vulkan SDK** - Download from [LunarG](https://vulkan.lunarg.com/)
- **SDL2** - Cross-platform window/input library

### Quick Start

1. **Clone the repository**
   ```bash
   git clone <your-repo-url>
   cd Vulkan-Renderer
   ```

2. **Build the project**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. **Run the engine**
   ```bash
   ./bin/engine
   ```

### Controls

- **Mouse** - Look around (pitch/yaw rotation)
- **W/A/S/D** - Move camera forward/left/backward/right
- **Shift** - Move down
- **Space** - Move up
- **ESC** - Exit application

## Project Structure

```
Vulkan-Renderer/
├── src/                        # Core engine source code
│   ├── vk_engine.cpp          # Main engine implementation
│   ├── vk_types.h             # Type definitions
│   ├── vk_initializers.cpp    # Vulkan helpers
│   ├── vk_descriptors.cpp     # Descriptor allocator
│   ├── vk_pipelines.cpp       # Pipeline builder
│   ├── vk_images.cpp          # Image utilities
│   ├── vk_loader.cpp          # GLTF loader
│   ├── camera.cpp             # Camera system
│   └── main.cpp               # Entry point
│
├── shaders/                    # GLSL shader source
│   ├── mesh.vert              # Main vertex shader
│   ├── mesh_pbr.frag          # PBR fragment shader
│   ├── sky.comp               # Procedural sky compute
│   └── ...                    # Additional shaders
│
├── assets/                     # 3D models and textures
│   ├── basicmesh.glb          # Simple test mesh
│   └── structure.glb          # Complex scene
│
├── third_party/               # External dependencies
│   ├── vkbootstrap/           # Vulkan initialization
│   ├── vma/                   # Memory allocator
│   ├── glm/                   # Math library
│   ├── imgui/                 # Debug UI
│   └── fastgltf/              # GLTF parser
│
└── CMakeLists.txt             # Build configuration
```

## Understanding the Rendering Pipeline

### Frame Lifecycle

The engine uses a double-buffered rendering approach:

```
Frame N-1 Rendering on GPU
         │
         ├─→ CPU waits for Frame N-1 fence
         │
         ├─→ Acquire swapchain image for Frame N
         │
         ├─→ Record commands for Frame N
         │   ├── Update scene (transforms, camera)
         │   ├── Draw background (compute shader)
         │   ├── Draw geometry (mesh rendering)
         │   └── Draw UI (ImGui)
         │
         ├─→ Submit Frame N to GPU
         │
         └─→ Present Frame N to screen

Frame N Rendering on GPU (loop continues...)
```

### Render Pass Structure

**1. Background Pass** (`draw_background()`)
- Compute shader execution
- Writes to swapchain image
- Procedural sky/effects generation

**2. Geometry Pass** (`draw_geometry()`)
- Material sorting and batching
- Opaque rendering first (depth testing)
- Transparent rendering second (alpha blending)
- Push constants for per-draw transforms

**3. UI Pass** (`draw_imgui()`)
- ImGui rendering for debug info
- Statistics display (FPS, frame time)

### Synchronization Model

```
CPU Timeline:          GPU Timeline:
    │                      │
    ├─ Wait on Fence ─────►├─ Frame N-1 Complete
    │                      │
    ├─ Acquire Image       │
    │                      │
    ├─ Record Commands     │
    │                      │
    ├─ Submit ────────────►├─ Execute Commands
    │                      │  ├─ Wait: swapImageSemaphore
    │                      │  ├─ Render
    │                      │  └─ Signal: renderSemaphore
    │                      │
    ├─ Present ───────────►├─ Wait: renderSemaphore
    │                      │  └─ Display Frame
    │                      │
    └─ Loop                └─ Signal: Fence
```

## Key Concepts Demonstrated

### 1. Vulkan Initialization (src/vk_engine.cpp:142)

The engine uses **VkBootstrap** to simplify the complex Vulkan initialization:

```cpp
// Instance creation with validation layers
vkb::Instance vkb_inst = vkb::InstanceBuilder()
    .set_app_name("Vulkan Renderer")
    .require_api_version(1, 3, 0)
    .use_default_debug_messenger()
    .build();
```

**Learning Point:** VkBootstrap abstracts away ~500 lines of boilerplate while maintaining full control.

### 2. Descriptor Management (src/vk_descriptors.cpp)

A custom descriptor allocator that handles automatic pool growth:

```cpp
class DescriptorAllocator {
    std::vector<VkDescriptorPool> pools;
    VkDescriptorPool get_pool();  // Automatic pool allocation
    void clear_pools();           // Frame-based reset
};
```

**Learning Point:** Descriptor pool exhaustion is handled gracefully without manual pool sizing.

### 3. Push Constants (src/vk_engine.cpp:891)

Efficient per-draw data without descriptor set overhead:

```cpp
GPUDrawPushConstants pushConstants;
pushConstants.worldMatrix = draw.transform;
pushConstants.vertexBuffer = draw.indexBuffer;

vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT,
                   0, sizeof(GPUDrawPushConstants), &pushConstants);
```

**Learning Point:** Push constants provide ~128 bytes of fast data for per-draw variations.

### 4. Material System (src/vk_types.h:124)

Bindless rendering with material instances:

```cpp
struct MaterialInstance {
    MaterialPipeline* pipeline;
    VkDescriptorSet materialSet;
    MaterialPass passType;
};
```

**Learning Point:** Separating pipeline from material data enables efficient batching.

### 5. Scene Graph (src/vk_types.h:158)

Hierarchical transform system:

```cpp
struct Node {
    std::weak_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;
    glm::mat4 localTransform;
    glm::mat4 worldTransform;

    void refreshTransform(const glm::mat4& parentMatrix);
};
```

**Learning Point:** Scene graphs enable complex hierarchies (e.g., character with equipment).

### 6. Immediate Submit Pattern (src/vk_engine.cpp:568)

One-time command execution for uploads:

```cpp
void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer)>&& function) {
    VkCommandBuffer cmd = _immCommandBuffer;
    // Begin recording
    function(cmd);  // User code executes
    // Submit and wait
}
```

**Learning Point:** Simplifies texture uploads and buffer initialization.

### 7. GLTF Loading (src/vk_loader.cpp)

Complete asset pipeline from disk to GPU:

```cpp
std::optional<LoadedGLTF> loadGltf(VulkanEngine* engine, std::string_view filePath) {
    // Parse GLTF file
    // Upload vertex/index buffers
    // Load textures with mipmapping
    // Create materials
    // Build scene hierarchy
}
```

**Learning Point:** Real-world asset loading requires handling multiple data formats and GPU uploads.

## Building the Project

### CMake Configuration

The build system automatically handles:

- **Shader Compilation**: GLSL → SPIR-V using `glslangValidator`
- **Dependency Management**: Third-party libraries linked automatically
- **Output Organization**: Binaries and DLLs in `bin/` directory

### Build Targets

```bash
# Debug build with validation layers
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Shader Compilation

Shaders are compiled automatically during build:

```cmake
# Finds all .vert, .frag, .comp files in shaders/
# Compiles to .spv using glslangValidator
# Output: shaders/mesh.vert.spv, etc.
```

**Manual compilation:**
```bash
glslangValidator -V shaders/mesh.vert -o shaders/mesh.vert.spv
```

## Next Steps for Learners

### Beginner Level

1. **Explore the main loop** (src/main.cpp:6)
   - Understand SDL initialization
   - Trace through `VulkanEngine::run()`

2. **Study shader code** (shaders/)
   - Start with `colored_triangle.frag`
   - Progress to `mesh_pbr.frag`

3. **Modify background effects** (shaders/sky.comp:15)
   - Change colors and patterns
   - Add new procedural effects

### Intermediate Level

1. **Add new material types**
   - Create a toon shader
   - Implement Blinn-Phong lighting

2. **Extend the GLTF loader** (src/vk_loader.cpp)
   - Add animation support
   - Load morph targets

3. **Implement frustum culling**
   - Use existing bounds calculation
   - Skip off-screen objects

### Advanced Level

1. **Add shadow mapping**
   - Create depth render pass
   - Implement shadow cascades

2. **Implement deferred rendering**
   - G-buffer generation
   - Light accumulation pass

3. **Multi-threaded command recording**
   - Leverage FRAME_OVERLAP
   - Thread-safe descriptor allocation

### Debugging Tips

**Enable Validation Layers:**
- Already enabled in Debug builds
- Check console for Vulkan errors

**Use RenderDoc:**
```bash
# Capture frames for GPU debugging
renderdoc ./bin/engine
```

**ImGui Statistics:**
- FPS and frame time displayed in-window
- Add custom metrics in `VulkanEngine::draw_imgui()`

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Vulkan SDK | 1.x | Graphics API |
| SDL2 | 2.x | Windowing and input |
| VkBootstrap | Latest | Vulkan initialization |
| VMA | 3.x | Memory allocation |
| GLM | 0.9.9+ | Math library |
| fastgltf | Latest | GLTF parsing |
| ImGui | 1.x | Debug UI |
| stb_image | Latest | Image loading |
| fmt | 9.x+ | String formatting |

## Resources for Learning

- **Vulkan Tutorial**: https://vulkan-tutorial.com/
- **Vulkan Guide**: https://vkguide.dev/
- **Khronos Vulkan Samples**: https://github.com/KhronosGroup/Vulkan-Samples
- **GLTF Specification**: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
- **Learn OpenGL (Concepts)**: https://learnopengl.com/

## License

See LICENSE file for details.

## Contributing

This is a learning project, but contributions are welcome! Consider:

- Adding more example shaders
- Implementing additional rendering techniques
- Improving documentation and comments
- Creating tutorials for specific features

---

**Happy Learning!** This renderer demonstrates production-quality Vulkan code while remaining accessible to learners. Start simple, experiment often, and don't hesitate to break things - that's how we learn.