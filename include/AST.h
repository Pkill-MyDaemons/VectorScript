#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <VESState.h>

// --- HELPER: String Splitter for Gradients ---
inline std::vector<std::string> splitString(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) result.push_back(item);
    return result;
}
class ASTNode;
class IconNode;
// 1. Updated Style Structure
struct VESStyle {
    std::string fill = "none";
    std::string gradient = ""; 
    int fontSize = 16;
    std::string onClick = ""; 
    std::string align = "left"; 
    float spacing = 10;        
    
    // NEW: Safely holds children in memory until the parser is finished!
    std::vector<std::shared_ptr<ASTNode>> children; 
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
    virtual void shift(float dx, float dy) = 0; 
    
    // NEW: Allow the layout engine to measure any node!
    virtual float getWidth() const { return 0; }
    virtual float getHeight() const { return 0; }
};

// 3. Text Element (Moved up so Box can see it)
// 3. Text Element
// 3. Text Element (Now with Data Binding & Stack Support!)
class TextNode : public ASTNode {
private:
    std::string id;
    float x, y;
    std::string content;
    std::string binding; // NEW: Holds variables like "utils.Time"
    VESStyle style;
    std::string anchor = "start";

public:
    // Notice the constructor requires the 'bind' string now
    TextNode(std::string id, float x, float y, std::string text, std::string bind, VESStyle s)
        : id(std::move(id)), x(x), y(y), content(std::move(text)), binding(std::move(bind)), style(std::move(s)) {}
        
    float getWidth() const override { return content.length() * (style.fontSize * 0.6f); }
    float getHeight() const override { return style.fontSize; }

    // --- Mutators ---
    void setX(float nx) { x = nx; }
    void setY(float ny) { y = ny; }
    void setAnchor(std::string a) { anchor = std::move(a); }
    std::string getContent() const { return content; }
    void setContent(std::string newText) { content = std::move(newText); }

    // --- ASTNode Overrides ---
    void shift(float dx, float dy) override { x += dx; y += dy; } // Allows VStacks to push this text down!
    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override { return false; } 

    // --- Rendering Engine ---
    std::string asSVG() const override {
        
        // 1. DATA BINDING MAGIC! (If bound to a variable, grab live data from C++ state)
        std::string displayText = content;
        if (!binding.empty() && VESState::values.count(binding)) {
            displayText = VESState::values[binding];
        }

        // 2. Generate the SVG string
        std::ostringstream svg;
        svg << "  <text id=\"" << id << "\" x=\"" << x << "\" y=\"" << y 
            << "\" fill=\"" << style.fill << "\" font-size=\"" << style.fontSize 
            << "\" font-family=\"sans-serif\" text-anchor=\"" << anchor 
            << "\" dominant-baseline=\"central\">" 
            << sanitizeXML(displayText) // 3. SANITIZER: Keeps LunaSVG from crashing on '<' symbols!
            << "</text>\n";
            
        return svg.str();
    }
};
class IconNode : public ASTNode {
private:
    std::string id, filepath;
    float x, y, width, height;
    VESStyle style;
    std::string svgCache;

public:
    IconNode(std::string id, float x, float y, float w, float h, std::string path, VESStyle s)
        : id(std::move(id)), x(x), y(y), width(w), height(h), filepath(std::move(path)), style(std::move(s)) {
        std::ifstream file("views/" + filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            svgCache = buffer.str();
            
            // --- THE FIX: Explicitly find the <svg> tag, skipping any <?xml> headers! ---
            size_t svgStart = svgCache.find("<svg");
            if (svgStart != std::string::npos) {
                size_t start = svgCache.find(">", svgStart);
                size_t end = svgCache.rfind("</svg>");
                if (start != std::string::npos && end != std::string::npos) {
                    svgCache = svgCache.substr(start + 1, end - start - 1);
                }
            }
        } else {
            svgCache = "<rect width=\"24\" height=\"24\" fill=\"red\"/>";
        }
    }
    void setX(float nx) { x = nx; }
    void setY(float ny) { y = ny; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    void shift(float dx, float dy) override { x += dx; y += dy; }
    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override { return (px >= x && px <= x + width && py >= y && py <= y + height); }

    std::string asSVG() const override {
        std::ostringstream svg;
        std::string iconColor = (style.fill == "none") ? "#11111B" : style.fill;

        svg << "  <svg id=\"" << id << "\" x=\"" << x << "\" y=\"" << y 
            << "\" width=\"" << width << "\" height=\"" << height 
            << "\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"" << iconColor 
            << "\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">\n"
            << svgCache << "\n  </svg>\n";
            
        return svg.str();
    }
};
// 4. Box Element (The Container)
// 4. Box Element (Now with Gradients!)
class BoxNode : public ASTNode {
protected:
    std::string id;
    float x, y, width, height;
    VESStyle style;
    std::vector<std::shared_ptr<ASTNode>> children;

public:
    BoxNode(std::string id, float x, float y, float w, float h, VESStyle s)
        : id(std::move(id)), x(x), y(y), width(w), height(h), style(std::move(s)) {}
        void setX(float nx) { x = nx; }
        void setY(float ny) { y = ny; }
        float getWidth() const { return width; }
        float getHeight() const { return height; }

    // Notice the 'virtual' keyword! This lets Stacks override it.
    virtual void addChild(std::shared_ptr<ASTNode> child) {
        // 1. Is it Text?
        if (auto textChild = std::dynamic_pointer_cast<TextNode>(child)) {
            if (style.align == "center") {
                textChild->setX(x + width / 2.0f);
                textChild->setY(y + height / 2.0f);
                textChild->setAnchor("middle");
            } else {
                textChild->setX(x + 10);
                textChild->setY(y + 20);
                textChild->setAnchor("start");
            }
        } 
        // 2. Is it an Icon?
        else if (auto iconChild = std::dynamic_pointer_cast<IconNode>(child)) {
            if (style.align == "center") {
                // Center it perfectly using the Box's width and the Icon's width!
                iconChild->setX(x + (width - iconChild->getWidth()) / 2.0f);
                iconChild->setY(y + (height - iconChild->getHeight()) / 2.0f);
            } else {
                iconChild->setX(x + 10);
                iconChild->setY(y + 10);
            }
        } else if(auto boxChild = std::dynamic_pointer_cast<BoxNode>(child)){
            if (style.align == "center") {
                // Center it perfectly using the Box's width and the Icon's width!
                boxChild->setX(x + (width - boxChild->getWidth()) / 2.0f);
                boxChild->setY(y + (height - boxChild->getHeight()) / 2.0f);
            } else {
                iconChild->setX(x + 10);
                boxChild->setY(y + 10);
            }
        }
        
        children.push_back(child);
    }

    void shift(float dx, float dy) override {
        x += dx; y += dy;
        for (auto& child : children) child->shift(dx, dy); 
    }

    const std::vector<std::shared_ptr<ASTNode>>& getChildren() const { return children; }
    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override { return (px >= x && px <= x + width && py >= y && py <= y + height); }

    std::string asSVG() const override {
        std::ostringstream svg;
        std::string fillAttr = style.fill;

        if (!style.gradient.empty()) {
            auto colors = splitString(style.gradient, ',');
            std::string gradId = "grad_" + id;
            fillAttr = "url(#" + gradId + ")";
            
            svg << "  <defs>\n    <linearGradient id=\"" << gradId << "\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
            for (size_t i = 0; i < colors.size(); ++i) {
                float offset = (float)i / (colors.size() - 1) * 100.0f;
                svg << "      <stop offset=\"" << offset << "%\" stop-color=\"" << colors[i] << "\" />\n";
            }
            svg << "    </linearGradient>\n  </defs>\n";
        }

        svg << "  <rect id=\"" << id << "\" x=\"" << x << "\" y=\"" << y 
            << "\" width=\"" << width << "\" height=\"" << height 
            << "\" fill=\"" << fillAttr << "\" rx=\"8\" />\n";
        
        for (const auto& child : children) svg << child->asSVG();
        return svg.str();
    }
};

// 5. Vertical Stack (Auto-Layout!)
class VStackNode : public BoxNode {
private:
    float currentYOffset;
public:
    VStackNode(std::string id, float x, float y, float w, float h, VESStyle s)
        : BoxNode(id, x, y, w, h, s), currentYOffset(y) {}

    void addChild(std::shared_ptr<ASTNode> child) override {
        float childX = x; // Default: left aligned with padding
        
        // Cross-Axis Alignment: Center the child horizontally!
        if (style.align == "center") {
            childX = x + (width - child->getWidth()) / 2.0f;
        }

        child->shift(childX, currentYOffset); 
        
        // Measure the child's exact height so the next item doesn't overlap!
        currentYOffset += child->getHeight() + style.spacing; 
        children.push_back(child);
    }
};

// 6. Horizontal Stack (Dynamic Auto-Layout!)
class HStackNode : public BoxNode {
private:
    float currentXOffset;
public:
    HStackNode(std::string id, float x, float y, float w, float h, VESStyle s)
        : BoxNode(id, x, y, w, h, s), currentXOffset(x) {}

    void addChild(std::shared_ptr<ASTNode> child) override {
        float childY = y;
        
        if (style.align == "center") {
            childY = y + (height - child->getHeight()) / 2.0f;
        }

        child->shift(currentXOffset, childY); 
        currentXOffset += child->getWidth() + style.spacing; 
        children.push_back(child);
    }
};

class CircleNode : public ASTNode {
private:
    std::string id;
    float cx, cy, radius;
    VESStyle style;

public:
    CircleNode(std::string id, float cx, float cy, float r, VESStyle s)
        : id(std::move(id)), cx(cx), cy(cy), radius(r), style(std::move(s)) {}

    void shift(float dx, float dy) override { cx += dx; cy += dy; } // Fixed!

    std::string getId() const override { return id; }
    std::string getOnClick() const override { return style.onClick; }
    bool contains(float px, float py) const override {
        float dx = px - cx; float dy = py - cy;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    std::string asSVG() const override {
        std::ostringstream svg;
        svg << "  <circle id=\"" << id << "\" cx=\"" << cx << "\" cy=\"" << cy 
            << "\" r=\"" << radius << "\" fill=\"" << style.fill << "\" />\n";
        return svg.str();
    }
};
// 8. Vector Icon Node (Now immune to hidden XML headers!)
