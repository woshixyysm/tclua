#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <memory>
#include <functional>

#include "InterpreterException.h"
#include "Table.h"
#include "VariableManager.h"
#include "ExpressionParser.h"
#include "CallStack.h"

class CommandHandler {
private:
    struct Procedure {
        std::vector<std::string> parameters;
        std::string body;
        std::map<std::string, Value> capturedVars; // 闭包捕获的变量
    };
    
    struct DebugInfo {
        bool breakpointsEnabled = false;
        std::map<int, std::string> breakpoints; // 行号 -> 条件
        bool stepMode = false;
    };
    
    struct TryCatchBlock {
        std::string catchVar;
        int catchLine;
    };
    
    VariableManager& varManager;
    ExpressionParser& exprParser;
    DebugInfo debugInfo;
    int currentLine = -1;
    CallStack& callStack;
    
    std::map<std::string, Procedure> procedures;
    std::map<std::string, std::shared_ptr<Table>> classes;
    std::stack<TryCatchBlock> tryStack;
    std::stack<std::string> loopStack;
    
public:
    CommandHandler(VariableManager& vm, ExpressionParser& ep, CallStack& cs) 
        : varManager(vm), exprParser(ep), callStack(cs) {}
    
    void setLineNumber(int line) { currentLine = line; }
    
    Value executeCommand(const std::string& cmd, const std::vector<std::string>& args);
    
private:
    bool shouldBreak();
    void enterDebugMode(const std::string& cmd, const std::vector<std::string>& args);
    void handleDebugCommand(const std::string& cmd);
    void printBacktrace();
    void printVariables(const std::string& filter);
    
    Value handleSet(const std::vector<std::string>& args);
    Value handleExpr(const std::vector<std::string>& args);
    Value handlePuts(const std::vector<std::string>& args);
    Value handleProc(const std::vector<std::string>& args);
    Value handleIf(const std::vector<std::string>& args);
    Value handleFor(const std::vector<std::string>& args);
    Value handleIncr(const std::vector<std::string>& args);
    Value handleReturn(const std::vector<std::string>& args);
    Value handleString(const std::vector<std::string>& args);
    Value handleWhile(const std::vector<std::string>& args);
    Value handleSwitch(const std::vector<std::string>& args);
    Value handleClass(const std::vector<std::string>& args);
    Value handleNew(const std::vector<std::string>& args);
    Value handleSetMetatable(const std::vector<std::string>& args);
    Value handleTry(const std::vector<std::string>& args);
    Value handleTable(const std::vector<std::string>& args);
    Value handleBreakpoint(const std::vector<std::string>& args);
    Value handleStep(const std::vector<std::string>& args);
    Value handleMath(const std::vector<std::string>& args);
    Value handleFile(const std::vector<std::string>& args);
    Value handleModule(const std::vector<std::string>& args);
    Value handleImport(const std::vector<std::string>& args);
    Value executeProcedure(const std::string& name, const std::vector<std::string>& args);
};

#endif // COMMAND_HANDLER_H
