# TCLuaInterpreter - TCL with Lua Tables and OOP

## 概述

TCLuaInterpreter 是一个创新的脚本解释器，巧妙地将 TCL 的简洁语法与 Lua 的强大数据结构相结合，并添加了面向对象编程功能。该解释器支持 TCL 的基本命令（如 `set`, `expr`, `puts`, `proc` 等），同时集成了 Lua 风格的表（Table）和元表（metatable），并提供了类（class）和对象（object）等面向对象特性。

## 主要特性

1. TCL 命令支持：
   - 变量操作：`set`, `incr`
   - 表达式计算：`expr`
   - 输出：`puts`
   - 流程控制：`if`, `for`, `while`, `switch`
   - 过程定义：`proc`
   - 异常处理：`try`/`catch`
   - 字符串操作：`string`
   - 表操作：`table`

2. Lua 表集成：
   - 支持键值对存储
   - 元表（metatable）实现继承和默认值
   - 表操作命令：`table keys`, `table values`, `table setdefault`

3. 面向对象编程：
   - `class` 命令定义类
   - `new` 命令创建对象实例
   - 方法调用：`$object method args`
   - 通过元表实现继承

4. 高级特性：
   - 命令替换：`[command]`
   - 变量替换：`$var`
   - 大括号字符串：`{text}`
   - 元表支持运算符重载

# 变量操作
set x 10
set y [expr $x * 2]
puts "Result: $y"

# 过程定义
proc add {a b} {
    return [expr $a + $b]
}
puts "5 + 3 = [add 5 3]"

# 条件判断
if {$x > 5} {
    puts "x is greater than 5"
}

## 表操作

# 创建表
set person {name "John" age 30}

# 设置字段
$person set name "John Doe"
$person set age 31

# 获取字段
puts "Name: [$person get name], Age: [$person get age]"

# 表命令
puts "Keys: [table keys $person]"
puts "Values: [table values $person]"

## 面向对象编程

# 定义类
class Counter {
    set count 0
    proc increment {} {
        set count [expr $count + 1]
    }
    proc get {} {
        return $count
    }
}

# 创建对象
set c [new Counter]
$c increment
$c increment
puts "Counter: [$c get]"  # 输出: Counter: 2

## 异常处理

# 异常捕获
try {
    expr 1 / 0
} catch err {
    puts "Caught error: $err"
}

## 元表操作

# 定义元表
set MathOps {
    __div: proc {a b} { expr $a / $b }
}

# 设置元表
setmetatable global MathOps

# 使用元表进行除法运算
puts [expr 32767 / 721]  # 使用自定义除法实现

## 构建与运行

1. 确保安装了CMake和C++17编译器
2. 编译程序：
   mkdir build
   cd build
   cmake ..
   cmake --build .
3. 运行解释器：
   ./tclua

## While 循环
set i 5
while {$i > 0} {
    puts "Countdown: $i"
    set i [expr $i - 1]
}

## Switch 语句
set day "Monday"
switch $day {
    "Monday" { puts "Start of work week" }
    "Friday" { puts "End of work week" }
    default { puts "Midweek" }
}

## 表默认值
set myTable {}
table setdefault $myTable 0
puts [myTable get non_existent_key]  # 输出默认值 0

## 注意事项

1. 解释器处于alpha阶段，某些高级特性可能不够完善
2. 错误处理会显示 C++ 异常信息
3. 性能未优化，不适用于高性能场景
4. 4. 4. 🧐

## 贡献

欢迎通过 Issue 和 Pull Request 贡献代码和改进建议！
