# Game Engine

A custom 3D game engine written in C with OpenGL rendering, featuring a modular architecture with platform abstraction.

## Features

- **OpenGL 3.3+ Renderer** - Modern graphics with shader support
- **SDL2 Windowing** - Cross-platform window management and input
- **3D Model Loading** - OBJ file support
- **Text Rendering** - TrueType font rendering with FreeType
- **Camera System** - First-person camera with mouse and keyboard controls
- **Event System** - Publisher-subscriber pattern for game events
- **Memory Management** - Custom allocator with memory tagging
- **Dynamic Arrays** - Generic resizable array implementation

## Project Structure

```
GameEngine/
├── engine/              # Engine library (libengine)
│   ├── src/
│   │   ├── core/       # Core systems (memory, events, logging)
│   │   ├── platform/   # Platform abstraction layer
│   │   ├── renderer/   # Rendering system (OpenGL backend)
│   │   ├── containers/ # Data structures
│   │   ├── models/     # 3D model loading
│   │   └── resources/  # Resource management
│   └── CMakeLists.txt
├── testbed/            # Example application
│   ├── src/
│   └── CMakeLists.txt
├── cmake/              # CMake modules
├── build/              # Build directory (generated)
└── CMakeLists.txt
```

## Requirements

### macOS
```bash
brew install sdl2 glew freetype cmake
```

### Linux (Ubuntu/Debian)
```bash
sudo apt install libsdl2-dev libglew-dev libfreetype6-dev cmake build-essential
```

### Windows
- Install [CMake](https://cmake.org/download/)
- Install [vcpkg](https://vcpkg.io/) and run:
```cmd
vcpkg install sdl2 glew freetype
```

## Building

### Quick Build (Unix-like systems)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Detailed Build Steps

1. **Create build directory**
   ```bash
   mkdir build
   cd build
   ```

2. **Configure with CMake**
   ```bash
   cmake ..
   ```
   
   For release build:
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```

3. **Build**
   ```bash
   make -j$(nproc)  # Linux/macOS
   cmake --build . --config Release  # Windows
   ```

4. **Run**
   ```bash
   cd bin
   ./testbed
   ```
   
   Or use the provided script from project root:
   ```bash
   ./run.sh
   ```

## Controls

- **WASD** - Move camera (forward/left/backward/right)
- **Q/E** - Move camera up/down
- **Mouse Drag** - Rotate camera (hold left mouse button)
- **ESC** - Quit application

## CMake Options

- `CMAKE_BUILD_TYPE` - Build type (Debug/Release)
- `CMAKE_EXPORT_COMPILE_COMMANDS` - Generate compile_commands.json (enabled by default)

## Architecture

### Engine Library (`libengine`)
The engine is compiled as a shared library providing:
- Platform abstraction for Windows, Linux, and macOS
- OpenGL rendering backend
- Event system for input and window events
- Memory management with tracking
- File I/O operations
- Clock/timing utilities

### Game Interface
Games interface with the engine through function pointers:
```c
typedef struct game {
    b8 (*initialize)(struct game* game_instance);
    b8 (*update)(struct game* game_instance, f32 delta_time);
    b8 (*render)(struct game* game_instance, f32 delta_time);
    void (*onresize)(struct game* game_instance, u32 width, u32 height);
    // ...
} game;
```

### Render Packet System
Rendering uses a command buffer approach:
- **mesh_command** - Individual mesh instances with transform
- **model_command** - Model instances with transform
- **text_command** - Text rendering commands

## Development

### IDE Support

The project generates `compile_commands.json` for IDE integration. After building:
```bash
cp build/compile_commands.json .
```

### Adding New Features

1. Add source files to appropriate `engine/src/` subdirectory
2. CMake will automatically pick them up via `GLOB_RECURSE`
3. Rebuild with `make`

### Debug Logging

The engine includes multiple log levels:
- `FATAL` - Critical errors
- `ERROR` - Errors
- `WARN` - Warnings
- `INFO` - Information
- `DEBUG` - Debug messages (disabled in Release)

## License

[Add your license here]

## Credits

Built with:
- [SDL2](https://www.libsdl.org/) - Window management and input
- [GLEW](http://glew.sourceforge.net/) - OpenGL extension loading
- [FreeType](https://freetype.org/) - Font rendering
- [stb_image](https://github.com/nothings/stb) - Image loading

