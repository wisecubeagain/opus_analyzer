/*
 * Opus Utils
 * Opus 解析工具函数实现
 */

#include "opus_utils.h"
#include "opus_types.h"
#include "opus_frame_parser.h"

namespace opus_analyzer {

bool getConfigInfo(uint8_t config, OpusMode& mode, OpusBandwidth& bandwidth, OpusFrameSize& frame_size) {
    if (config > 31) {
        return false;
    }

    // 根据配置数确定编码模式、带宽和帧长度
    if (config >= 0 && config <= 3) {
        // SILK-only NB
        mode = OpusMode::SILK_ONLY;
        bandwidth = OpusBandwidth::NB;
        switch (config) {
            case 0: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 1: frame_size = OpusFrameSize::FRAME_20_MS; break;
            case 2: frame_size = OpusFrameSize::FRAME_40_MS; break;
            case 3: frame_size = OpusFrameSize::FRAME_60_MS; break;
            default: return false;
        }
    } else if (config >= 4 && config <= 7) {
        // SILK-only MB
        mode = OpusMode::SILK_ONLY;
        bandwidth = OpusBandwidth::MB;
        switch (config) {
            case 4: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 5: frame_size = OpusFrameSize::FRAME_20_MS; break;
            case 6: frame_size = OpusFrameSize::FRAME_40_MS; break;
            case 7: frame_size = OpusFrameSize::FRAME_60_MS; break;
            default: return false;
        }
    } else if (config >= 8 && config <= 11) {
        // SILK-only WB
        mode = OpusMode::SILK_ONLY;
        bandwidth = OpusBandwidth::WB;
        switch (config) {
            case 8: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 9: frame_size = OpusFrameSize::FRAME_20_MS; break;
            case 10: frame_size = OpusFrameSize::FRAME_40_MS; break;
            case 11: frame_size = OpusFrameSize::FRAME_60_MS; break;
            default: return false;
        }
    } else if (config >= 12 && config <= 13) {
        // Hybrid SWB
        mode = OpusMode::HYBRID;
        bandwidth = OpusBandwidth::SWB;
        switch (config) {
            case 12: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 13: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else if (config >= 14 && config <= 15) {
        // Hybrid FB
        mode = OpusMode::HYBRID;
        bandwidth = OpusBandwidth::FB;
        switch (config) {
            case 14: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 15: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else if (config >= 16 && config <= 19) {
        // CELT-only NB
        mode = OpusMode::CELT_ONLY;
        bandwidth = OpusBandwidth::NB;
        switch (config) {
            case 16: frame_size = OpusFrameSize::FRAME_2_5_MS; break;
            case 17: frame_size = OpusFrameSize::FRAME_5_MS; break;
            case 18: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 19: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else if (config >= 20 && config <= 23) {
        // CELT-only WB
        mode = OpusMode::CELT_ONLY;
        bandwidth = OpusBandwidth::WB;
        switch (config) {
            case 20: frame_size = OpusFrameSize::FRAME_2_5_MS; break;
            case 21: frame_size = OpusFrameSize::FRAME_5_MS; break;
            case 22: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 23: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else if (config >= 24 && config <= 27) {
        // CELT-only SWB
        mode = OpusMode::CELT_ONLY;
        bandwidth = OpusBandwidth::SWB;
        switch (config) {
            case 24: frame_size = OpusFrameSize::FRAME_2_5_MS; break;
            case 25: frame_size = OpusFrameSize::FRAME_5_MS; break;
            case 26: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 27: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else if (config >= 28 && config <= 31) {
        // CELT-only FB
        mode = OpusMode::CELT_ONLY;
        bandwidth = OpusBandwidth::FB;
        switch (config) {
            case 28: frame_size = OpusFrameSize::FRAME_2_5_MS; break;
            case 29: frame_size = OpusFrameSize::FRAME_5_MS; break;
            case 30: frame_size = OpusFrameSize::FRAME_10_MS; break;
            case 31: frame_size = OpusFrameSize::FRAME_20_MS; break;
            default: return false;
        }
    } else {
        return false;
    }

    return true;
}

bool parseFrameSizeEncoding(const uint8_t* data, size_t length, uint32_t& frame_size, size_t& bytes_read) {
    if (data == nullptr || length < 1) {
        return false;
    }

    bytes_read = 0;
    frame_size = 0;

    // 第一个字节为 0：表示没有任何帧数据（DTX 或包丢失）
    if (data[0] == 0) {
        frame_size = 0;
        bytes_read = 1;
        return true;
    }

    // 第一个字节为 1 ~ 251：表示帧的字节数
    if (data[0] >= 1 && data[0] <= 251) {
        frame_size = data[0];
        bytes_read = 1;
        return true;
    }

    // 第一个字节为 252 ~ 255：需要第二个字节参与编码
    if (data[0] >= 252 && data[0] <= 255) {
        if (length < 2) {
            return false;
        }
        // 帧长度 = (第二字节 * 4) + 第一字节
        frame_size = (data[1] * 4) + data[0];
        bytes_read = 2;
        return true;
    }

    return false;
}

bool parsePaddingLength(const uint8_t* data, size_t length, uint32_t& padding_size, size_t& bytes_read) {
    if (data == nullptr || length < 1) {
        return false;
    }

    padding_size = 0;
    bytes_read = 0;
    size_t offset = 0;

    // 填充长度编码：连续 0xFF 累加 + 最后一个 < 0xFF 的值
    while (offset < length && data[offset] == 0xFF) {
        padding_size += 0xFF;
        offset++;
    }

    if (offset >= length) {
        return false;
    }

    padding_size += data[offset];
    offset++;

    // 如果第一个字节是 255，需要继续读取
    // 根据标准，如果值为 N 且 N > 0 且 N <= 254，表示填充 N 个字节
    // 如果值为 255，表示填充了 254 个字节，下一个字节编码更多填充字节数
    // 但这里我们简化处理，只处理第一层编码
    bytes_read = offset;
    return true;
}

bool findNextPacket(const uint8_t* data, size_t length, size_t current_offset, size_t& next_offset) {
    if (data == nullptr || length == 0 || current_offset >= length) {
        return false;
    }

    // 尝试解析当前包来确定下一个包的位置
    OpusFrameInfo frame_info;
    if (!parseOpusPacket(data + current_offset, length - current_offset, frame_info)) {
        return false;
    }

    // 如果成功解析，下一个包的位置就是当前包的总大小
    if (frame_info.total_size > 0 && current_offset + frame_info.total_size <= length) {
        next_offset = current_offset + frame_info.total_size;
        return true;
    }

    return false;
}

} // namespace opus_analyzer

