#pragma once

#ifdef __APPLE__

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <SDL.h>
#include <lunasvg.h>
#include "AST.h"

class NativeRenderer {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    int width, height;

public:
    NativeRenderer(int w = 240, int h = 240) : width(w), height(h) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return;
        }

        // Create the interactive hardware window!
        window = SDL_CreateWindow("VectorScript Firmware Simulator", 
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                  width, height, SDL_WINDOW_SHOWN);
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        
        // This texture acts exactly like our Linux Framebuffer memory!
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, 
                                    SDL_TEXTUREACCESS_STREAMING, width, height);
                                    
        std::cout << "[Mac Test Mode] SDL2 Interactive Engine Initialized." << std::endl;
    }

    ~NativeRenderer() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // NEW: SDL Event Poller (Replaces reading /dev/input/event0 on Linux)
    bool pollEvents(float& touchX, float& touchY, bool& isTouched) {
        SDL_Event e;
        isTouched = false;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return false; // User closed the window
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                touchX = static_cast<float>(e.button.x);
                touchY = static_cast<float>(e.button.y);
                isTouched = true;
            }
        }
        return true; // Keep running
    }

    void renderToScreen(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
        std::ostringstream svgString;
        svgString << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height << "\">\n";
        for (const auto& node : nodes) svgString << node->asSVG();
        svgString << "</svg>";

        // 1. Rasterize with LunaSVG
        auto document = lunasvg::Document::loadFromData(svgString.str());
        if (!document) return;
        auto bitmap = document->renderToBitmap(width, height);

        // 2. Blast the pixels to the SDL Window!
        SDL_UpdateTexture(texture, nullptr, bitmap.data(), width * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
};

#endif // __APPLE__