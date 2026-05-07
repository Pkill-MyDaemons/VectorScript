#pragma once
#include "AST.h"
#include <fstream>
#include <iostream>

class SVGRenderer {
private:
    std::vector<std::shared_ptr<ASTNode>> nodes;
    int canvasWidth;
    int canvasHeight;

public:
    SVGRenderer(int w = 800, int h = 600) : canvasWidth(w), canvasHeight(h) {}

    void addNode(std::shared_ptr<ASTNode> node) {
        nodes.push_back(node);
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                 << "viewBox=\"0 0 " << canvasWidth << " " << canvasHeight << "\" "
                 << "width=\"" << canvasWidth << "\" height=\"" << canvasHeight << "\">\n";
            
            // Draw all nodes (Boxes will natively draw their own children)
            for (const auto& node : nodes) {
                file << node->asSVG();
            }

            file << "</svg>";
            file.close();
            std::cout << "[Renderer] Successfully saved VectorScript UI to " << filename << std::endl;
        }
    }
};