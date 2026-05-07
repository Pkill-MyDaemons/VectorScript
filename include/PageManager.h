#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include "AST.h"
#include "Lexer.h"
#include "Parser.h"

class PageManager {
private:
    std::vector<std::shared_ptr<ASTNode>> activeNodes;
    bool dirty = false;

    // Helper: Read a physical file from the hard drive
    std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[Error] Could not open file: " << filepath << std::endl;
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

public:
    // Helper: A tiny, dependency-free JSON parser just for our entry page
    std::string getEntryPage() {
        std::string json = readFile("vesconfig.json");
        size_t pos = json.find("\"entry\"");
        if (pos != std::string::npos) {
            size_t start = json.find('"', pos + 7);
            size_t end = json.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                return json.substr(start + 1, end - start - 1);
            }
        }
        return "home.ves"; // Fallback
    }

    // The Engine: Loads a .ves file and compiles it!
    void loadPage(const std::string& filename) {
        std::string filepath = "views/" + filename;
        std::string code = readFile(filepath);
        if (code.empty()) return;

        try {
            Lexer lexer(code);
            Parser parser(lexer.tokenize());
            activeNodes = parser.parse(); // Swap the active UI!
            dirty = true;                 // Tell the renderer to redraw
            std::cout << "[PageManager] Loaded: " << filename << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "\n[Compiler Error in " << filename << "]\n" << e.what() << "\n" << std::endl;
        }
    }

    const std::vector<std::shared_ptr<ASTNode>>& getActiveNodes() const { return activeNodes; }
    bool isDirty() const { return dirty; }
    void clearDirty() { dirty = false; }
    
    // NEW: Allow backend libraries to trigger a screen redraw!
    void markDirty() { dirty = true; }
};