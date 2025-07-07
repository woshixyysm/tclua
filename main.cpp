#include "include/Tclua.h"
#include "include/InterpreterException.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

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
                        currentToken += line[pos];
                        pos++;
                        if (pos < line.length()) currentToken += line[pos];
                    } else {
                        currentToken += line[pos];
                    }
                    pos++;
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
};

int main() {
    Tclua interpreter;
    
    // 示例脚本
    std::string script = R"(
        # 基础变量操作
        set x 10
        set y [expr $x * 2]
        puts "x = $x, y = $y"
        
        # 表操作
        table create person
        table set person name "John"
        table set person age 30
        table set person scores {math 90 science 85}
        puts "Person: [table get person]"
        
        # 条件断点
        breakpoint add 15 {$x > 5}
        breakpoint enable
        
        # 数学函数
        set pi 3.14159
        set rad [math sin [expr $pi / 4]]
        puts "sin(π/4) = $rad"
        
        # 文件操作
        file write "test.txt" "Hello, World!"
        set content [file read "test.txt"]
        puts "File content: $content"
        
        # 模块系统
        module mymodule
        set mymodule::version 1.0
        import mymodule
        puts "Module version: $mymodule::version"
        
        # 调试功能
        breakpoint add 25
        puts "This line has a breakpoint"
    )";
    
    interpreter.execute(script);
    
    return 0;
}
