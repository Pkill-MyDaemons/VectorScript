#pragma once
#include "Lexer.h"
#include "AST.h"
#include <memory>
#include <stdexcept>
#include <map> // NEW: For our Symbol Table

class Parser {
private:
    std::vector<Token> tokens;
    size_t pos = 0;
    
    // THE SYMBOL TABLE: Remembers elements we have declared!
    std::map<std::string, std::shared_ptr<ASTNode>> environment;

    Token current() { return tokens[pos]; }
    
    Token consume(TokenType expected, const std::string& errorMsg) {
        if (current().type == expected) {
            return tokens[pos++];
        }
        throw std::runtime_error("VectorScript Syntax Error: " + errorMsg);
    }

    // --- HELPER: Parse Styles and Children ---
    VESStyle parseStyleBlock(std::shared_ptr<BoxNode> parentBox = nullptr) {
        VESStyle style;
        if (current().type != TokenType::LBrace) return style;

        consume(TokenType::LBrace, "Expected '{'");

        while (current().type != TokenType::RBrace) {
            std::string key = consume(TokenType::Identifier, "Expected style key.").value;
            consume(TokenType::Colon, "Expected ':'");

            if (key == "fill") {
                style.fill = consume(TokenType::String, "Expected color.").value;
            } else if (key == "size") {
                style.fontSize = std::stof(consume(TokenType::Number, "Expected size.").value);
            } else if (key == "onClick") {
                style.onClick = consume(TokenType::String, "Expected C++ function.").value;
            } else if (key == "align") {
                style.align = consume(TokenType::String, "Expected alignment.").value;
            } else if (key == "gradient") {
                style.gradient = consume(TokenType::String, "Expected comma separated colors.").value;
            } else if (key == "spacing") {
                style.spacing = std::stof(consume(TokenType::Number, "Expected spacing value.").value);
            } else if (key == "child" && parentBox != nullptr) {
                
                // --- NEW: Variable Lookup! ---
                if (current().type == TokenType::Identifier) {
                    std::string varName = consume(TokenType::Identifier, "Expected variable name.").value;
                    
                    // Does this variable exist in our memory?
                    if (environment.find(varName) != environment.end()) {
                        parentBox->addChild(environment[varName]); // Link it!
                    } else {
                        throw std::runtime_error("VectorScript Error: Undefined variable '" + varName + "'");
                    }
                } 
                // Fallback: We still support the old inline syntax just in case
                else if (current().type == TokenType::New) {
                    consume(TokenType::New, "Expected 'new'");
                    consume(TokenType::Identifier, "Expected 'Text'");
                    consume(TokenType::LParen, "Expected '('");
                    std::string content = consume(TokenType::String, "Expected string.").value;
                    consume(TokenType::RParen, "Expected ')'");
                    VESStyle childStyle = parseStyleBlock(nullptr); 
                    parentBox->addChild(std::make_shared<TextNode>("inline_child", 0, 0, content, childStyle));
                }
            }

            if (current().type == TokenType::Comma) consume(TokenType::Comma, "Expected ','");
        }

        consume(TokenType::RBrace, "Expected '}'");
        return style;
    }

    // --- ELEMENT PARSERS ---
    std::shared_ptr<ASTNode> parseBoxDeclaration(const std::string& type) {
        consume(TokenType::Identifier, "Expected type declaration"); 
        
        // Now it correctly grabs 'menu' as the variable name, and '=' as the operator!
        std::string varName = consume(TokenType::Identifier, "Expected id").value;
        consume(TokenType::Equals, "Expected '='");
        consume(TokenType::New, "Expected 'new'");
        consume(TokenType::Identifier, "Expected type"); 
        consume(TokenType::LParen, "Expected '('");

        float x = std::stof(consume(TokenType::Number, "Expected X").value); consume(TokenType::Comma, ",");
        float y = std::stof(consume(TokenType::Number, "Expected Y").value); consume(TokenType::Comma, ",");
        float w = std::stof(consume(TokenType::Number, "Expected Width").value); consume(TokenType::Comma, ",");
        float h = std::stof(consume(TokenType::Number, "Expected Height").value);
        consume(TokenType::RParen, "Expected ')'");

        std::shared_ptr<BoxNode> container;
        if (type == "VStack") container = std::make_shared<VStackNode>(varName, x, y, w, h, VESStyle());
        else if (type == "HStack") container = std::make_shared<HStackNode>(varName, x, y, w, h, VESStyle());
        else container = std::make_shared<BoxNode>(varName, x, y, w, h, VESStyle());

        VESStyle s = parseStyleBlock(container); 
        
        // Re-build with final styles
        std::shared_ptr<BoxNode> finalContainer;
        if (type == "VStack") finalContainer = std::make_shared<VStackNode>(varName, x, y, w, h, s);
        else if (type == "HStack") finalContainer = std::make_shared<HStackNode>(varName, x, y, w, h, s);
        else finalContainer = std::make_shared<BoxNode>(varName, x, y, w, h, s);

        for(auto& child : container->getChildren()) finalContainer->addChild(child);

        consume(TokenType::SemiColon, "Expected ';'");
        environment[varName] = finalContainer;
        return finalContainer;
    }

    std::shared_ptr<ASTNode> parseTextDeclaration() {
        std::string varName = consume(TokenType::Identifier, "Expected id").value;
        consume(TokenType::Equals, "Expected '='");
        consume(TokenType::New, "Expected 'new'");
        consume(TokenType::Identifier, "Expected 'Text'");
        consume(TokenType::LParen, "Expected '('");

        float x = std::stof(consume(TokenType::Number, "Expected X").value); consume(TokenType::Comma, ",");
        float y = std::stof(consume(TokenType::Number, "Expected Y").value); consume(TokenType::Comma, ",");
        
        std::string content = "";
        std::string binding = "";
        
        // Is it a static string, or a dynamic data binding?
        if (current().type == TokenType::String) {
            content = consume(TokenType::String, "Expected text content").value;
        } else if (current().type == TokenType::Identifier) {
            binding = consume(TokenType::Identifier, "Expected variable binding").value;
        }
        
        consume(TokenType::RParen, "Expected ')'");

        VESStyle s = parseStyleBlock(nullptr);
        consume(TokenType::SemiColon, "Expected ';'");

        auto node = std::make_shared<TextNode>(varName, x, y, content, binding, s);
        environment[varName] = node;
        return node;
    }

    std::shared_ptr<ASTNode> parseCircleDeclaration() {
        consume(TokenType::Identifier, "Expected 'Circle'");
        std::string varName = consume(TokenType::Identifier, "Expected id").value;
        consume(TokenType::Equals, "Expected '='");
        consume(TokenType::New, "Expected 'new'");
        consume(TokenType::Identifier, "Expected 'Circle'");
        consume(TokenType::LParen, "Expected '('");

        float cx = std::stof(consume(TokenType::Number, "Expected CX").value); consume(TokenType::Comma, ",");
        float cy = std::stof(consume(TokenType::Number, "Expected CY").value); consume(TokenType::Comma, ",");
        float r = std::stof(consume(TokenType::Number, "Expected Radius").value);
        consume(TokenType::RParen, "Expected ')'");

        VESStyle s = parseStyleBlock();
        consume(TokenType::SemiColon, "Expected ';'");

        auto node = std::make_shared<CircleNode>(varName, cx, cy, r, s);
        environment[varName] = node; // Save to Symbol Table!
        return node;
    }

public:
    Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    // Replaces your main parse() loop at the bottom
    std::vector<std::shared_ptr<ASTNode>> parse() {
        std::vector<std::shared_ptr<ASTNode>> nodes;

        while (current().type != TokenType::EndOfFile) {
            
            // Look for Box, VStack, OR HStack!
            if (current().type == TokenType::Identifier && current().value == "import") {
                pos++; // Skip 'import'
                pos++; // Skip the library name (e.g. 'utils')
            } else if (current().type == TokenType::Identifier && 
               (current().value == "Box" || current().value == "VStack" || current().value == "HStack")) {
                nodes.push_back(parseBoxDeclaration(current().value));
            } 
            else if (current().type == TokenType::Identifier && current().value == "Text") {
                nodes.push_back(parseTextDeclaration());
            }
            else if (current().type == TokenType::Identifier && current().value == "Circle") {
                nodes.push_back(parseCircleDeclaration());
            }
            else {
                pos++; 
            }
        }
        return nodes;
    }
};