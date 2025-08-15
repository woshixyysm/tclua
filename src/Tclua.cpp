#include "Tclua.h"
#include "Tokenizer.h"
#include <sstream>

void Tclua::execute(const std::string& script) {
    std::istringstream iss(script);
    std::string line;
    currentLine = 0;
    
    while (std::getline(iss, line)) {
        currentLine++;
        try {
            execute_line(line);
        } catch (const InterpreterException& e) {
            std::cerr << "Error: " << e.fullMessage() << std::endl;
            if (e.getLine() == -1) std::cerr << "  At line: " << currentLine << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error at line " << currentLine << ": " << e.what() << std::endl;
        }
    }
}

Value Tclua::execute_line(const std::string& line) {
    auto tokens = Tokenizer::tokenize(line, currentLine);
    if (tokens.empty()) return 0.0;
    
    cmdHandler.setLineNumber(currentLine);
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    return cmdHandler.executeCommand(tokens[0], args);
}
