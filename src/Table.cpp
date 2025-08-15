#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <functional>

using Value = std::variant<double, std::string, bool, std::nullptr_t, std::shared_ptr<class Table>>;

class Table;

class Table : public std::enable_shared_from_this<Table> {
public:
    std::map<std::string, Value> fields;
    std::shared_ptr<Table> metatable;
    
    bool contains(const std::string& key) const;
    Value get(const std::string& key, int line = -1) const;
    void set(const std::string& key, const Value& value);
    
    std::vector<std::string> keys() const;
    std::vector<Value> values() const;
    
    void sort(const std::function<bool(const Value&, const Value&)>& comparator);
    Table filter(const std::function<bool(const std::string&, const Value&)>& predicate) const;
    Table map(const std::function<Value(const std::string&, const Value&)>& mapper) const;
};

#endif // TABLE_H
void Table::set(const std::string& key, const Value& value) {
    fields[key] = value;
}

Value Table::get(const std::string& key, int line) const {
    auto it = fields.find(key);
    if (it == fields.end()) return nullptr;
    return it->second;
}
