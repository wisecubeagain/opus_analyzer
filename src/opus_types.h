/*
 * Opus Types
 * Opus 数据结构定义
 */

#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace opus_analyzer {

// 编码模式
enum class OpusMode {
    SILK_ONLY,
    HYBRID,
    CELT_ONLY
};

// 音频带宽
enum class OpusBandwidth {
    NB,    // Narrowband (4 kHz)
    MB,    // Medium-band (6 kHz)
    WB,    // Wideband (8 kHz)
    SWB,   // Super-wideband (12 kHz)
    FB     // Fullband (20 kHz)
};

// 帧长度（毫秒）
enum class OpusFrameSize {
    FRAME_2_5_MS,
    FRAME_5_MS,
    FRAME_10_MS,
    FRAME_20_MS,
    FRAME_40_MS,
    FRAME_60_MS
};

// Opus 帧信息
struct OpusFrameInfo {
    uint8_t toc_byte;            // TOC 字节（原始值）
    uint8_t config;              // 配置数 (0-31)
    OpusMode mode;                // 编码模式
    OpusBandwidth bandwidth;      // 音频带宽
    OpusFrameSize frame_size;      // 帧长度
    bool stereo;                  // 是否立体声
    uint8_t frame_count_code;     // 帧数代码 (0-3)
    uint32_t frame_count;         // 实际帧数
    std::vector<uint32_t> frame_sizes;  // 每帧的字节数
    uint32_t total_size;          // 包总大小（字节）
    uint32_t data_offset;         // 数据起始偏移
    bool is_self_delimiting;      // 是否为带分界包
    bool is_cbr;                  // 是否为 CBR（仅用于3号包）
    bool has_padding;             // 是否有填充字节（仅用于3号包）
    uint32_t padding_size;        // 填充字节数（仅用于3号包）
};

// 获取编码模式字符串
std::string getModeString(OpusMode mode);

// 获取带宽字符串
std::string getBandwidthString(OpusBandwidth bandwidth);

// 获取帧长度字符串
std::string getFrameSizeString(OpusFrameSize frame_size);

} // namespace opus_analyzer

// 内联函数实现
namespace opus_analyzer {

inline std::string getModeString(OpusMode mode) {
    switch (mode) {
        case OpusMode::SILK_ONLY: return "SILK-only";
        case OpusMode::HYBRID: return "Hybrid";
        case OpusMode::CELT_ONLY: return "CELT-only";
        default: return "Unknown";
    }
}

inline std::string getBandwidthString(OpusBandwidth bandwidth) {
    switch (bandwidth) {
        case OpusBandwidth::NB: return "NB (4 kHz)";
        case OpusBandwidth::MB: return "MB (6 kHz)";
        case OpusBandwidth::WB: return "WB (8 kHz)";
        case OpusBandwidth::SWB: return "SWB (12 kHz)";
        case OpusBandwidth::FB: return "FB (20 kHz)";
        default: return "Unknown";
    }
}

inline std::string getFrameSizeString(OpusFrameSize frame_size) {
    switch (frame_size) {
        case OpusFrameSize::FRAME_2_5_MS: return "2.5 ms";
        case OpusFrameSize::FRAME_5_MS: return "5 ms";
        case OpusFrameSize::FRAME_10_MS: return "10 ms";
        case OpusFrameSize::FRAME_20_MS: return "20 ms";
        case OpusFrameSize::FRAME_40_MS: return "40 ms";
        case OpusFrameSize::FRAME_60_MS: return "60 ms";
        default: return "Unknown";
    }
}

} // namespace opus_analyzer

