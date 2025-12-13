cmake_minimum_required(VERSION 3.10)

project(opus_sample
    VERSION 1.0.0
    DESCRIPTION "Opus Sample - Example program for parsing Opus files"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

# 源文件
set(SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/opus_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/opus_frame_parser.cpp
)

# 创建可执行文件
add_executable(opus_sample opus_sample.cpp ${SRC_FILES})

