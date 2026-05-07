#include <iostream>
#include <thread>
#include <chrono>

#include "VectorScript.h"
#include "AST.h"
#include "Lexer.h"
#include "Parser.h"

// The OS-Detecting Renderers
#include "LinuxRenderer.h"
#include "MacRenderer.h"

int main() {
    std::cout << "Starting VectorScript Firmware..." << std::endl;

    // A beautiful test UI for our smartwatch screen
    std::string vesCode = R"(
        Box bg = new Box(0, 0, 240, 240) { fill: "#11111B" };
        
        Text clock = new Text(0, 0, "12:00") { fill: "#A6E3A1", size: 48 };
        Box center_face = new Box(0, 0, 240, 240) { 
            align: "center", 
            child: clock 
        };
    )";

    try {
        Lexer lexer(vesCode);
        Parser parser(lexer.tokenize());
        auto astNodes = parser.parse();

        // Magic! On Mac this becomes the Quartz PNG renderer.
        // On Linux, this becomes the /dev/fb0 framebuffer renderer!
        NativeRenderer display(240, 240);
        
        // Rasterize the pixels!
        display.renderToScreen(astNodes);

    } catch (const std::exception& e) {
        std::cerr << "\n" << e.what() << "\n" << std::endl;
    }

    return 0;
}