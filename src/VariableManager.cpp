#include "VariableManager.h"
#include "InterpreterException.h"

VariableManager::VariableManager(CallStack& cs) : callStack(cs) {}

void VariableManager::set(const std::string& name, const Value& value, int line) {
    // 先检查局部变量
    auto localValue = callStack.getLocal(name);
    if (!std::holds_alternative<std::nullptr_t>(localValue)) {
        callStack.setLocal(name, value);
        return;
    }
    
    // 处理表字段
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

Value VariableManager::get(const std::string& name, int line) const {
    // 先检查局部变量
    auto localValue = callStack.getLocal(name);
    if (!std::holds_alternative<std::nullptr_t>(localValue)) {
        return localValue;
    }
    
    // 处理表字段
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

bool VariableManager::exists(const std::string& name) const {
    return variables.find(name) != variables.end();
}
