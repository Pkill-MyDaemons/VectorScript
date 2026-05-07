#pragma once

// ONLY compile this file if we are on macOS!
#ifdef __APPLE__

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <ApplicationServices/ApplicationServices.h> // Apple Quartz
#include <lunasvg.h>
#include "AST.h"

class NativeRenderer {
private:
    int width, height;

public:
    NativeRenderer(int w = 240, int h = 240) : width(w), height(h) {
        std::cout << "[Mac Test Mode] Quartz Graphics Engine Initialized." << std::endl;
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
        
        // 2. Wrap LunaSVG pixels in a Quartz CGImage
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGContextRef context = CGBitmapContextCreate(
            (void*)bitmap.data(), width, height, 8, width * 4, colorSpace,
            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big
        );
        CGImageRef imageRef = CGBitmapContextCreateImage(context);

        // 3. Save to PNG using CoreGraphics (No external libraries needed!)
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, CFSTR("mac_test_output.png"), kCFURLPOSIXPathStyle, false);
        CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
        CGImageDestinationAddImage(destination, imageRef, NULL);
        CGImageDestinationFinalize(destination);

        // Cleanup Quartz memory
        CFRelease(destination);
        CFRelease(url);
        CGImageRelease(imageRef);
        CGContextRelease(context);
        CGColorSpaceRelease(colorSpace);

        std::cout << "[Mac Test Mode] Rendered frame to mac_test_output.png" << std::endl;
    }
};

#endif // __APPLE__