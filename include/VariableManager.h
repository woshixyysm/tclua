#ifndef VARIABLE_MANAGER_H
#define VARIABLE_MANAGER_H

#include <unordered_map>
#include <map>
#include <string>
#include <variant>
#include <memory>

#include "Table.h"
#include "CallStack.h"

class VariableManager {
private:
    struct Variable {
        Value value;
        bool isTableField = false;
        std::string tableName;
        std::string fieldName;
    };
    
    std::unordered_map<std::string, Variable> variables;
    CallStack& callStack;
    
public:
    VariableManager(CallStack& cs);
    
    void set(const std::string& name, const Value& value, int line = -1);
    Value get(const std::string& name, int line = -1) const;
    bool exists(const std::string& name) const;
};

#endif // VARIABLE_MANAGER_H
