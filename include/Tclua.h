#ifndef LUA_INTERPRETER_H
#define LUA_INTERPRETER_H

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <memory>

#include "CallStack.h"
#include "VariableManager.h"
#include "ExpressionParser.h"
#include "CommandHandler.h"

class Tclua {
private:
    CallStack callStack;
    VariableManager varManager;
    ExpressionParser exprParser;
    CommandHandler cmdHandler;
    int currentLine = 0;
    
    // 添加私有方法声明
    Value execute_line(const std::string& line);
    
public:
    Tclua() 
        : varManager(callStack), exprParser(varManager), 
          cmdHandler(varManager, exprParser, callStack) {}
    
    void execute(const std::string& script);
};

#endif // LUA_INTERPRETER_H