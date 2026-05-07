cmake_minimum_required(VERSION 3.10)
project(VectorScriptEmbedded)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Give CMake hints for Homebrew (Apple Silicon & Intel)
set(CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local" "~/my_libs")

find_package(lunasvg REQUIRED)

include_directories(include)

add_executable(VectorScriptFirmware app/main.cpp)

# --- OS DETECTION MAGIC ---
if (APPLE)
    message(STATUS "🍎 Detected macOS: Compiling SDL2 Interactive Mode")
    find_package(SDL2 REQUIRED)
    target_include_directories(VectorScriptFirmware PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(VectorScriptFirmware PRIVATE lunasvg::lunasvg ${SDL2_LIBRARIES})
elseif (UNIX AND NOT APPLE)
    message(STATUS "🐧 Detected Linux: Compiling Framebuffer Target")
    target_link_libraries(VectorScriptFirmware PRIVATE lunasvg::lunasvg)
endif()