#include <iostream>
#include <thread>
#include <chrono>

#include "VectorScript.h"
#include "PageManager.h"
#include "LinuxRenderer.h"
#include "MacRenderer.h"

int main() {
    PageManager pages;
    VESBridge bridge;

    // 1. Read vesconfig.json and boot the entry page
    std::string entryPage = pages.getEntryPage();
    pages.loadPage(entryPage);

    NativeRenderer display(240, 240);
    
    // Force the first frame to render immediately
    display.renderToScreen(pages.getActiveNodes());
    pages.clearDirty();

    bool isRunning = true;
    float touchX = 0, touchY = 0;
    bool isTouched = false;

    // --- THE FIRMWARE LOOP ---
    while (isRunning) {
        
        // Poll Hardware/SDL
        #ifdef __APPLE__
        isRunning = display.pollEvents(touchX, touchY, isTouched);
        #endif

        // Process Touches against the ACTIVE page
        if (isTouched) {
            auto currentNodes = pages.getActiveNodes();
            
            // Reverse loop so elements drawn "on top" are clicked first
            for (auto it = currentNodes.rbegin(); it != currentNodes.rend(); ++it) {
                if ((*it)->contains(touchX, touchY)) {
                    
                    std::string callback = (*it)->getOnClick();
                    
                    // --- THE NATIVE ROUTER ---
                    if (callback.find("route:") == 0) {
                        // Extract the filename (everything after "route:")
                        std::string targetFile = callback.substr(6);
                        pages.loadPage(targetFile);
                    } 
                    // --- STANDARD C++ BRIDGE ---
                    else if (!callback.empty()) {
                        bridge.execute(callback, { (*it)->getId() });
                    }
                    
                    break; // Stop checking after we hit the top element
                }
            }
        }

        // Render ONLY if the PageManager loaded a new file or data changed
        if (pages.isDirty()) {
            display.renderToScreen(pages.getActiveNodes());
            pages.clearDirty();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "System Shutdown." << std::endl;
    return 0;
}