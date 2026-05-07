#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <functional>

// 1. The universal data type that VectorScript and C++ share
using VESValue = std::variant<int, float, bool, std::string>;

// 2. The standard signature for all C++ callbacks
using VESCallback = std::function<void(const std::vector<VESValue>&)>;

// 3. The Bridge that connects the parsed script to C++ logic
class VESBridge {
private:
    std::map<std::string, VESCallback> registry;

public:
    // Register a C++ function
    void registerFunction(const std::string& name, VESCallback func) {
        registry[name] = func;
        std::cout << "[VESBridge] Linked C++ function: '" << name << "'" << std::endl;
    }

    // Execute a function (Triggered by your hit-tester)
    void execute(const std::string& name, const std::vector<VESValue>& args) {
        if (registry.find(name) != registry.end()) {
            registry[name](args);
        } else {
            std::cerr << "[VES Error] No C++ function registered for: '" << name << "'" << std::endl;
        }
    }
};