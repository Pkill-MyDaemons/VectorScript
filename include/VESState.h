#pragma once
#include <map>
#include <string>

// A global registry for Data Binding!
class VESState {
public:
    static std::map<std::string, std::string> values;
};

// Define the static map memory
inline std::map<std::string, std::string> VESState::values;