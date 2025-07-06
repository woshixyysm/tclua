#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include <vector>
#include <string>
#include <variant>
#include <memory>
#include <unordered_map>
#include <stack>
#include <functional>

#include "Table.h"
#include "VariableManager.h"
#include "InterpreterException.h"

class ExpressionParser {
private:
    VariableManager& varManager;
    int currentLine = -1;
    
    enum TokenType { NUMBER, OPERATOR, VARIABLE, STRING, BRACE, BRACKET, END, IDENTIFIER };
    
    struct Token {
        TokenType type;
        std::string value;
        double numValue = 0.0;
    };
    
    std::vector<Token> tokens;
    size_t currentToken = 0;
    
    std::unordered_map<std::string, int> opPrecedence = {
        {"||", 1}, {"&&", 2},
        {"==", 3}, {"!=", 3}, {"<", 4}, {">", 4}, {"<=", 4}, {">=", 4},
        {"|", 5}, {"^", 6}, {"&", 7},
        {"<<", 8}, {">>", 8},
        {"+", 9}, {"-", 9},
        {"*", 10}, {"/", 10}, {"%", 10},
        {"!", 11}, {"~", 11},
        {"^", 12}
    };
    
    void tokenizeExpression(const std::string& expr);
    
    const Token& peek() const;
    Token consume();
    
    Value parsePrimary();
    Value parseExpression(int precedence = 0);
    
    Value applyUnaryOp(const std::string& op, const Value& operand);
    Value applyBinaryOp(const std::string& op, const Value& left, const Value& right);
    bool isTruthy(const Value& val) const;
    
public:
    ExpressionParser(VariableManager& vm) : varManager(vm) {}
    
    Value evaluate(const std::string& expr, int line = -1);
    
    static std::string valueToString(const Value& value);
};

#endif // EXPRESSION_PARSER_H
