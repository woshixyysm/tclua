#ifndef INTERPRETER_EXCEPTION_H
#define INTERPRETER_EXCEPTION_H

#include <stdexcept>
#include <string>

class InterpreterException : public std::runtime_error {
public:
    int line = -1;
    std::string context;
    
    InterpreterException(const std::string& msg, int line = -1, std::string ctx = "")
        : std::runtime_error(msg), line(line), context(ctx) {}
    
    virtual std::string fullMessage() const;
};

class SyntaxError : public InterpreterException {
public:
    SyntaxError(const std::string& msg, int line = -1, std::string ctx = "")
        : InterpreterException("Syntax error: " + msg, line, ctx) {}
};

class RuntimeError : public InterpreterException {
public:
    RuntimeError(const std::string& msg, int line = -1, std::string ctx = "")
        : InterpreterException("Runtime error: " + msg, line, ctx) {}
};

class UndefinedVariable : public RuntimeError {
public:
    UndefinedVariable(const std::string& var, int line = -1)
        : RuntimeError("Undefined variable: " + var, line) {}
};

#endif // INTERPRETER_EXCEPTION_H
