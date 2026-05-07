cmake_minimum_required(VERSION 3.10)
project(VectorScriptEmbedded)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH ~/my_libs)

# Find LunaSVG (Make sure it's installed via brew on Mac, or built from source on Linux)
find_package(lunasvg REQUIRED)

include_directories(include)

add_executable(VectorScriptFirmware app/main.cpp)

# --- OS DETECTION MAGIC ---
if (APPLE)
    message(STATUS "🍎 Detected macOS: Compiling Quartz Test Mode")
    # Link Apple's Quartz/CoreGraphics framework
    target_link_libraries(VectorScriptFirmware lunasvg::lunasvg "-framework ApplicationServices")
elseif (UNIX AND NOT APPLE)
    message(STATUS "🐧 Detected Linux: Compiling Framebuffer Target")
    target_link_libraries(VectorScriptFirmware lunasvg::lunasvg)
endif()