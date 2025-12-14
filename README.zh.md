# Opus 解析器

一个独立的库和工具集，用于从 Opus 格式文件中解析每一帧的配置信息。支持解析 Opus 包的 TOC 字节、编码模式、音频带宽、帧长度等参数。

[English Documentation](README.md)

## 功能

从 Opus 文件中提取并解析每一帧的配置信息，输出相关信息。当前版本支持：
- **TOC 字节解析**：解析配置数、立体声标志、帧数代码
- **编码模式识别**：SILK-only、Hybrid、CELT-only
- **音频带宽识别**：NB、MB、WB、SWB、FB
- **帧长度识别**：2.5ms、5ms、10ms、20ms、40ms、60ms
- **包类型识别**：0号包（单帧）、1号包（两帧相同大小）、2号包（两帧不同大小）、3号包（多帧）
- **带分界包支持**：支持解析带分界格式的 Opus 包
- **CBR/VBR 支持**：支持恒定比特率（CBR）和可变比特率（VBR）包
- **填充字节支持**：支持解析 3 号包中的填充字节

## 项目结构

```
opus_analyzer/
├── src/                      # 核心解析代码
│   ├── opus_types.h          # Opus 数据结构定义
│   ├── opus_utils.h/cpp      # Opus 解析工具函数
│   └── opus_frame_parser.h/cpp # Opus 帧解析器
├── sample/                   # 示例程序
│   ├── opus_sample.cpp       # Opus 解析示例
│   └── CMakeLists.txt
├── script/                   # 构建脚本
│   └── build_sample.sh       # 构建示例程序的脚本
├── CMakeLists.txt            # 根目录 CMake（构建库）
└── README.md
```

## 依赖

- CMake 3.10+
- C++11 编译器
- 无其他外部依赖

## 快速开始

### 构建示例程序

使用提供的构建脚本：

```bash
cd opus_analyzer
./script/build_sample.sh
```

或者手动构建：

```bash
cd opus_analyzer
mkdir build && cd build
cmake ..
make
```

### 运行示例

```bash
cd build/sample
./opus_sample ../../../test.opus
```

## 集成到其他项目

如果需要将解析功能集成到自己的项目中，可以复制 `src/` 目录下的文件：

- `src/opus_types.h` - 数据结构定义
- `src/opus_utils.h/cpp` - 工具函数（配置信息获取、帧长度编码解析等）
- `src/opus_frame_parser.h/cpp` - Opus 包解析器

使用示例：

```cpp
#include "opus_frame_parser.h"
#include "opus_types.h"

using namespace opus_analyzer;

// 解析 Opus 包
OpusFrameInfo frame_info;
if (parseOpusPacket(data, data_size, frame_info)) {
    // 使用解析后的数据
    std::cout << "编码模式: " << getModeString(frame_info.mode) << std::endl;
    std::cout << "音频带宽: " << getBandwidthString(frame_info.bandwidth) << std::endl;
    std::cout << "帧长度: " << getFrameSizeString(frame_info.frame_size) << std::endl;
    std::cout << "帧数: " << frame_info.frame_count << std::endl;
}
```

## 数据格式说明

关于 Opus 数据结构的详细说明，请参考：
- [Opus 音频编码格式 - 陈亮的个人博客](https://chenliang.org/2020/03/15/opus-format/)
- [Opus 音频编码格式 - CSDN博客](https://blog.csdn.net/u011487024/article/details/155133281)

主要字段说明：
- **配置数 (config)**：0-31，定义编码模式、音频带宽和帧长度
- **立体声标志 (s)**：0 表示单声道，1 表示立体声
- **帧数代码 (c)**：0-3，表示包中包含的帧数类型
- **帧长度**：每个帧的最大长度为 1275 字节

## 注意事项

- 程序支持解析 Opus 裸流（raw Opus stream）
- 支持带分界包（self-delimiting packets）和普通包
- 对于 Opus 裸流，包与包之间没有明确的边界标记，程序通过解析包结构来确定边界
- 如果文件是封装在容器格式中（如 Ogg），需要先提取出 Opus 裸流
- 解析逻辑参考了官方 libopus 实现，确保兼容性和正确性

