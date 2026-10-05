# Lightweight 3D Viewer

A high-performance, cross-platform 3D model viewer built with C++ and [Sokol](https://github.com/floooh/sokol).

This project demonstrates how to build a unified codebase that compiles to native graphics APIs across different operating systems without using a heavy game engine. It features an interactive orbit camera, OBJ loading, and a custom shader pipeline.

## Features
- **Cross-Platform Native Rendering:**
  - Windows: **Direct3D 11**
  - macOS: **Metal**
  - Linux: **OpenGL Core**
- **Interactive Orbit Camera:** Mouse drag to rotate, scroll wheel to zoom.
- **OBJ Model Loading:** Parses standard `.obj` files using `tinyobjloader`.
- **Shader Pipeline:** Custom GLSL shaders cross-compiled to HLSL/Metal using `sokol-shdc`.

## Dependencies
This project uses lightweight, header-only or single-folder C/C++ libraries. All dependencies are included in the `third_party` folder:
* **Sokol:** Core graphics and app framework.
* **cglm:** Highly optimized math library for 3D graphics.
* **tinyobjloader:** Single-header OBJ parser.

## Build Instructions

You will need [CMake](https://cmake.org/download/) (3.12+) and a C++20 compatible compiler (MSVC, Clang, or GCC).

### Windows (Visual Studio)
1. Clone the repository.
2. Open the project folder in Visual Studio or run from the Developer Command Prompt:
```bash
mkdir build
cd build
cmake ..
cmake --build .

```
macOS
macOS requires the Apple Clang compiler (included with Xcode). The CMake file automatically compiles the Sokol implementation as Objective-C++ to enable Metal support.
```bash
mkdir build
cd build
cmake ..
cmake --build .

```
Linux
Ensure you have the required X11 and OpenGL development headers installed (`libx11-dev`, `libxi-dev`, `libxcursor-dev`, `libgl1-mesa-dev`).
```bash
mkdir build
cd build
cmake ..
cmake --build .

```
Running the App
Make sure `model.obj` is located in your project root. CMake is configured to automatically copy this file to the build directory alongside the executable during compilation. Simply run the generated `3DViewer` executable!
