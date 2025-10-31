# Shader Files

This directory contains GLSL shaders for the game engine.

## Standard Shaders (3D Rendering)

### `standard_vertex.vert`
- Vertex shader for 3D meshes and models
- Handles MVP (Model-View-Projection) matrix transformations
- Passes texture coordinates and vertex colors to fragment shader

**Inputs:**
- `aPos` (vec3) - Vertex position
- `aTexCoords` (vec2) - Texture coordinates
- `aColor` (vec4) - Vertex color

**Uniforms:**
- `model` (mat4) - Model transformation matrix
- `view` (mat4) - Camera view matrix
- `projection` (mat4) - Projection matrix

### `standard_frag.frag`
- Fragment shader for 3D rendering
- Supports both textured and untextured rendering
- Multiplies texture color with vertex color

**Inputs:**
- `TexCoords` (vec2) - From vertex shader
- `Color` (vec4) - From vertex shader

**Uniforms:**
- `texture1` (sampler2D) - Texture sampler
- `useTexture` (bool) - Whether to use texture or just vertex colors

**Output:**
- `FragColor` (vec4) - Final pixel color

## Text Rendering Shaders

### `text_vertex.vert`
- Vertex shader for text rendering
- Uses orthographic projection for 2D screen space
- Simpler than standard shader (no model/view matrices needed)

**Inputs:**
- `aPos` (vec3) - Vertex position
- `aTexCoords` (vec2) - Font atlas texture coordinates
- `aColor` (vec4) - Text color

**Uniforms:**
- `projection` (mat4) - Orthographic projection matrix for 2D

### `text_frag.frag`
- Fragment shader for text rendering
- Uses font atlas texture (single channel)
- Applies text color to rendered glyphs

**Inputs:**
- `TexCoords` (vec2) - From vertex shader
- `Color` (vec4) - From vertex shader

**Uniforms:**
- `text` (sampler2D) - Font atlas texture

**Output:**
- `FragColor` (vec4) - Final text pixel color

## Notes

- All shaders use GLSL version 3.30 core
- Vertex attributes use explicit location bindings:
  - Location 0: Position (vec3)
  - Location 1: Texture Coordinates (vec2)
  - Location 2: Color (vec4)
- The text shader reads from the red channel of the font atlas texture for glyph alpha values
- Standard shader can render both textured and non-textured geometry based on the `useTexture` uniform

