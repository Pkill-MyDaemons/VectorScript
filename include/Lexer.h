#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cctype>

enum class TokenType {
    Identifier, Number, String, Equals, New,
    LParen, RParen, Comma, SemiColon,
    LBrace,     // {
    RBrace,     // }
    Colon,      // :
    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
private:
    std::string source;
    size_t pos = 0;

    char peek() { return pos < source.length() ? source[pos] : '\0'; }
    char advance() { return pos < source.length() ? source[pos++] : '\0'; }
    void skipWhitespace() { while (isspace(peek())) advance(); }

public:
    Lexer(std::string src) : source(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (pos < source.length()) {
            skipWhitespace();
            char c = peek();
            if (c == '\0') break;

            // --- NEW: IGNORE COMMENTS ---
            if (c == '/' && pos + 1 < source.length() && source[pos + 1] == '/') {
                while (peek() != '\n' && peek() != '\0') advance();
                continue; // Skip to the next line
            }

            if (isalpha(c)) {
                std::string ident;
                // Allow letters, numbers, and underscores
                while (isalnum(peek()) || peek() == '_') ident += advance(); 
                
                if (ident == "new") tokens.push_back({TokenType::New, ident});
                else tokens.push_back({TokenType::Identifier, ident});
            } 
            else if (isdigit(c)) {
                std::string num;
                while (isdigit(peek()) || peek() == '.') num += advance();
                tokens.push_back({TokenType::Number, num});
            }
            else if (c == '"') {
                advance(); 
                std::string str;
                while (peek() != '"' && peek() != '\0') str += advance();
                advance(); 
                tokens.push_back({TokenType::String, str});
            }
            else if (c == '=') { tokens.push_back({TokenType::Equals, "="}); advance(); }
            else if (c == '(') { tokens.push_back({TokenType::LParen, "("}); advance(); }
            else if (c == ')') { tokens.push_back({TokenType::RParen, ")"}); advance(); }
            else if (c == '{') { tokens.push_back({TokenType::LBrace, "{"}); advance(); }
            else if (c == '}') { tokens.push_back({TokenType::RBrace, "}"}); advance(); }
            else if (c == ':') { tokens.push_back({TokenType::Colon, ":"}); advance(); }
            else if (c == ',') { tokens.push_back({TokenType::Comma, ","}); advance(); }
            else if (c == ';') { tokens.push_back({TokenType::SemiColon, ";"}); advance(); }
            else { advance(); } 
        }
        tokens.push_back({TokenType::EndOfFile, ""});
        return tokens;
    }
};