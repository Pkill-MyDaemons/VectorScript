# VectorOS & VectorScript

A lightning-fast, C++ based UI framework and declarative language designed for raw Linux embedded systems (smartwatches, IoT) with an interactive macOS Simulator.

## Architecture
* **VectorScript (`.ves`)**: A custom, strongly-typed declarative UI language.
* **Compiler**: Hand-rolled C++ Lexer/Parser that compiles `.ves` files to an Abstract Syntax Tree (AST).
* **Render Engine**: Uses `LunaSVG` to rasterize the AST directly to raw pixels.
* **Firmware Target**: Blasts pixels directly to the Linux Framebuffer (`/dev/fb0`).
* **Simulator Target**: Seamlessly scales and maps pixels to a native `SDL2` window on macOS for UI testing.

## Building (macOS Test Mode)
Dependencies: `brew install sdl2 lunasvg`
```bash
mkdir build && cd build
cmake ..
cmake --build .
./VectorScriptFirmware 
```
## Example code
Copy the contents of ExampleVectorScript into build/
Then run 
```bash
./VectorScriptFirmware
```

## Goal
Build a lightweight frontend design language similar to Java that can be imported into a C++ file and used with minimal effort. 
