#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>
#include <cmath>
#include <memory>
#include <sstream>
#include <cctype>
#include <functional>
#include <algorithm>
#include <stack>

// ====================== 错误处理模块 ======================
class InterpreterException : public std::runtime_error {
public:
    int line = -1;
    std::string context;
    
    InterpreterException(const std::string& msg, int line = -1, std::string ctx = "")
        : std::runtime_error(msg), line(line), context(ctx) {}
    
    virtual std::string fullMessage() const {
        std::ostringstream oss;
        if (line != -1) oss << "Line " << line << ": ";
        oss << what();
        if (!context.empty()) oss << " [Context: " << context << "]";
        return oss.str();
    }
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

// ====================== 值类型和表结构 ======================
class Table;
using Value = std::variant<double, std::string, bool, std::nullptr_t, std::shared_ptr<Table>>;

class Table : public std::enable_shared_from_this<Table> {
public:
    std::map<std::string, Value> fields;
    std::shared_ptr<Table> metatable;
    
    bool contains(const std::string& key) const {
        return fields.find(key) != fields.end();
    }
    
    Value get(const std::string& key, int line = -1) const {
        auto it = fields.find(key);
        if (it != fields.end()) return it->second;
        
        if (metatable) {
            try {
                if (metatable->contains("__index")) {
                    Value indexVal = metatable->get("__index", line);
                    
                    if (auto* t = std::get_if<std::shared_ptr<Table>>(&indexVal)) {
                        return (*t)->get(key, line);
                    }
                }
            } catch (...) {}
        }
        
        throw UndefinedVariable(key, line);
    }
    
    void set(const std::string& key, const Value& value) {
        fields[key] = value;
    }
    
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        for (const auto& pair : fields) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    std::vector<Value> values() const {
        std::vector<Value> result;
        for (const auto& pair : fields) {
            result.push_back(pair.second);
        }
        return result;
    }
};

// ====================== 变量管理器 ======================
class VariableManager {
private:
    struct Variable {
        Value value;
        bool isTableField = false;
        std::string tableName;
        std::string fieldName;
    };
    
    std::map<std::string, Variable> variables;
    
public:
    void set(const std::string& name, const Value& value, int line = -1) {
        size_t dotPos = name.find('.');
        size_t parenPos = name.find('(');
        
        if (dotPos != std::string::npos) {
            std::string tableName = name.substr(0, dotPos);
            std::string fieldName = name.substr(dotPos + 1);
            
            if (variables.find(tableName) == variables.end() || 
                !variables[tableName].isTableField) {
                variables[tableName] = {
                    std::make_shared<Table>(),
                    true,
                    tableName,
                    ""
                };
            }
            
            auto table = std::get<std::shared_ptr<Table>>(variables[tableName].value);
            table->set(fieldName, value);
            return;
        }
        
        if (parenPos != std::string::npos && name.back() == ')') {
            std::string tableName = name.substr(0, parenPos);
            std::string fieldName = name.substr(parenPos + 1, name.length() - parenPos - 2);
            
            if (variables.find(tableName) == variables.end() || 
                !variables[tableName].isTableField) {
                variables[tableName] = {
                    std::make_shared<Table>(),
                    true,
                    tableName,
                    ""
                };
            }
            
            auto table = std::get<std::shared_ptr<Table>>(variables[tableName].value);
            table->set(fieldName, value);
            return;
        }
        
        variables[name] = {value, false, "", ""};
    }
    
    Value get(const std::string& name, int line = -1) const {
        size_t dotPos = name.find('.');
        size_t parenPos = name.find('(');
        
        if (dotPos != std::string::npos) {
            std::string tableName = name.substr(0, dotPos);
            std::string fieldName = name.substr(dotPos + 1);
            
            auto it = variables.find(tableName);
            if (it == variables.end() || !it->second.isTableField) {
                throw UndefinedVariable(name, line);
            }
            
            auto table = std::get<std::shared_ptr<Table>>(it->second.value);
            return table->get(fieldName, line);
        }
        
        if (parenPos != std::string::npos && name.back() == ')') {
            std::string tableName = name.substr(0, parenPos);
            std::string fieldName = name.substr(parenPos + 1, name.length() - parenPos - 2);
            
            auto it = variables.find(tableName);
            if (it == variables.end() || !it->second.isTableField) {
                throw UndefinedVariable(name, line);
            }
            
            auto table = std::get<std::shared_ptr<Table>>(it->second.value);
            return table->get(fieldName, line);
        }
        
        auto it = variables.find(name);
        if (it == variables.end()) {
            throw UndefinedVariable(name, line);
        }
        return it->second.value;
    }
    
    bool exists(const std::string& name) const {
        return variables.find(name) != variables.end();
    }
};

// ====================== 分词器模块 ======================
class Tokenizer {
public:
    static std::vector<std::string> tokenize(const std::string& line, int lineNum = -1) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string currentToken;
        
        while (pos < line.length()) {
            char c = line[pos];
            
            if (c == '"') {
                currentToken += c;
                pos++;
                while (pos < line.length() && line[pos] != '"') {
                    if (line[pos] == '\\' && pos + 1 < line.length()) {
                        currentToken += escapeChar(line[pos + 1]);
                        pos += 2;
                    } else {
                        currentToken += line[pos];
                        pos++;
                    }
                }
                if (pos < line.length()) {
                    currentToken += line[pos];
                    pos++;
                }
                tokens.push_back(currentToken);
                currentToken.clear();
                continue;
            }
            
            if (c == '{') {
                int braceDepth = 1;
                currentToken += c;
                pos++;
                
                while (pos < line.length() && braceDepth > 0) {
                    c = line[pos];
                    currentToken += c;
                    
                    if (c == '{') braceDepth++;
                    else if (c == '}') braceDepth--;
                    else if (c == '\\' && pos + 1 < line.length()) {
                        pos++;
                        currentToken += line[pos];
                    }
                    
                    pos++;
                }
                
                tokens.push_back(currentToken);
                currentToken.clear();
                continue;
            }
            
            if (c == '[') {
                int bracketDepth = 1;
                currentToken += c;
                pos++;
                
                while (pos < line.length() && bracketDepth > 0) {
                    c = line[pos];
                    currentToken += c;
                    
                    if (c == '[') bracketDepth++;
                    else if (c == ']') bracketDepth--;
                    else if (c == '\\' && pos + 1 < line.length()) {
                        pos++;
                        currentToken += line[pos];
                    }
                    
                    pos++;
                }
                
                tokens.push_back(currentToken);
                currentToken.clear();
                continue;
            }
            
            if (c == '$') {
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
                
                currentToken += c;
                pos++;
                
                // 处理${var}语法
                if (pos < line.length() && line[pos] == '{') {
                    currentToken += '{';
                    pos++;
                    while (pos < line.length() && line[pos] != '}') {
                        currentToken += line[pos];
                        pos++;
                    }
                    if (pos < line.length()) {
                        currentToken += '}';
                        pos++;
                    }
                    tokens.push_back(currentToken);
                    currentToken.clear();
                    continue;
                }
                
                // 处理$var语法
                while (pos < line.length() && (isalnum(line[pos]) || line[pos] == '_' || 
                       line[pos] == '(' || line[pos] == ')' || line[pos] == '.')) {
                    currentToken += line[pos];
                    pos++;
                }
                
                tokens.push_back(currentToken);
                currentToken.clear();
                continue;
            }
            
            if (isspace(c)) {
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
                pos++;
                continue;
            }
            
            currentToken += c;
            pos++;
        }
        
        if (!currentToken.empty()) {
            tokens.push_back(currentToken);
        }
        
        return tokens;
    }
    
private:
    static char escapeChar(char c) {
        switch (c) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '"': return '"';
            case '\\': return '\\';
            default: return c;
        }
    }
};

// ====================== 表达式解析器 ======================
class ExpressionParser {
private:
    VariableManager& varManager;
    int currentLine = -1;
    
    enum TokenType { NUMBER, OPERATOR, VARIABLE, STRING, BRACE, BRACKET, END };
    
    struct Token {
        TokenType type;
        std::string value;
        double numValue = 0.0;
    };
    
    std::vector<Token> tokens;
    size_t currentToken = 0;
    
    void tokenizeExpression(const std::string& expr) {
        tokens.clear();
        currentToken = 0;
        
        size_t pos = 0;
        while (pos < expr.length()) {
            char c = expr[pos];
            
            if (isspace(c)) {
                pos++;
                continue;
            }
            
            if (isdigit(c) || c == '.') {
                size_t start = pos;
                while (pos < expr.length() && (isdigit(expr[pos]) || expr[pos] == '.' || 
                       expr[pos] == 'e' || expr[pos] == 'E' || expr[pos] == '+' || expr[pos] == '-') {
                    pos++;
                }
                std::string numStr = expr.substr(start, pos - start);
                try {
                    Token token;
                    token.type = NUMBER;
                    token.value = numStr;
                    token.numValue = std::stod(numStr);
                    tokens.push_back(token);
                } catch (...) {
                    throw SyntaxError("Invalid number: " + numStr, currentLine);
                }
                continue;
            }
            
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || 
                c == '(' || c == ')' || c == '=' || c == '<' || c == '>' || c == '!') {
                tokens.push_back({OPERATOR, std::string(1, c)});
                pos++;
                continue;
            }
            
            if (c == '$') {
                size_t start = pos;
                pos++;
                while (pos < expr.length() && (isalnum(expr[pos]) || expr[pos] == '_' || 
                       expr[pos] == '(' || expr[pos] == ')' || expr[pos] == '.')) {
                    pos++;
                }
                tokens.push_back({VARIABLE, expr.substr(start, pos - start)});
                continue;
            }
            
            if (c == '"') {
                size_t start = pos;
                tokens.push_back({STRING, "\""});
                pos++;
                while (pos < expr.length() && expr[pos] != '"') {
                    if (expr[pos] == '\\' && pos + 1 < expr.length()) {
                        pos += 2;
                    } else {
                        pos++;
                    }
                }
                if (pos < expr.length()) {
                    tokens.push_back({STRING, expr.substr(start + 1, pos - start - 1)});
                    tokens.push_back({STRING, "\""});
                    pos++;
                } else {
                    tokens.push_back({STRING, expr.substr(start + 1)});
                }
                continue;
            }
            
            if (c == '{') {
                size_t start = pos;
                int depth = 1;
                pos++;
                while (pos < expr.length() && depth > 0) {
                    if (expr[pos] == '{') depth++;
                    else if (expr[pos] == '}') depth--;
                    pos++;
                }
                tokens.push_back({BRACE, expr.substr(start, pos - start)});
                continue;
            }
            
            if (c == '[') {
                size_t start = pos;
                int depth = 1;
                pos++;
                while (pos < expr.length() && depth > 0) {
                    if (expr[pos] == '[') depth++;
                    else if (expr[pos] == ']') depth--;
                    pos++;
                }
                tokens.push_back({BRACKET, expr.substr(start, pos - start)});
                continue;
            }
            
            // 默认处理为字符串
            size_t start = pos;
            while (pos < expr.length() && !isspace(c) && !strchr("+-*/^()=<>!$\"{}[]", expr[pos])) {
                pos++;
            }
            tokens.push_back({STRING, expr.substr(start, pos - start)});
        }
        
        tokens.push_back({END, ""});
    }
    
    const Token& peek() const {
        return tokens[currentToken];
    }
    
    Token consume() {
        return tokens[currentToken++];
    }
    
    Value parsePrimary() {
        Token token = consume();
        
        switch (token.type) {
            case NUMBER:
                return token.numValue;
            case VARIABLE:
                return varManager.get(token.value, currentLine);
            case STRING:
                return token.value;
            case BRACE:
                return token.value.substr(1, token.value.size() - 2);
            case BRACKET: {
                std::string inner = token.value.substr(1, token.value.size() - 2);
                // 这里需要访问解释器来执行命令
                throw RuntimeError("Command substitution not implemented in expression", currentLine);
            }
            case OPERATOR:
                if (token.value == "(") {
                    Value result = parseExpression();
                    if (peek().type != OPERATOR || peek().value != ")") {
                        throw SyntaxError("Expected ')'", currentLine);
                    }
                    consume();
                    return result;
                }
                if (token.value == "-") {
                    Value operand = parsePrimary();
                    if (auto* num = std::get_if<double>(&operand)) {
                        return -(*num);
                    }
                    throw RuntimeError("Unary '-' requires a number", currentLine);
                }
            default:
                throw SyntaxError("Unexpected token: " + token.value, currentLine);
        }
    }
    
    Value parsePower() {
        Value left = parsePrimary();
        
        while (peek().type == OPERATOR && peek().value == "^") {
            consume();
            Value right = parsePrimary();
            
            if (auto* lnum = std::get_if<double>(&left)) {
                if (auto* rnum = std::get_if<double>(&right)) {
                    left = std::pow(*lnum, *rnum);
                } else {
                    throw RuntimeError("Exponent must be a number", currentLine);
                }
            } else {
                throw RuntimeError("Base must be a number", currentLine);
            }
        }
        
        return left;
    }
    
    Value parseTerm() {
        Value left = parsePower();
        
        while (peek().type == OPERATOR && (peek().value == "*" || peek().value == "/")) {
            std::string op = consume().value;
            Value right = parsePower();
            
            if (auto* lnum = std::get_if<double>(&left)) {
                if (auto* rnum = std::get_if<double>(&right)) {
                    if (op == "*") {
                        left = (*lnum) * (*rnum);
                    } else {
                        if (*rnum == 0) throw RuntimeError("Division by zero", currentLine);
                        left = (*lnum) / (*rnum);
                    }
                } else {
                    throw RuntimeError("Operand must be a number", currentLine);
                }
            } else {
                throw RuntimeError("Operand must be a number", currentLine);
            }
        }
        
        return left;
    }
    
    Value parseExpression() {
        Value left = parseTerm();
        
        while (peek().type == OPERATOR && (peek().value == "+" || peek().value == "-")) {
            std::string op = consume().value;
            Value right = parseTerm();
            
            if (auto* lnum = std::get_if<double>(&left)) {
                if (auto* rnum = std::get_if<double>(&right)) {
                    if (op == "+") {
                        left = (*lnum) + (*rnum);
                    } else {
                        left = (*lnum) - (*rnum);
                    }
                } else {
                    throw RuntimeError("Operand must be a number", currentLine);
                }
            } else if (auto* lstr = std::get_if<std::string>(&left)) {
                if (op == "+") {
                    std::string rstr = valueToString(right);
                    left = *lstr + rstr;
                } else {
                    throw RuntimeError("Cannot subtract from a string", currentLine);
                }
            } else {
                throw RuntimeError("Unsupported operand type", currentLine);
            }
        }
        
        return left;
    }
    
public:
    ExpressionParser(VariableManager& vm) : varManager(vm) {}
    
    Value evaluate(const std::string& expr, int line = -1) {
        currentLine = line;
        try {
            tokenizeExpression(expr);
            return parseExpression();
        } catch (const InterpreterException&) {
            throw;
        } catch (const std::exception& e) {
            throw RuntimeError(e.what(), line);
        }
    }
    
    static std::string valueToString(const Value& value) {
        if (auto* num = std::get_if<double>(&value)) {
            if (*num == static_cast<int>(*num)) {
                return std::to_string(static_cast<int>(*num));
            }
            return std::to_string(*num);
        }
        if (auto* str = std::get_if<std::string>(&value)) {
            return *str;
        }
        if (auto* b = std::get_if<bool>(&value)) {
            return *b ? "1" : "0";
        }
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return "";
        }
        if (auto* t = std::get_if<std::shared_ptr<Table>>(&value)) {
            return "table";
        }
        return "unknown";
    }
};

// ====================== 命令处理器 ======================
class CommandHandler {
private:
    struct Procedure {
        std::vector<std::string> parameters;
        std::string body;
    };
    
    struct DebugInfo {
        bool breakpointsEnabled = false;
        std::set<int> breakpoints;
        bool stepMode = false;
    };
    
    VariableManager& varManager;
    ExpressionParser& exprParser;
    DebugInfo debugInfo;
    int currentLine = -1;
    
    std::map<std::string, Procedure> procedures;
    std::map<std::string, std::shared_ptr<Table>> classes;
    
public:
    CommandHandler(VariableManager& vm, ExpressionParser& ep) 
        : varManager(vm), exprParser(ep) {}
    
    void setLineNumber(int line) { currentLine = line; }
    
    Value executeCommand(const std::string& cmd, const std::vector<std::string>& args) {
        // 调试检查点
        if (debugInfo.breakpointsEnabled) {
            if (debugInfo.breakpoints.find(currentLine) != debugInfo.breakpoints.end() ||
                debugInfo.stepMode) {
                std::cout << "Breakpoint at line " << currentLine << ": " << cmd;
                for (const auto& arg : args) std::cout << " " << arg;
                std::cout << "\n> ";
                
                std::string debugCmd;
                std::getline(std::cin, debugCmd);
                handleDebugCommand(debugCmd);
            }
        }
        
        if (cmd == "set") {
            return handleSet(args);
        } else if (cmd == "expr") {
            return handleExpr(args);
        } else if (cmd == "puts") {
            return handlePuts(args);
        } else if (cmd == "proc") {
            return handleProc(args);
        } else if (cmd == "if") {
            return handleIf(args);
        } else if (cmd == "for") {
            return handleFor(args);
        } else if (cmd == "incr") {
            return handleIncr(args);
        } else if (cmd == "return") {
            return handleReturn(args);
        } else if (cmd == "string") {
            return handleString(args);
        } else if (cmd == "while") {
            return handleWhile(args);
        } else if (cmd == "switch") {
            return handleSwitch(args);
        } else if (cmd == "class") {
            return handleClass(args);
        } else if (cmd == "new") {
            return handleNew(args);
        } else if (cmd == "setmetatable") {
            return handleSetMetatable(args);
        } else if (cmd == "try") {
            return handleTry(args);
        } else if (cmd == "table") {
            return handleTable(args);
        } else if (cmd == "breakpoint") {
            return handleBreakpoint(args);
        } else if (cmd == "step") {
            return handleStep(args);
        } else if (procedures.find(cmd) != procedures.end()) {
            return executeProcedure(cmd, args);
        }
        
        throw RuntimeError("Unknown command: " + cmd, currentLine);
    }

private:
    void handleDebugCommand(const std::string& cmd) {
        if (cmd == "c" || cmd == "continue") {
            debugInfo.stepMode = false;
        } else if (cmd == "s" || cmd == "step") {
            debugInfo.stepMode = true;
        } else if (cmd == "bt" || cmd == "backtrace") {
            // 显示调用栈（简化版）
            std::cout << "Backtrace not implemented\n";
        } else if (cmd == "v" || cmd == "vars") {
            // 显示变量（简化版）
            std::cout << "Variables display not implemented\n";
        } else {
            std::cout << "Unknown debug command. Available: c(continue), s(tep), bt(backtrace), v(ars)\n";
        }
    }
    
    Value handleSet(const std::vector<std::string>& args) {
        if (args.size() < 2) throw RuntimeError("set: missing arguments", currentLine);
        varManager.set(args[0], exprParser.evaluate(args[1], currentLine), currentLine);
        return 0.0;
    }
    
    Value handleExpr(const std::vector<std::string>& args) {
        if (args.empty()) throw RuntimeError("expr: missing expression", currentLine);
        return exprParser.evaluate(args[0], currentLine);
    }
    
    Value handlePuts(const std::vector<std::string>& args) {
        for (const auto& arg : args) {
            std::cout << exprParser.valueToString(exprParser.evaluate(arg, currentLine)) << std::endl;
        }
        return 0.0;
    }
    
    // 其他命令处理函数类似，限于篇幅省略...
    // 实际实现中需要完整实现每个命令的处理逻辑
    
    Value executeProcedure(const std::string& name, const std::vector<std::string>& args) {
        auto& proc = procedures[name];
        if (args.size() != proc.parameters.size()) {
            throw RuntimeError("Wrong number of arguments for procedure " + name, currentLine);
        }
        
        // 保存当前作用域
        auto backup = varManager;
        
        // 设置参数
        for (size_t i = 0; i < proc.parameters.size(); i++) {
            varManager.set(proc.parameters[i], exprParser.evaluate(args[i], currentLine), currentLine);
        }
        
        // 执行过程体
        try {
            // 这里需要访问解释器执行过程体
            // executeBody(proc.body);
        } catch (const Value& returnValue) {
            varManager = backup;
            return returnValue;
        }
        
        // 恢复作用域
        varManager = backup;
        return 0.0;
    }
    
    Value handleBreakpoint(const std::vector<std::string>& args) {
        if (args.empty()) {
            debugInfo.breakpointsEnabled = !debugInfo.breakpointsEnabled;
            std::cout << "Breakpoints " << (debugInfo.breakpointsEnabled ? "enabled" : "disabled") << "\n";
            return 0.0;
        }
        
        if (args[0] == "add") {
            if (args.size() < 2) throw RuntimeError("breakpoint add: missing line number", currentLine);
            try {
                int line = std::stoi(args[1]);
                debugInfo.breakpoints.insert(line);
                std::cout << "Breakpoint added at line " << line << "\n";
            } catch (...) {
                throw RuntimeError("Invalid line number: " + args[1], currentLine);
            }
        } else if (args[0] == "remove") {
            if (args.size() < 2) throw RuntimeError("breakpoint remove: missing line number", currentLine);
            try {
                int line = std::stoi(args[1]);
                debugInfo.breakpoints.erase(line);
                std::cout << "Breakpoint removed at line " << line << "\n";
            } catch (...) {
                throw RuntimeError("Invalid line number: " + args[1], currentLine);
            }
        } else if (args[0] == "list") {
            std::cout << "Breakpoints: ";
            for (int line : debugInfo.breakpoints) {
                std::cout << line << " ";
            }
            std::cout << "\n";
        } else {
            throw RuntimeError("Unknown breakpoint subcommand: " + args[0], currentLine);
        }
        
        return 0.0;
    }
    
    Value handleStep(const std::vector<std::string>& args) {
        debugInfo.stepMode = true;
        std::cout << "Stepping enabled\n";
        return 0.0;
    }
};

// ====================== 解释器核心 ======================
class LuaInterpreter {
private:
    VariableManager varManager;
    ExpressionParser exprParser;
    CommandHandler cmdHandler;
    int currentLine = 0;
    
public:
    LuaInterpreter() 
        : exprParser(varManager), cmdHandler(varManager, exprParser) {}
    
    void execute(const std::string& script) {
        std::istringstream iss(script);
        std::string line;
        currentLine = 0;
        
        while (std::getline(iss, line)) {
            currentLine++;
            try {
                execute_line(line);
            } catch (const InterpreterException& e) {
                std::cerr << "Error: " << e.fullMessage() << std::endl;
                if (e.line == -1) std::cerr << "  At line: " << currentLine << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error at line " << currentLine << ": " << e.what() << std::endl;
            }
        }
    }
    
    Value execute_line(const std::string& line) {
        auto tokens = Tokenizer::tokenize(line, currentLine);
        if (tokens.empty()) return 0.0;
        
        cmdHandler.setLineNumber(currentLine);
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        return cmdHandler.executeCommand(tokens[0], args);
    }
};

// ====================== 示例用法 ======================
int main() {
    LuaInterpreter interpreter;
    
    // 示例脚本
    std::string script = R"(
        set x 10
        set y 20
        puts "Sum: [expr $x + $y]"
        
        # 使用表
        set person {name "John" age 30}
        puts "Name: $person.name, Age: $person(age)"
        
        # 使用过程
        proc add {a b} {
            return [expr $a + $b]
        }
        puts "5 + 7 = [add 5 7]"
        
        # 调试功能
        breakpoint add 10
        breakpoint enable
        puts "This line has a breakpoint"
    )";
    
    interpreter.execute(script);
    
    return 0;
}