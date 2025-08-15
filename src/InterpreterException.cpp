#include "InterpreterException.h"
#include <string>

InterpreterException::InterpreterException(const std::string& msg, int ln) 
    : std::runtime_error(msg), line(ln) {}

std::string InterpreterException::fullMessage() const {
    return std::string(what()) + (line > 0 ? " at line " + std::to_string(line) : "");
}

UndefinedVariable::UndefinedVariable(const std::string& name, int ln)
    : InterpreterException("Undefined variable"), varName(name) {
    line = ln;
}

std::string UndefinedVariable::fullMessage() const {
    return "Undefined variable: " + varName + 
           (line > 0 ? " at line " + std::to_string(line) : "");
}

RuntimeError::RuntimeError(const std::string& msg, int ln)
    : InterpreterException(msg) {
    line = ln;
}

std::string RuntimeError::fullMessage() const {
    return "Runtime error: " + std::string(what()) + 
           (line > 0 ? " at line " + std::to_string(line) : "");
}
