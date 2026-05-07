#pragma once
#include <SFML/Graphics.hpp>
#include "AST.h"
#include <vector>
#include <memory>
#include <iostream>

class SFMLRenderer {
private:
    sf::Font font;
    bool fontLoaded = false;

    sf::Color hexToColor(const std::string& hex) {
        if (hex.length() == 7 && hex[0] == '#') {
            std::uint8_t r = static_cast<std::uint8_t>(std::stoi(hex.substr(1, 2), nullptr, 16));
            std::uint8_t g = static_cast<std::uint8_t>(std::stoi(hex.substr(3, 2), nullptr, 16));
            std::uint8_t b = static_cast<std::uint8_t>(std::stoi(hex.substr(5, 2), nullptr, 16));
            return sf::Color(r, g, b);
        }
        return sf::Color::White; 
    }

public:
    SFMLRenderer() {
        if (!font.openFromFile("arial.ttf")) {
            std::cerr << "[Warning] Could not load arial.ttf. Text will be invisible!" << std::endl;
        } else {
            fontLoaded = true;
        }
    }

    void draw(sf::RenderWindow& window, const std::vector<std::shared_ptr<ASTNode>>& nodes) {
        for (const auto& node : nodes) {
            
            // 1. Is it a Box?
            if (auto box = dynamic_cast<BoxNode*>(node.get())) {
                
                // Draw the Parent Box
                sf::RectangleShape rect({box->getWidth(), box->getHeight()});
                rect.setPosition({box->getX(), box->getY()});
                rect.setFillColor(hexToColor(box->getStyle().fill));
                window.draw(rect);

                // Draw the Nested Children
                for (const auto& child : box->getChildren()) {
                    if (auto textNode = dynamic_cast<TextNode*>(child.get())) {
                        if (fontLoaded) {
                            sf::Text text(font, textNode->getContent(), textNode->getStyle().fontSize);
                            text.setFillColor(hexToColor(textNode->getStyle().fill));

                            // Auto-Centering Logic!
                            if (box->getStyle().align == "center") {
                                sf::FloatRect bounds = text.getLocalBounds();
                                float centerX = box->getX() + (box->getWidth() / 2.0f);
                                float centerY = box->getY() + (box->getHeight() / 2.0f);

                                // SFML 3.0 uses .size.x and .position.x!
                                text.setPosition({
                                    centerX - (bounds.size.x / 2.0f) - bounds.position.x,
                                    centerY - (bounds.size.y / 2.0f) - bounds.position.y
                                });
                            } else {
                                text.setPosition({box->getX() + 10, box->getY() + 10});
                            }
                            window.draw(text);
                        }
                    }
                }
            }
            // 2. Is it a Circle?
            else if (auto circle = dynamic_cast<CircleNode*>(node.get())) {
                sf::CircleShape circ(circle->getRadius());
                circ.setPosition({circle->getCX() - circle->getRadius(), circle->getCY() - circle->getRadius()});
                circ.setFillColor(hexToColor(circle->getStyle().fill));
                window.draw(circ);
            }
            // 3. Is it a standalone Text Node? (Like the Dashboard Title)
            else if (auto textNode = dynamic_cast<TextNode*>(node.get())) {
                if (fontLoaded) {
                    sf::Text text(font, textNode->getContent(), textNode->getStyle().fontSize);
                    text.setPosition({textNode->getX(), textNode->getY()});
                    text.setFillColor(hexToColor(textNode->getStyle().fill));
                    window.draw(text);
                }
            }
        }
    }
};