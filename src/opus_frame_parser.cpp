/*
 * Opus Frame Parser
 * Opus 帧解析器实现
 */

#include "opus_frame_parser.h"
#include "opus_utils.h"
#include <cstring>
#include <cstddef>

namespace opus_analyzer {

bool parseTOC(uint8_t toc, uint8_t& config, bool& stereo, uint8_t& frame_count_code) {
    // TOC 字节结构：| config (5 bits) | s (1 bit) | c (2 bits) |
    config = (toc >> 3) & 0x1F;        // 高5位
    stereo = ((toc >> 2) & 0x01) != 0; // 第3位
    frame_count_code = toc & 0x03;     // 低2位
    return true;
}

bool parseOpusPacket(const uint8_t* data, size_t length, OpusFrameInfo& frame_info) {
    if (data == nullptr || length < 1) {
        return false;
    }

    // 清空输出结构
    memset(&frame_info, 0, sizeof(frame_info));
    frame_info.frame_sizes.clear();

    // 解析 TOC 字节
    uint8_t toc = data[0];
    uint8_t config;
    bool stereo;
    uint8_t frame_count_code;
    if (!parseTOC(toc, config, stereo, frame_count_code)) {
        return false;
    }

    frame_info.toc_byte = toc;   // 保存原始 TOC 字节
    frame_info.config = config;
    frame_info.stereo = stereo;
    frame_info.frame_count_code = frame_count_code;
    frame_info.data_offset = 1; // TOC 字节

    // 获取配置信息
    if (!getConfigInfo(config, frame_info.mode, frame_info.bandwidth, frame_info.frame_size)) {
        // 配置无效，返回 false
        return false;
    }

    size_t offset = 1; // 跳过 TOC 字节

    // 根据帧数代码解析不同的包类型
    switch (frame_count_code) {
        case 0: {
            // 0号包：一个包只包含一帧音频
            frame_info.frame_count = 1;
            
            // 检查是否为带分界包（在 TOC 后是否有帧长度编码）
            // 对于普通包，TOC 后直接是帧数据
            // 对于带分界包，TOC 后是帧长度编码，然后是帧数据
            if (offset >= length) {
                // 只有 TOC 字节，没有帧数据（合法的0号包）
                frame_info.frame_sizes.push_back(0);
                frame_info.total_size = 1;
                return true;
            }

            // 尝试解析帧长度编码（可能是带分界包）
            uint32_t frame_size = 0;
            size_t bytes_read = 0;
            if (parseFrameSizeEncoding(data + offset, length - offset, frame_size, bytes_read)) {
                // 可能是带分界包
                if (frame_size > 0 && offset + bytes_read + frame_size <= length) {
                    frame_info.is_self_delimiting = true;
                    frame_info.frame_sizes.push_back(frame_size);
                    offset += bytes_read;
                    frame_info.data_offset = offset;
                    frame_info.total_size = offset + frame_size;
                    return true;
                }
            }

            // 普通包：剩余所有数据都是帧数据
            frame_size = length - offset;
            if (frame_size > 1275) {
                return false; // 帧长度不能超过 1275 字节
            }
            frame_info.frame_sizes.push_back(frame_size);
            frame_info.total_size = length;
            return true;
        }

        case 1: {
            // 1号包：一个包里面含有两个大小相同的帧
            // 包大小必须是奇数（因为 (N-1)/2 必须是整数）
            if (length < 2) {
                return false;
            }

            // 检查是否为带分界包
            uint32_t frame_size = 0;
            size_t bytes_read = 0;
            if (parseFrameSizeEncoding(data + offset, length - offset, frame_size, bytes_read)) {
                // 带分界包
                if (frame_size > 0 && offset + bytes_read + frame_size * 2 <= length) {
                    frame_info.is_self_delimiting = true;
                    frame_info.frame_sizes.push_back(frame_size);
                    frame_info.frame_sizes.push_back(frame_size);
                    offset += bytes_read;
                    frame_info.data_offset = offset;
                    frame_info.total_size = offset + frame_size * 2;
                    frame_info.frame_count = 2;
                    return true;
                }
            }

            // 普通包：剩余数据平均分成两帧
            uint32_t remaining = length - offset;
            if (remaining % 2 != 0) {
                return false; // 必须是偶数
            }
            frame_size = remaining / 2;
            if (frame_size > 1275) {
                return false;
            }
            frame_info.frame_sizes.push_back(frame_size);
            frame_info.frame_sizes.push_back(frame_size);
            frame_info.total_size = length;
            frame_info.frame_count = 2;
            return true;
        }

        case 2: {
            // 2号包：一个包里面含有两个大小不同的帧
            if (offset >= length) {
                return false;
            }

            // 解析第一帧的长度
            uint32_t frame1_size = 0;
            size_t bytes_read = 0;
            if (!parseFrameSizeEncoding(data + offset, length - offset, frame1_size, bytes_read)) {
                return false;
            }
            offset += bytes_read;

            // 检查是否为带分界包（需要解析第二帧的长度）
            if (offset < length) {
                uint32_t frame2_size = 0;
                size_t bytes_read2 = 0;
                if (parseFrameSizeEncoding(data + offset, length - offset, frame2_size, bytes_read2)) {
                    // 带分界包
                    if (frame1_size > 0 && frame2_size > 0 && 
                        offset + bytes_read2 + frame1_size + frame2_size <= length) {
                        frame_info.is_self_delimiting = true;
                        frame_info.frame_sizes.push_back(frame1_size);
                        frame_info.frame_sizes.push_back(frame2_size);
                        offset += bytes_read2;
                        frame_info.data_offset = offset;
                        frame_info.total_size = offset + frame1_size + frame2_size;
                        frame_info.frame_count = 2;
                        return true;
                    }
                }
            }

            // 普通包：第一帧长度已知，剩余数据是第二帧
            if (frame1_size > 1275) {
                return false;
            }
            if (offset + frame1_size > length) {
                return false;
            }
            uint32_t frame2_size = length - offset - frame1_size;
            if (frame2_size > 1275) {
                return false;
            }
            frame_info.frame_sizes.push_back(frame1_size);
            frame_info.frame_sizes.push_back(frame2_size);
            frame_info.total_size = length;
            frame_info.frame_count = 2;
            return true;
        }

        case 3: {
            // 3号包：一个包里面含有任意个帧
            if (offset >= length) {
                return false;
            }

            // 读取帧数量字节
            uint8_t frame_count_byte = data[offset++];
            bool is_vbr = ((frame_count_byte >> 7) & 0x01) != 0;
            bool has_padding = ((frame_count_byte >> 6) & 0x01) != 0;
            uint8_t frame_count = frame_count_byte & 0x3F; // 低6位

            if (frame_count == 0) {
                return false; // 至少包含一个帧
            }

            frame_info.is_cbr = !is_vbr;
            frame_info.has_padding = has_padding;
            frame_info.frame_count = frame_count;

            // 解析填充长度（如果有）
            // 参考 libopus 的实现：填充字节在包的最后，从 len 中减去填充字节数
            uint32_t padding_size = 0;
            size_t padding_bytes_read = 0;
            if (has_padding) {
                // 按照 libopus 的方式解析填充长度
                // 填充长度编码：连续 0xFF 表示 254 个填充字节，最后一个字节 < 0xFF 表示剩余的填充字节数
                size_t padding_offset = offset;
                int p;
                do {
                    if (padding_offset >= length) {
                        return false;
                    }
                    p = data[padding_offset++];
                    int tmp = (p == 255) ? 254 : p;
                    if (length < padding_offset + tmp) {
                        return false;
                    }
                    length -= tmp;  // 从 length 中减去填充字节数（参考 libopus）
                    padding_size += tmp;
                } while (p == 255);
                padding_bytes_read = padding_offset - offset;
                offset = padding_offset;
                frame_info.padding_size = padding_size;
            }
            
            // 现在 length 已经是有效数据的大小（不包括填充字节）
            // 参考 libopus：对于 CBR，使用 len/count 来计算每帧大小
            
            if (is_vbr) {
                // VBR：解析前 M-1 个帧的长度
                // 参考 libopus：last_size = len，然后逐个解析前 M-1 个帧的大小
                frame_info.frame_sizes.clear();
                uint32_t last_size = length - offset;  // 剩余的数据大小
                for (uint8_t i = 0; i < frame_count - 1; i++) {
                    if (offset >= length) {
                        return false;
                    }
                    uint32_t frame_size = 0;
                    size_t bytes_read = 0;
                    if (!parseFrameSizeEncoding(data + offset, length - offset, frame_size, bytes_read)) {
                        return false;
                    }
                    if (frame_size < 0 || frame_size > length - offset) {
                        return false;
                    }
                    if (frame_size > 1275) {
                        return false;
                    }
                    frame_info.frame_sizes.push_back(frame_size);
                    offset += bytes_read;
                    last_size -= bytes_read + frame_size;  // 减去已解析的帧大小
                }
                
                if (last_size < 0 || last_size > 1275) {
                    return false;
                }
                frame_info.frame_sizes.push_back(last_size);
            } else {
                // CBR：所有帧大小相同
                // 参考 libopus：last_size = len/count
                // 注意：此时 length 已经是有效数据的大小（不包括填充字节）
                
                // 对于 Opus 裸流，如果传入的 length 太大（整个文件大小），需要先找到包边界
                size_t packet_size = 0;
                uint32_t effective_data_size;
                
                // 计算可能的包大小范围
                uint32_t min_packet_size = offset + frame_count * 10 + padding_size;
                uint32_t max_packet_size = offset + frame_count * 1275 + padding_size;
                
                // 检查 length 是否在合理范围内
                size_t original_length = length + padding_size;  // 恢复原始长度（包括填充）
                
                if (original_length >= min_packet_size && original_length <= max_packet_size) {
                    // length 在合理范围内，使用它
                    packet_size = original_length;
                    effective_data_size = length - offset;
                } else if (original_length > max_packet_size) {
                    // length 太大，说明传入的是整个文件大小
                    // 搜索下一个相同的 TOC+frame_count_byte 组合来找到包边界
                    uint8_t current_toc = data[0];
                    uint8_t current_frame_count_byte = data[1];
                    
                    for (size_t try_size = min_packet_size; try_size <= max_packet_size && try_size < original_length; try_size++) {
                        if (try_size + 1 < original_length && 
                            data[try_size] == current_toc && 
                            data[try_size + 1] == current_frame_count_byte) {
                            // 找到相同的 TOC+frame_count_byte 组合
                            packet_size = try_size;
                            break;
                        }
                    }
                    
                    if (packet_size == 0) {
                        return false;  // 无法找到包边界
                    }
                    
                    // 重新计算有效数据大小：packet_size - offset - padding_size
                    effective_data_size = packet_size - offset - padding_size;
                } else {
                    // length 太小
                    return false;
                }
                
                // 使用 libopus 的逻辑：last_size = len/count
                uint32_t frame_size = effective_data_size / frame_count;
                
                if (frame_size * frame_count != effective_data_size) {
                    return false;  // 不能整除
                }
                if (frame_size > 1275) {
                    return false;
                }
                
                for (uint8_t i = 0; i < frame_count; i++) {
                    frame_info.frame_sizes.push_back(frame_size);
                }
                
                // 设置包的总大小
                frame_info.total_size = packet_size;
            }

            frame_info.data_offset = offset;
            // 对于 3 号包，total_size 在各个分支中已经设置
            // 如果没有设置，说明是 VBR 或其他特殊情况，无法确定包边界
            if (frame_info.total_size == 0) {
                // 无法确定包大小，返回 0
                frame_info.total_size = 0;
            }
            return true;
        }

        default:
            return false;
    }
}

} // namespace opus_analyzer

