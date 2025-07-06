#ifndef CALL_STACK_H
#define CALL_STACK_H

#include <stack>
#include <map>
#include <vector>
#include <string>

struct StackFrame {
    std::string function;
    int line;
    std::map<std::string, Value> locals;
};

class CallStack {
private:
    std::stack<StackFrame> frames;
    
public:
    void push(const std::string& func, int line);
    void pop();
    StackFrame& top();
    void setLocal(const std::string& name, const Value& value);
    Value getLocal(const std::string& name) const;
    std::vector<StackFrame> getFrames() const;
    bool empty() const;
};

#endif // CALL_STACK_H

