#!/bin/bash

PROJECT_ROOT=$(dirname "$(realpath $0)")

BUILD_DIR="${PROJECT_ROOT}/build"
mkdir -p "${BUILD_DIR}"

cd "${BUILD_DIR}"

cmake ..

if [ $? -ne 0 ]; then
    echo "CMake 配置失败"
    exit 1
fi

cmake --build .

if [ $? -ne 0 ]; then
    echo "项目构建失败"
    exit 1
fi

echo "构建成功,位于 ${BUILD_DIR}"
