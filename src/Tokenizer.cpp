#include "Tokenizer.h"
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::string> Tokenizer::tokenize(const std::string& input, int line) {
    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    
    while (iss >> token) {
        // 移除常见的标点符号
        token.erase(std::remove_if(token.begin(), token.end(), 
                   [](char c) { return c == ',' || c == ';' || c == ':'; }), 
                   token.end());
        tokens.push_back(token);
    }
    
    return tokens;
}