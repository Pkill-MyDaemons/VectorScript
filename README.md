# VectorOS & VectorScript

A lightning-fast, C++ based UI framework and declarative language designed for raw Linux embedded systems (smartwatches, IoT) with an interactive macOS Simulator.

## Architecture
* **VectorScript (`.ves`)**: A custom, strongly-typed declarative UI language.
* **Compiler**: Hand-rolled C++ Lexer/Parser that compiles `.ves` files to an Abstract Syntax Tree (AST).
* **Render Engine**: Uses `LunaSVG` to rasterize the AST directly to raw pixels.
* **Firmware Target**: Blasts pixels directly to the Linux Framebuffer (`/dev/fb0`).
* **Simulator Target**: Seamlessly scales and maps pixels to a native `SDL2` window on macOS for UI testing.

## Features
* **Auto-Layout Stacks:** Built-in `VStack` and `HStack` mathematical alignment.
* **Reactive Data Binding:** Native state injection via standard libraries (e.g., `utils.Time`).
* **Dynamic File Routing:** Intercepts `onClick: "route:file.ves"` to hot-swap UI views without recompiling C++.
* **Zero Web Tech:** No HTML, no Chromium wrapper, no Electron overhead. Just pure C++ and Vector math.

## Building (macOS Test Mode)
Dependencies: `brew install sdl2 lunasvg`
```bash
mkdir build && cd build
cmake ..
cmake --build .
./VectorScriptFirmware 
```