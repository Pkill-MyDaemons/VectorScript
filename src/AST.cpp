#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>

// Base class for all VectorScript UI elements
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // Every node must know how to draw itself as an SVG string
    virtual std::string asSVG() const = 0;
};

// A specific UI element: The Box
class BoxNode : public ASTNode {
private:
    std::string id;
    float x, y, width, height;
    std::string fillColor;

public:
    BoxNode(std::string id, float x, float y, float w, float h, std::string color = "#CCCCCC")
        : id(std::move(id)), x(x), y(y), width(w), height(h), fillColor(std::move(color)) {}

    std::string asSVG() const override {
        std::ostringstream svg;
        svg << "  <rect id=\"" << id << "\" "
            << "x=\"" << x << "\" y=\"" << y << "\" "
            << "width=\"" << width << "\" height=\"" << height << "\" "
            << "fill=\"" << fillColor << "\" rx=\"8\" />\n"; // rx=8 adds rounded corners
        return svg.str();
    }
};