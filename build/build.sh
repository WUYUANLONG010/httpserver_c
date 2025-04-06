#!/bin/bash

# 设置项目目录（假设脚本位于项目根目录下）
SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
 
# 提取脚本所在的目录（不包括脚本文件名）
PROJECT_DIR="${SCRIPT_PATH%/*}"

# PROJECT_DIR=$(dirname "$0")/..
cd "$PROJECT_DIR" || exit
echo "$PROJECT_DIR"
# 创建构建目录（如果不存在）
BUILD_DIR="$PROJECT_DIR/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit
 
# 在构建目录中运行 CMake 配置项目

cmake "$PROJECT_DIR"
 
# 编译项目
make clean
make
 
# 可选：运行生成的可执行文件（假设名称为 MyExecutable）
#./MyExecutable
 
# 提示用户构建完成
echo "Build completed. Executable is located in $BUILD_DIR"
