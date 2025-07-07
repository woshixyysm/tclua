#!/bin/bash

# 设置项目根目录，假设脚本位于项目根目录下
PROJECT_ROOT=$(dirname "$(realpath $0)")

# 创建构建目录
BUILD_DIR="${PROJECT_ROOT}/build"
mkdir -p "${BUILD_DIR}"

# 进入构建目录
cd "${BUILD_DIR}"

# 运行 CMake 生成构建文件
cmake ..

# 检查 CMake 是否成功
if [ $? -ne 0 ]; then
    echo "CMake 配置失败"
    exit 1
fi

# 构建项目
cmake --build .

# 检查构建是否成功
if [ $? -ne 0 ]; then
    echo "项目构建失败"
    exit 1
fi

echo "项目构建成功！可执行文件位于 ${BUILD_DIR}"
