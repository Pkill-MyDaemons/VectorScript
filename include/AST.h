#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>

// 1. Style Structure
struct VESStyle {
    std::string fill = "none";
    int fontSize = 16;
    std::string onClick = ""; 
    std::string align = "left"; 
};

inline std::string sanitizeXML(const std::string& input) {
    std::string output;
    output.reserve(input.length()); // Optimize memory allocation
    for (char c : input) {
        switch (c) {
            case '&':  output += "&amp;"; break;
            case '<':  output += "&lt;"; break;
            case '>':  output += "&gt;"; break;
            case '"':  output += "&quot;"; break;
            case '\'': output += "&apos;"; break;
            default:   output += c; break;
        }
    }
    return output;
}

// 2. Base Node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string asSVG() const = 0;
    virtual bool contains(float px, float py) const = 0;
    virtual std::string getOnClick() const = 0;
    virtual std::string getId() const = 0;
};

// 3. Text Element (Moved up so Box can see it)
// 3. Text Element
class TextNode : public ASTNode {
private:
    std::string id;
    float x, y;
    std::string content;
    VESStyle style;
    std::string anchor = "start";

public:
    TextNode(std::string id, float x, float y, std::string text, VESStyle s)
        : id(std::move(id)), x(x), y(y), content(std::move(text)), style(std::move(s)) {}

    void setX(float nx) { x = nx; }
    void setY(float ny) { y = ny; }
    void setAnchor(std::string a) { anchor = std::move(a); }
    
    // --- NEW: State Mutators ---
    std::string getContent() const { return content; }
    void setContent(std::string newText) { content = std::move(newText); }

    std::string getId() const override { return id; }
    
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override { return false; } 

    std::string asSVG() const override {
        std::ostringstream svg;
        svg << "  <text id=\"" << id << "\" x=\"" << x << "\" y=\"" << y 
            << "\" fill=\"" << style.fill << "\" font-size=\"" << style.fontSize 
            << "\" font-family=\"sans-serif\" text-anchor=\"" << anchor 
            << "\" dominant-baseline=\"central\">" 
            << sanitizeXML(content)
            << "</text>\n";
        return svg.str();
    }
};

// 4. Box Element (The Container)
class BoxNode : public ASTNode {
private:
    std::string id;
    float x, y, width, height;
    VESStyle style;
    std::vector<std::shared_ptr<ASTNode>> children;

public:
    BoxNode(std::string id, float x, float y, float w, float h, VESStyle s)
        : id(std::move(id)), x(x), y(y), width(w), height(h), style(std::move(s)) {}

    // When a child is added, the Box positions it automatically!
    void addChild(std::shared_ptr<ASTNode> child) {
        if (auto textChild = std::dynamic_pointer_cast<TextNode>(child)) {
            if (style.align == "center") {
                textChild->setX(x + width / 2.0f);
                textChild->setY(y + height / 2.0f);
                textChild->setAnchor("middle");
            } else {
                textChild->setX(x + 10);
                textChild->setY(y + 20); // Default padding
                textChild->setAnchor("start");
            }
        }
        children.push_back(child);
    }

    const std::vector<std::shared_ptr<ASTNode>>& getChildren() const { return children; }
    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    
    bool contains(float px, float py) const override {
        return (px >= x && px <= x + width && py >= y && py <= y + height);
    }

    std::string asSVG() const override {
        std::ostringstream svg;
        // Draw the Box with rounded corners again!
        svg << "  <rect id=\"" << id << "\" x=\"" << x << "\" y=\"" << y 
            << "\" width=\"" << width << "\" height=\"" << height 
            << "\" fill=\"" << style.fill << "\" rx=\"8\" />\n";
        
        // Draw the Children!
        for (const auto& child : children) {
            svg << child->asSVG();
        }
        return svg.str();
    }
};

// 5. Circle Element
class CircleNode : public ASTNode {
private:
    std::string id;
    float cx, cy, radius;
    VESStyle style;

public:
    CircleNode(std::string id, float cx, float cy, float r, VESStyle s)
        : id(std::move(id)), cx(cx), cy(cy), radius(r), style(std::move(s)) {}

    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override {
        float dx = px - cx;
        float dy = py - cy;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    std::string asSVG() const override {
        std::ostringstream svg;
        svg << "  <circle id=\"" << id << "\" cx=\"" << cx << "\" cy=\"" << cy 
            << "\" r=\"" << radius << "\" fill=\"" << style.fill << "\" />\n";
        return svg.str();
    }
};