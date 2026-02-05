# Lab 2 - Transmittance Effects

This lab focuses on real-time transmittance using environment mapping in OpenGL.
The scene renders multiple 3D objects and lets you interactively compare reflection,
refraction, Fresnel mixing, and chromatic dispersion.

## Lab Goal

Implement and evaluate transmittance effects on mesh objects:

- Refraction through a cubemap environment.
- Reflection from the same cubemap.
- Fresnel-based blending between reflection and refraction.
- Optional RGB channel splitting for simple dispersion.

## Current Implementation

The current codebase includes:

- OpenGL 3.3 core pipeline with GLFW + GLAD + GLM + ImGui.
- A row of 6 models loaded with Assimp:
  - Ball, Skull, Teapot, Ring, Teardrop, Star.
- Two switchable skyboxes:
  - Dock (`assets/skybox/docks`)
  - Vatican (`assets/skybox/vatican`)
- Per-object material controls in ImGui:
  - Reflection toggle
  - Refraction toggle
  - Fresnel toggle and `F0` slider
  - Dispersion toggle and strength slider
  - Refractive index (`IOR`) slider
- Animated object rotation (enabled in code).

Shader flow:

- `shaders/main.vert`: outputs world-space normal and position.
- `shaders/main.frag`:
  - samples cubemap reflection via `reflect(...)`
  - samples cubemap refraction via `refract(...)`
  - applies Schlick Fresnel when enabled
  - applies simple per-channel IOR offsets for dispersion
- `shaders/skybox.vert` + `shaders/skybox.frag`: renders background cubemap.

## Controls

- `TAB`: toggle UI mode and FPS camera mode.
- `ESC`: close app.
- Camera mode:
  - `W/A/S/D`: move
  - `SPACE` / `LEFT SHIFT`: up/down
  - Mouse: look
  - Scroll: zoom

## Build and Run

Dependencies used by this project:

- CMake
- OpenGL
- GLFW
- GLM
- Assimp
- GLEW (required by `CMakeLists.txt`)

Build:

```bash
chmod +x build.sh
./build.sh
```

Run:

```bash
chmod +x run.sh
./run.sh
```

## Project Layout

```text
Lab2/
├── src/main.cpp              # App setup, rendering loop, ImGui controls
├── shaders/
│   ├── main.vert
│   ├── main.frag
│   ├── skybox.vert
│   └── skybox.frag
├── include/
│   ├── camera.h
│   ├── model.h
│   ├── shaders.h
│   └── imgui_style.h
├── assets/
│   ├── models/               # Object meshes
│   └── skybox/               # Cubemap textures
└── CMakeLists.txt
```

## Resource Tracking

### Model Links

- Ball: [add source link]
- Skull: [add source link]
- Teapot: [add source link]
- Ring: [add source link]
- Teardrop: [add source link]
- Star: [add source link]

### Skybox Links

- Dock cubemap: [add source link]
- Vatican cubemap: [add source link]

### Tutorials / References Used

- [add tutorial/reference]
- [add tutorial/reference]
- [add tutorial/reference]
