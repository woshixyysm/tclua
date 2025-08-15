#include "CallStack.h"

void CallStack::push(const std::string& func, int line) {
    frames.push({func, line, {}});
}

void CallStack::pop() {
    if (!frames.empty()) frames.pop();
}

StackFrame& CallStack::top() {
    return frames.top();
}

void CallStack::setLocal(const std::string& name, const Value& value) {
    if (!frames.empty()) {
        frames.top().locals[name] = value;
    }
}

Value CallStack::getLocal(const std::string& name) const {
    if (!frames.empty()) {
        auto it = frames.top().locals.find(name);
        if (it != frames.top().locals.end()) {
            return it->second;
        }
    }
    return nullptr;
}

std::vector<StackFrame> CallStack::getFrames() const {
    std::vector<StackFrame> result;
    auto temp = frames;
    while (!temp.empty()) {
        result.push_back(temp.top());
        temp.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

bool CallStack::empty() const {
    return frames.empty();
}
