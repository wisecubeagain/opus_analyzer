cmake_minimum_required(VERSION 3.10)

project(opus_analyzer
    VERSION 1.0.0
    DESCRIPTION "Opus Analyzer - Parse Opus audio format files"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 源文件
set(SOURCES
    src/opus_utils.cpp
    src/opus_frame_parser.cpp
)

# 头文件
set(HEADERS
    src/opus_types.h
    src/opus_utils.h
    src/opus_frame_parser.h
)

# 创建静态库（可选，用于集成到其他项目）
add_library(opus_analyzer_lib STATIC ${SOURCES} ${HEADERS})

# 添加 sample 子目录（构建示例程序）
add_subdirectory(sample)

