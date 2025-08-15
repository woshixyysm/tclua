#ifndef INTERPRETER_EXCEPTION_H
#define INTERPRETER_EXCEPTION_H

#include <string>
#include <stdexcept>

class InterpreterException : public std::runtime_error {
protected:
    int line;
    
public:
    InterpreterException(const std::string& msg, int ln = -1);
    virtual ~InterpreterException() = default;
    
    virtual std::string fullMessage() const;
    
    int getLine() const { return line; }
};

class UndefinedVariable : public InterpreterException {
    std::string varName;
public:
    UndefinedVariable(const std::string& name, int ln = -1);
    virtual std::string fullMessage() const override;
};

class RuntimeError : public InterpreterException {
public:
    RuntimeError(const std::string& msg, int ln = -1);
    virtual std::string fullMessage() const override;
};

#endif // INTERPRETER_EXCEPTION_H
