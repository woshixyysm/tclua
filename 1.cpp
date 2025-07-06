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
#include <list>
#include <algorithm>
#include <typeinfo>

// 值类型（添加了bool和Table支持）
using Value = std::variant<double, std::string, bool, std::nullptr_t, std::shared_ptr<class Table>>;

// 表结构（用于数组/字典）
class Table : public std::enable_shared_from_this<Table> {
public:
    std::map<std::string, Value> fields;
    std::shared_ptr<Table> metatable; // 元表支持
    
    // 检查字段是否存在
    bool contains(const std::string& key) const {
        return fields.find(key) != fields.end();
    }
    
    // 获取字段值（支持元表查找）
    Value get(const std::string& key) const {
        auto it = fields.find(key);
        if (it != fields.end()) return it->second;
        
        // 检查元表
        if (metatable) {
            try {
                if (metatable->contains("__index")) {
                    Value indexVal = metatable->get("__index");
                    
                    // 如果__index是表，则递归查找
                    if (std::holds_alternative<std::shared_ptr<Table>>(indexVal)) {
                        auto indexTable = std::get<std::shared_ptr<Table>>(indexVal);
                        return indexTable->get(key);
                    }
                    // 如果__index是过程，则调用它
                    else if (std::holds_alternative<std::shared_ptr<Table>>(indexVal)) {
                        // 这里需要解释器支持，稍后处理
                    }
                }
            } catch (...) {
                // 忽略元表访问错误
            }
        }
        
        throw std::runtime_error("Key not found: " + key);
    }
    
    // 设置字段值
    void set(const std::string& key, const Value& value) {
        fields[key] = value;
    }
    
    // 获取所有键
    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        for (const auto& pair : fields) {
            result.push_back(pair.first);
        }
        return result;
    }
    
    // 获取所有值
    std::vector<Value> values() const {
        std::vector<Value> result;
        for (const auto& pair : fields) {
            result.push_back(pair.second);
        }
        return result;
    }
    
    // 获取键值对
    std::vector<std::pair<std::string, Value>> items() const {
        std::vector<std::pair<std::string, Value>> result;
        for (const auto& pair : fields) {
            result.push_back(pair);
        }
        return result;
    }
    
    // 设置默认值（通过元表）
    void setDefault(const Value& defaultValue) {
        if (!metatable) {
            metatable = std::make_shared<Table>();
        }
        metatable->set("__default", defaultValue);
    }
};

// 过程（函数）定义
struct Procedure {
    std::vector<std::string> parameters;
    std::string body;
};

// 解释器核心类
class TCLuaInterpreter {
public:
    TCLuaInterpreter() {
        // 初始化内置命令
        commands["set"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) throw std::runtime_error("set: missing arguments");
            setVariable(args[0], evaluateExpression(args[1]));
            return Value(0.0);
        };
        
        commands["expr"] = [this](const std::vector<std::string>& args) {
            if (args.empty()) throw std::runtime_error("expr: missing expression");
            return evaluateArithmeticExpression(args[0]);
        };
        
        commands["puts"] = [this](const std::vector<std::string>& args) {
            for (const auto& arg : args) {
                std::cout << valueToString(evaluateExpression(arg)) << std::endl;
            }
            return Value(0.0);
        };
        
        commands["proc"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 3) throw std::runtime_error("proc: missing arguments");
            defineProcedure(args[0], args[1], args[2]);
            return Value(0.0);
        };
        
        commands["if"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) throw std::runtime_error("if: missing arguments");
            return executeIfCommand(args);
        };
        
        commands["for"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 4) throw std::runtime_error("for: missing arguments");
            return executeForLoop(args);
        };
        
        commands["incr"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 1) throw std::runtime_error("incr: missing arguments");
            return executeIncrCommand(args);
        };
        
        commands["return"] = [this](const std::vector<std::string>& args) {
            if (!args.empty()) {
                throw evaluateExpression(args[0]); // 使用异常实现返回
            }
            throw Value(0.0);
        };
        
        commands["string"] = [this](const std::vector<std::string>& args) {
            if (args.empty()) throw std::runtime_error("string: missing subcommand");
            return executeStringCommand(args);
        };
        
        // 新增命令
        commands["while"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) throw std::runtime_error("while: missing arguments");
            return executeWhileLoop(args);
        };
        
        commands["switch"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 1) throw std::runtime_error("switch: missing arguments");
            return executeSwitchCommand(args);
        };
        
        commands["class"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 1) throw std::runtime_error("class: missing class name");
            return defineClass(args);
        };
        
        commands["new"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 1) throw std::runtime_error("new: missing class name");
            return createInstance(args[0]);
        };
        
        commands["setmetatable"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 2) throw std::runtime_error("setmetatable: missing arguments");
            return setMetatable(args[0], args[1]);
        };
        
        commands["try"] = [this](const std::vector<std::string>& args) {
            if (args.size() < 3) throw std::runtime_error("try: missing arguments");
            return executeTryCatch(args);
        };
        
        commands["table"] = [this](const std::vector<std::string>& args) {
            if (args.empty()) throw std::runtime_error("table: missing subcommand");
            return executeTableCommand(args);
        };
    }
    
    // 执行脚本
    void execute(const std::string& script) {
        size_t pos = 0;
        while (pos < script.length()) {
            auto line = extract_line(script, pos);
            if (!line.empty()) {
                execute_line(line);
            }
        }
    }
    
    // 执行单行命令
    Value execute_line(const std::string& line) {
        auto tokens = tokenize(line);
        if (tokens.empty()) return Value(0.0);
        
        // 检查是否是内置命令
        if (commands.find(tokens[0]) != commands.end()) {
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());
            try {
                return commands[tokens[0]](args);
            } catch (const Value& returnValue) {
                return returnValue; // 捕获return语句
            } catch (const std::exception& e) {
                throw std::runtime_error(std::string("Error in command '") + tokens[0] + "': " + e.what());
            }
        }
        
        // 否则作为表达式求值
        return evaluateExpression(line);
    }
    
private:
    std::map<std::string, Value> variables;
    std::map<std::string, std::function<Value(const std::vector<std::string>&)>> commands;
    std::map<std::string, Procedure> procedures;
    std::map<std::string, std::shared_ptr<Table>> classes; // 类定义存储
    
    // 提取一行
    std::string extract_line(const std::string& script, size_t& pos) {
        size_t start = pos;
        while (pos < script.length() && script[pos] != '\n') {
            pos++;
        }
        std::string line = script.substr(start, pos - start);
        if (pos < script.length()) pos++; // 跳过换行符
        return trim(line);
    }
    
    // 去除首尾空白
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
    
    // 高级分词（支持引号、大括号、变量替换和命令替换）
    std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        bool inQuotes = false;
        bool inBraces = false;
        int braceDepth = 0;
        std::string currentToken;
        
        while (pos < line.length()) {
            char c = line[pos];
            
            if (c == '"' && !inBraces) {
                inQuotes = !inQuotes;
                currentToken += c;
                pos++;
                continue;
            }
            
            if (c == '{' && !inQuotes) {
                if (inBraces) {
                    braceDepth++;
                    currentToken += c;
                } else {
                    if (!currentToken.empty()) {
                        tokens.push_back(currentToken);
                        currentToken.clear();
                    }
                    inBraces = true;
                    braceDepth = 1;
                    currentToken += c;
                }
                pos++;
                continue;
            }
            
            if (c == '}' && inBraces && !inQuotes) {
                braceDepth--;
                currentToken += c;
                if (braceDepth == 0) {
                    inBraces = false;
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
                pos++;
                continue;
            }
            
            if (c == '[' && !inQuotes && !inBraces) {
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
                size_t end = line.find(']', pos);
                if (end == std::string::npos) {
                    throw std::runtime_error("Unmatched '['");
                }
                tokens.push_back(line.substr(pos, end - pos + 1));
                pos = end + 1;
                continue;
            }
            
            if (c == '$' && !inQuotes) {
                if (!currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                }
                size_t start = pos++;
                while (pos < line.length() && (isalnum(line[pos]) || line[pos] == '_' || 
                       line[pos] == '(' || line[pos] == ')' || line[pos] == '.')) {
                    pos++;
                }
                tokens.push_back(line.substr(start, pos - start));
                continue;
            }
            
            if (isspace(c) && !inQuotes && !inBraces) {
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
    
    // 表达式求值（支持字符串拼接）
    Value evaluateExpression(const std::string& expr) {
        // 处理命令替换 [command]
        if (expr.size() > 2 && expr[0] == '[' && expr.back() == ']') {
            return execute_line(expr.substr(1, expr.size() - 2));
        }
        
        // 处理大括号字符串 {}
        if (expr.size() > 1 && expr[0] == '{' && expr.back() == '}') {
            return expr.substr(1, expr.size() - 2);
        }
        
        // 处理变量替换 $var
        if (!expr.empty() && expr[0] == '$') {
            return getVariable(expr.substr(1));
        }
        
        // 处理字符串拼接
        std::string result;
        size_t pos = 0;
        while (pos < expr.length()) {
            if (expr[pos] == '$') {
                // 变量替换
                size_t start = pos++;
                while (pos < expr.length() && (isalnum(expr[pos]) || expr[pos] == '_' || 
                       expr[pos] == '(' || expr[pos] == ')' || expr[pos] == '.')) {
                    pos++;
                }
                std::string var = expr.substr(start + 1, pos - start - 1);
                Value val = getVariable(var);
                result += valueToString(val);
            } else {
                result += expr[pos++];
            }
        }
        
        // 尝试转换为数字
        try {
            return std::stod(result);
        } catch (...) {
            return result;
        }
    }
    
    // 算术表达式求值
    Value evaluateArithmeticExpression(const std::string& expr) {
        std::string processed = expr;
        size_t pos = 0;
        
        // 替换变量
        while ((pos = processed.find('$', pos)) != std::string::npos) {
            size_t start = pos++;
            while (pos < processed.length() && (isalnum(processed[pos]) || processed[pos] == '_' || 
                   processed[pos] == '(' || processed[pos] == ')' || processed[pos] == '.')) {
                pos++;
            }
            std::string var = processed.substr(start + 1, pos - start - 1);
            Value val = getVariable(var);
            std::string strVal = valueToString(val);
            processed.replace(start, pos - start, strVal);
            pos = start + strVal.length();
        }
        
        // 执行计算
        std::istringstream iss(processed);
        double left, right;
        char op;
        std::string token;
        
        if (iss >> left) {
            if (iss >> op) {
                if (iss >> right) {
                    switch (op) {
                        case '+': return left + right;
                        case '-': return left - right;
                        case '*': return left * right;
                        case '/': 
                            if (right == 0) throw std::runtime_error("Division by zero");
                            return left / right;
                        case '^': return std::pow(left, right);
                        default: throw std::runtime_error("Unknown operator: " + std::string(1, op));
                    }
                }
                throw std::runtime_error("Missing right operand");
            }
            return left;
        }
        
        // 无法解析为算术表达式，返回原值
        return evaluateExpression(expr);
    }
    
    // 设置变量（支持数组/表）
    void setVariable(const std::string& name, const Value& value) {
        // 检查是否为表元素
        size_t dotPos = name.find('.');
        if (dotPos != std::string::npos) {
            std::string tableName = name.substr(0, dotPos);
            std::string fieldName = name.substr(dotPos + 1);
            
            // 获取或创建表
            std::shared_ptr<Table> table;
            if (variables.find(tableName) != variables.end() && 
                std::holds_alternative<std::shared_ptr<Table>>(variables[tableName])) {
                table = std::get<std::shared_ptr<Table>>(variables[tableName]);
            } else {
                table = std::make_shared<Table>();
                variables[tableName] = table;
            }
            
            table->set(fieldName, value);
            return;
        }
        
        // 检查是否为数组元素
        size_t parenPos = name.find('(');
        if (parenPos != std::string::npos && name.back() == ')') {
            std::string tableName = name.substr(0, parenPos);
            std::string fieldName = name.substr(parenPos + 1, name.length() - parenPos - 2);
            
            // 获取或创建表
            std::shared_ptr<Table> table;
            if (variables.find(tableName) != variables.end() && 
                std::holds_alternative<std::shared_ptr<Table>>(variables[tableName])) {
                table = std::get<std::shared_ptr<Table>>(variables[tableName]);
            } else {
                table = std::make_shared<Table>();
                variables[tableName] = table;
            }
            
            table->set(fieldName, value);
            return;
        }
        
        // 普通变量
        variables[name] = value;
    }
    
    // 获取变量（支持数组/表）
    Value getVariable(const std::string& name) {
        // 检查是否为表元素
        size_t dotPos = name.find('.');
        if (dotPos != std::string::npos) {
            std::string tableName = name.substr(0, dotPos);
            std::string fieldName = name.substr(dotPos + 1);
            
            if (variables.find(tableName) != variables.end() && 
                std::holds_alternative<std::shared_ptr<Table>>(variables[tableName])) {
                auto table = std::get<std::shared_ptr<Table>>(variables[tableName]);
                if (table->contains(fieldName)) {
                    return table->get(fieldName);
                }
            }
            throw std::runtime_error("Undefined variable: " + name);
        }
        
        // 检查是否为数组元素
        size_t parenPos = name.find('(');
        if (parenPos != std::string::npos && name.back() == ')') {
            std::string tableName = name.substr(0, parenPos);
            std::string fieldName = name.substr(parenPos + 1, name.length() - parenPos - 2);
            
            if (variables.find(tableName) != variables.end() && 
                std::holds_alternative<std::shared_ptr<Table>>(variables[tableName])) {
                auto table = std::get<std::shared_ptr<Table>>(variables[tableName]);
                if (table->contains(fieldName)) {
                    return table->get(fieldName);
                }
            }
            throw std::runtime_error("Undefined variable: " + name);
        }
        
        // 普通变量
        if (variables.find(name) != variables.end()) {
            return variables[name];
        }
        throw std::runtime_error("Undefined variable: " + name);
    }
    
    // 值转字符串
    std::string valueToString(const Value& value) {
        if (std::holds_alternative<double>(value)) {
            double num = std::get<double>(value);
            if (num == static_cast<int>(num)) {
                return std::to_string(static_cast<int>(num));
            }
            return std::to_string(num);
        }
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? "1" : "0";
        }
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return "";
        }
        if (std::holds_alternative<std::shared_ptr<Table>>(value)) {
            return "table";
        }
        return "unknown";
    }
    
    // 定义过程
    void defineProcedure(const std::string& name, const std::string& paramsStr, const std::string& body) {
        // 解析参数
        std::vector<std::string> params;
        std::istringstream iss(paramsStr);
        std::string param;
        while (iss >> param) {
            params.push_back(param);
        }
        
        procedures[name] = {params, body};
        
        // 注册为命令
        commands[name] = [this, name](const std::vector<std::string>& args) {
            auto& proc = procedures[name];
            if (args.size() != proc.parameters.size()) {
                throw std::runtime_error("Wrong number of arguments for procedure " + name);
            }
            
            // 保存当前作用域
            auto backup = variables;
            
            // 设置参数
            for (size_t i = 0; i < proc.parameters.size(); i++) {
                setVariable(proc.parameters[i], evaluateExpression(args[i]));
            }
            
            // 执行过程体
            try {
                execute(proc.body);
            } catch (const Value& returnValue) {
                variables = backup;
                return returnValue;
            }
            
            // 恢复作用域
            variables = backup;
            return Value(0.0);
        };
    }
    
    // 执行if命令
    Value executeIfCommand(const std::vector<std::string>& args) {
        // 简单实现：if {condition} {body}
        if (args.size() < 2) return Value(0.0);
        
        std::string condition = args[0];
        std::string body = args[1];
        
        Value condValue = evaluateExpression(condition);
        bool conditionMet = false;
        
        if (std::holds_alternative<double>(condValue)) {
            conditionMet = (std::get<double>(condValue) != 0.0);
        } else if (std::holds_alternative<std::string>(condValue)) {
            conditionMet = !std::get<std::string>(condValue).empty();
        } else if (std::holds_alternative<bool>(condValue)) {
            conditionMet = std::get<bool>(condValue);
        }
        
        if (conditionMet) {
            execute(body);
        }
        return Value(0.0);
    }
    
    // 执行for循环
    Value executeForLoop(const std::vector<std::string>& args) {
        // 格式：for {init} {condition} {increment} {body}
        if (args.size() < 4) return Value(0.0);
        
        std::string init = args[0];
        std::string condition = args[1];
        std::string increment = args[2];
        std::string body = args[3];
        
        // 执行初始化
        execute_line(init);
        
        while (true) {
            // 检查条件
            Value condValue = evaluateExpression(condition);
            bool conditionMet = false;
            
            if (std::holds_alternative<double>(condValue)) {
                conditionMet = (std::get<double>(condValue) != 0.0);
            } else if (std::holds_alternative<std::string>(condValue)) {
                conditionMet = !std::get<std::string>(condValue).empty();
            } else if (std::holds_alternative<bool>(condValue)) {
                conditionMet = std::get<bool>(condValue);
            }
            
            if (!conditionMet) break;
            
            // 执行循环体
            execute(body);
            
            // 执行增量
            execute_line(increment);
        }
        
        return Value(0.0);
    }
    
    // 执行incr命令
    Value executeIncrCommand(const std::vector<std::string>& args) {
        if (args.empty()) return Value(0.0);
        
        std::string varName = args[0];
        double increment = 1.0;
        
        if (args.size() > 1) {
            try {
                increment = std::stod(args[1]);
            } catch (...) {
                throw std::runtime_error("Invalid increment value");
            }
        }
        
        Value currentValue = getVariable(varName);
        if (!std::holds_alternative<double>(currentValue)) {
            throw std::runtime_error("Variable is not a number: " + varName);
        }
        
        double newValue = std::get<double>(currentValue) + increment;
        setVariable(varName, newValue);
        return newValue;
    }
    
    // 执行string命令
    Value executeStringCommand(const std::vector<std::string>& args) {
        if (args[0] == "map") {
            if (args.size() < 4) throw std::runtime_error("string map: missing arguments");
            
            // 格式：string map {charMap} {string}
            std::string charMap = valueToString(evaluateExpression(args[1]));
            std::string input = valueToString(evaluateExpression(args[2]));
            
            // 简单实现：替换字符映射
            for (size_t i = 0; i < charMap.length(); i += 2) {
                if (i + 1 >= charMap.length()) break;
                char from = charMap[i];
                char to = charMap[i+1];
                
                size_t pos = 0;
                while ((pos = input.find(from, pos)) != std::string::npos) {
                    input.replace(pos, 1, 1, to);
                    pos++;
                }
            }
            return input;
        }
        
        throw std::runtime_error("Unknown string subcommand: " + args[0]);
    }
    
    // 执行while循环
    Value executeWhileLoop(const std::vector<std::string>& args) {
        if (args.size() < 2) return Value(0.0);
        
        std::string condition = args[0];
        std::string body = args[1];
        
        while (true) {
            Value condValue = evaluateExpression(condition);
            bool conditionMet = false;
            
            if (std::holds_alternative<double>(condValue)) {
                conditionMet = (std::get<double>(condValue) != 0.0);
            } else if (std::holds_alternative<std::string>(condValue)) {
                conditionMet = !std::get<std::string>(condValue).empty();
            } else if (std::holds_alternative<bool>(condValue)) {
                conditionMet = std::get<bool>(condValue);
            }
            
            if (!conditionMet) break;
            
            execute(body);
        }
        
        return Value(0.0);
    }
    
    // 执行switch命令
    Value executeSwitchCommand(const std::vector<std::string>& args) {
        if (args.empty()) return Value(0.0);
        
        // 获取要匹配的值
        Value switchValue = evaluateExpression(args[0]);
        std::string switchValueStr = valueToString(switchValue);
        
        // 处理匹配分支
        bool matched = false;
        for (size_t i = 1; i < args.size(); i++) {
            if (i + 1 >= args.size()) break; // 确保有模式和代码块
            
            // 检查模式
            std::string pattern = valueToString(evaluateExpression(args[i]));
            i++;
            
            // 检查是否为默认分支
            if (pattern == "default") {
                execute(args[i]);
                matched = true;
                break;
            }
            
            // 匹配模式
            if (pattern == switchValueStr) {
                execute(args[i]);
                matched = true;
                break;
            }
        }
        
        if (!matched) {
            throw std::runtime_error("No matching case in switch statement");
        }
        
        return Value(0.0);
    }
    
    // 定义类
    Value defineClass(const std::vector<std::string>& args) {
        std::string className = args[0];
        auto classTable = std::make_shared<Table>();
        
        // 如果有类体，执行类体
        if (args.size() > 1) {
            std::string body = args[1];
            // 在类作用域中执行
            auto backup = variables;
            variables.clear();
            variables["self"] = classTable;
            
            try {
                execute(body);
            } catch (...) {
                variables = backup;
                throw;
            }
            
            variables = backup;
        }
        
        classes[className] = classTable;
        return Value(0.0);
    }
    
    // 创建类的实例
    Value createInstance(const std::string& className) {
        if (classes.find(className) == classes.end()) {
            throw std::runtime_error("Class not defined: " + className);
        }
        
        auto instance = std::make_shared<Table>();
        instance->metatable = classes[className]; // 设置元表为类定义
        return instance;
    }
    
    // 设置元表
    Value setMetatable(const std::string& tableName, const std::string& metaName) {
        // 获取表
        Value tableValue = getVariable(tableName);
        if (!std::holds_alternative<std::shared_ptr<Table>>(tableValue)) {
            throw std::runtime_error("setmetatable: first argument must be a table");
        }
        auto table = std::get<std::shared_ptr<Table>>(tableValue);
        
        // 获取元表
        Value metaValue = getVariable(metaName);
        if (!std::holds_alternative<std::shared_ptr<Table>>(metaValue)) {
            throw std::runtime_error("setmetatable: second argument must be a table");
        }
        auto metaTable = std::get<std::shared_ptr<Table>>(metaValue);
        
        table->metatable = metaTable;
        return tableValue;
    }
    
    // 执行try-catch结构
    Value executeTryCatch(const std::vector<std::string>& args) {
        if (args.size() < 3) return Value(0.0);
        
        std::string tryBody = args[0];
        std::string catchKeyword = args[1];
        std::string errorVar = args[2];
        std::string catchBody = args[3];
        
        if (catchKeyword != "catch") {
            throw std::runtime_error("try: expected 'catch' keyword");
        }
        
        try {
            execute(tryBody);
        } catch (const Value& errorValue) {
            auto backup = variables;
            setVariable(errorVar, errorValue);
            execute(catchBody);
            variables = backup;
        } catch (const std::exception& e) {
            auto backup = variables;
            setVariable(errorVar, std::string(e.what()));
            execute(catchBody);
            variables = backup;
        } catch (...) {
            auto backup = variables;
            setVariable(errorVar, std::string("Unknown error"));
            execute(catchBody);
            variables = backup;
        }
        
        return Value(0.0);
    }
    
    // 执行table命令
    Value executeTableCommand(const std::vector<std::string>& args) {
        if (args[0] == "keys") {
            if (args.size() < 2) throw std::runtime_error("table keys: missing table");
            Value tableValue = evaluateExpression(args[1]);
            if (!std::holds_alternative<std::shared_ptr<Table>>(tableValue)) {
                throw std::runtime_error("table keys: argument is not a table");
            }
            
            auto table = std::get<std::shared_ptr<Table>>(tableValue);
            auto keysTable = std::make_shared<Table>();
            int index = 0;
            for (const auto& key : table->keys()) {
                keysTable->set(std::to_string(index++), key);
            }
            return keysTable;
        }
        else if (args[0] == "values") {
            if (args.size() < 2) throw std::runtime_error("table values: missing table");
            Value tableValue = evaluateExpression(args[1]);
            if (!std::holds_alternative<std::shared_ptr<Table>>(tableValue)) {
                throw std::runtime_error("table values: argument is not a table");
            }
            
            auto table = std::get<std::shared_ptr<Table>>(tableValue);
            auto valuesTable = std::make_shared<Table>();
            int index = 0;
            for (const auto& value : table->values()) {
                valuesTable->set(std::to_string(index++), value);
            }
            return valuesTable;
        }
        else if (args[0] == "setdefault") {
            if (args.size() < 3) throw std::runtime_error("table setdefault: missing arguments");
            Value tableValue = evaluateExpression(args[1]);
            Value defaultValue = evaluateExpression(args[2]);
            
            if (!std::holds_alternative<std::shared_ptr<Table>>(tableValue)) {
                throw std::runtime_error("table setdefault: first argument is not a table");
            }
            
            auto table = std::get<std::shared_ptr<Table>>(tableValue);
            table->setDefault(defaultValue);
            return Value(0.0);
        }
        
        throw std::runtime_error("Unknown table subcommand: " + args[0]);
    }
};

// 示例用法
int main() {
    TCLuaInterpreter interpreter;
    
    // 定义元表
    interpreter.execute(R"(
        set MathOps {
            __div: proc {a b} { expr $a / $b }
        }
    )");
    
    // 设置元表
    interpreter.execute("setmetatable global MathOps");
    
    // 使用元表进行除法运算
    interpreter.execute("puts [expr 32767 / 721]");
    
    // 使用while循环
    interpreter.execute(R"(
        set i 5
        while {$i > 0} {
            puts "Countdown: $i"
            set i [expr $i - 1]
        }
    )");
    
    // 使用switch语句
    interpreter.execute(R"(
        set day "Monday"
        switch $day {
            "Monday" { puts "Start of work week" }
            "Friday" { puts "End of work week" }
            default { puts "Midweek" }
        }
    )");
    
    // 使用类
    interpreter.execute(R"(
        class Counter {
            set value 0
            proc increment {} {
                set value [expr $value + 1]
            }
            proc get {} {
                return $value
            }
        }
        
        set c1 [new Counter]
        $c1 increment
        $c1 increment
        puts "Counter value: [$c1 get]"
    )");
    
    // 使用try-catch
    interpreter.execute(R"(
        try {
            expr 1 / 0
        } catch err {
            puts "Caught error: $err"
        }
    )");
    
    // 使用table命令
    interpreter.execute(R"(
        set myTable {a 1 b 2 c 3}
        puts "Keys: [table keys $myTable]"
        puts "Values: [table values $myTable]"
    )");
    
    return 0;
}