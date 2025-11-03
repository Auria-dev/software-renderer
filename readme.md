# Software Renderer

this project is a software-based 3d renderer implemented entirely in c99. it mimics the functionality of a modern graphics api such as opengl, but executes fully on the cpu. the renderer is built around a modular architecture, focusing on clarity, flexibility, and ease of integration.

<img src="assets/showcase.gif"></img>
<br>
“The Lighthouse” by [Cotman Sam](https://sketchfab.com/cotman_sam), used under CC BY 4.0 — https://creativecommons.org/licenses/by/4.0/

## Features

* **Complete 3d rendering pipeline:** from model import to final pixel output, every stage of the pipeline is implemented in software
* **Cross-platform support:** includes a lightweight platform layer compatible with both windows (`windows.h`) and linux (`x11`)
* **Custom asset loaders:** manually written parsers for `.obj` and `.mtl` formats
* **Minimal external dependencies:** uses only platform libraries for window management and `stb_image` for texture loading
* **Optimized rasterization:**

  * perspective-correct interpolation for vertex attributes (color, uv coordinates)
  * back-face culling for performance
  * depth buffering (`z-buffer`) for proper occlusion
* **Shading and texture mapping:**

  * supports gouraud and flat shading
  * textured and non-textured rendering modes
  * bilinear and nearest-neighbor texture sampling
* **3d math library:**

  * custom implementation for vector and matrix operations
  * left-handed coordinate system
  * column-major matrix layout for transformations

## Technical Specifications

* **Language:** c99 (no external frameworks)
* **Rendering core:**

  * cpu-driven rasterizer
  * dedicated framebuffers for color and depth
* **Implemented graphics pipeline stages:**

  1. **model & view transformation:** converts object-space vertices into world and camera space
  2. **projection:** applies perspective projection to map 3d coordinates into 2d screen space
  3. **clipping:** clips primitives against the view frustum boundaries
  4. **rasterization:** converts triangles into pixel fragments
  5. **shading & texturing:** applies color interpolation or texture sampling per pixel
* **Dependencies:**

  * `stb_image.h` for texture loading
  * `windows.h` (on windows) or `x11/xlib.h` (on linux) for windowing and input handling

## Showcase

<div style="display: flex; flex-wrap: wrap; gap: 10px; justify-content: center;">
  <img src="assets/textured.gif" style="flex: 1 1 45%; max-width: 45%; height: auto;" />
  <img src="assets/materials.gif" style="flex: 1 1 45%; max-width: 45%; height: auto;" />
  <img src="assets/wireframe.gif" style="flex: 1 1 45%; max-width: 45%; height: auto;" />
  <img src="assets/normals.gif" style="flex: 1 1 45%; max-width: 45%; height: auto;" />
</div>

## Building and Running

the project includes a `makefile` for straightforward compilation.

1. **clone the repository:**

   ```bash
   git clone https://github.com/auria-dev/software-renderer.git
   cd software-renderer
   ```

2. **build the project:**

   * on linux, ensure x11 development headers are installed:

     ```bash
     # debian/ubuntu
     sudo apt-get install libx11-dev
 
     # arch
     sudo pacman -S libx11
 
     # nix
     nix-shell -p libX11
 
     # void
     sudo xbps-install -S libX11-devel
     ```
   * compile the source using:

     ```bash
     make
     ```

3. **run the application:**

   ```bash
   ./renderer
   ```

## Controls

* **w, a, s, d:** move camera forward, left, backward, and right
* **space:** move camera up
* **left shift:** move camera down
* **arrow keys:** rotate camera
* **escape:** exit the application