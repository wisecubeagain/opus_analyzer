#!/bin/bash

# 构建 Opus Sample 示例程序

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SAMPLE_DIR="$PROJECT_ROOT/sample"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=========================================="
echo "构建 Opus Sample"
echo "=========================================="

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 运行 CMake
echo "运行 CMake..."
cmake ..

# 编译
echo "编译中..."
make

echo ""
echo "=========================================="
echo "构建完成！"
echo "=========================================="
echo "可执行文件位置: $BUILD_DIR/sample/opus_sample"
echo ""
echo "使用方法:"
echo "  $BUILD_DIR/sample/opus_sample <opus_file>"
echo ""

