#pragma once

#ifdef __linux__
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <lunasvg.h>
#include "AST.h"

class LinuxRenderer {
private:
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int width, height;

public:
    LinuxRenderer(int w = 240, int h = 240) : width(w), height(h) {
        // 1. Open the raw Linux Framebuffer (Typical for smartwatches/IoT)
        fbfd = open("/dev/fb0", O_RDWR);
        if (fbfd == -1) {
            std::cerr << "[Warning] Could not open /dev/fb0. Running in headless test mode." << std::endl;
            return;
        }

        // 2. Get screen metadata
        ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
        ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
        screensize = vinfo.yres_virtual * finfo.line_length;

        // 3. Map the screen to memory so we can write pixels to it directly
        fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    }

    ~LinuxRenderer() {
        if (fbp) munmap(fbp, screensize);
        if (fbfd != -1) close(fbfd);
    }

    // This is the core graphics pipeline!
    void renderToScreen(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
        // 1. Build the SVG String from the AST
        std::ostringstream svgString;
        svgString << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                  << "width=\"" << width << "\" height=\"" << height << "\">\n";
        for (const auto& node : nodes) {
            svgString << node->asSVG();
        }
        svgString << "</svg>";

        // 2. LunaSVG Rasterization (SVG String -> RGBA Pixel Array)
        auto document = lunasvg::Document::loadFromData(svgString.str());
        if (!document) {
            std::cerr << "LunaSVG Failed to parse layout." << std::endl;
            return;
        }
        
        auto bitmap = document->renderToBitmap(width, height);
        const uint8_t* pixels = bitmap.data();

        // 3. Blast the pixels to the Linux Screen!
        if (fbp) {
            // Copy the RGBA pixels from LunaSVG directly into the Linux framebuffer memory
            // (Note: Some framebuffers are BGRA or RGB565, so you might need a quick pixel-swapping loop here depending on the smartwatch hardware)
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    long int location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                                        (y + vinfo.yoffset) * finfo.line_length;
                    
                    int pixelIndex = (y * width + x) * 4; // LunaSVG outputs 4 bytes per pixel (RGBA)
                    
                    // Assuming 32-bit framebuffer (BGRA usually for Linux fb)
                    *((uint32_t*)(fbp + location)) = 
                        (pixels[pixelIndex + 3] << 24) | // A
                        (pixels[pixelIndex + 0] << 16) | // R
                        (pixels[pixelIndex + 1] << 8)  | // G
                        (pixels[pixelIndex + 2]);        // B
                }
            }
        } else {
            std::cout << "[Test Mode] Successfully rasterized " << width << "x" << height << " UI frame." << std::endl;
        }
    }
};
#endif