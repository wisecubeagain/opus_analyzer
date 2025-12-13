/*
 * Opus Sample
 * 示例程序：从 Opus 文件中解析每一帧的配置信息
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <iomanip>

#include "../src/opus_frame_parser.h"
#include "../src/opus_types.h"

using namespace opus_analyzer;

// 打印 Opus 帧信息
void printOpusFrameInfo(const OpusFrameInfo& frame_info, int frame_index) {
    std::cout << "\n========== Opus 包 #" << frame_index << " ==========" << std::endl;
    std::cout << "TOC 字节: 0x" << std::hex << std::setw(2) << std::setfill('0') 
              << (int)frame_info.toc_byte << std::dec << std::endl;
    std::cout << "配置数 (config): " << (int)frame_info.config << std::endl;
    std::cout << "编码模式: " << getModeString(frame_info.mode) << std::endl;
    std::cout << "音频带宽: " << getBandwidthString(frame_info.bandwidth) << std::endl;
    std::cout << "帧长度: " << getFrameSizeString(frame_info.frame_size) << std::endl;
    std::cout << "立体声: " << (frame_info.stereo ? "是" : "否") << std::endl;
    std::cout << "帧数代码 (c): " << (int)frame_info.frame_count_code << std::endl;
    std::cout << "实际帧数: " << frame_info.frame_count << std::endl;
    std::cout << "包总大小: " << frame_info.total_size << " 字节" << std::endl;
    std::cout << "数据起始偏移: " << frame_info.data_offset << " 字节" << std::endl;
    std::cout << "带分界包: " << (frame_info.is_self_delimiting ? "是" : "否") << std::endl;

    if (frame_info.frame_count_code == 3) {
        std::cout << "CBR/VBR: " << (frame_info.is_cbr ? "CBR" : "VBR") << std::endl;
        std::cout << "有填充字节: " << (frame_info.has_padding ? "是" : "否") << std::endl;
        if (frame_info.has_padding) {
            std::cout << "填充字节数: " << frame_info.padding_size << " 字节" << std::endl;
        }
    }

    if (!frame_info.frame_sizes.empty()) {
        std::cout << "\n各帧大小:" << std::endl;
        for (size_t i = 0; i < frame_info.frame_sizes.size(); i++) {
            std::cout << "  帧 #" << (i + 1) << ": " << frame_info.frame_sizes[i] << " 字节" << std::endl;
        }
    }
    std::cout << "=====================================" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <opus_file>" << std::endl;
        std::cerr << "示例: " << argv[0] << " ../../test.opus" << std::endl;
        return 1;
    }

    const char* opus_file = argv[1];
    FILE* infile = fopen(opus_file, "rb");
    if (infile == nullptr) {
        std::cerr << "错误: 无法打开文件: " << opus_file << std::endl;
        return 1;
    }

    std::cout << "正在解析 Opus 文件: " << opus_file << std::endl;
    std::cout << "解析每一帧的配置信息..." << std::endl;

    // 分配缓冲区
    const size_t BUFSIZE = 32 * 1024 * 1024; // 32MB
    uint8_t* buf = (uint8_t*)malloc(BUFSIZE);
    if (buf == nullptr) {
        std::cerr << "错误: 无法分配内存" << std::endl;
        fclose(infile);
        return 1;
    }

    size_t rsz = 0;
    size_t sz = 0;
    uint8_t* p = buf;
    int packet_count = 0;
    size_t current_offset = 0;

    // 读取并解析 Opus 流
    while (1) {
        rsz = fread(buf + sz, 1, BUFSIZE - sz, infile);
        if (rsz == 0) {
            if (ferror(infile)) {
                std::cerr << "错误: 读取文件失败" << std::endl;
                break;
            }
            break; // EOF
        }

        sz += rsz;

        // 尝试解析 Opus 包
        // Opus 裸流：第一个字节就是 TOC，按照协议规范解析
        while (current_offset < sz) {
            OpusFrameInfo frame_info;
            if (parseOpusPacket(p + current_offset, sz - current_offset, frame_info)) {
                packet_count++;
                printOpusFrameInfo(frame_info, packet_count);

                // 移动到下一个包
                if (frame_info.total_size > 0) {
                    current_offset += frame_info.total_size;
                } else {
                    // 如果无法确定包大小（例如 3 号包），尝试查找下一个有效的包
                    // 对于 Opus 裸流，我们只能通过尝试解析来确定包边界
                    // 从当前包的数据结束位置开始查找
                    size_t data_end = current_offset + frame_info.data_offset + 
                                     (frame_info.frame_sizes.empty() ? 0 : 
                                      frame_info.frame_sizes[0] * frame_info.frame_count);
                    size_t next_offset = data_end;
                    bool found_next = false;
                    // 尝试从数据结束位置开始解析，最多尝试 1000 个字节
                    for (size_t i = 0; i < 1000 && next_offset < sz; i++) {
                        OpusFrameInfo test_info;
                        if (parseOpusPacket(p + next_offset, sz - next_offset, test_info)) {
                            // 找到了下一个有效的包
                            current_offset = next_offset;
                            found_next = true;
                            break;
                        }
                        next_offset++;
                    }
                    if (!found_next) {
                        // 没有找到下一个包，向前移动一个字节
                        current_offset++;
                    }
                }
            } else {
                // 解析失败，可能是数据不完整或不是有效的 Opus 包
                // 尝试向前移动一个字节继续查找
                current_offset++;
                
                // 如果已经移动到缓冲区末尾，需要读取更多数据
                if (current_offset >= sz) {
                    break;
                }
            }

            // 防止无限循环
            if (current_offset >= sz) {
                break;
            }
        }

        // 保留未处理的数据到缓冲区开头
        if (current_offset > 0 && current_offset < sz) {
            size_t remaining = sz - current_offset;
            memmove(buf, p + current_offset, remaining);
            sz = remaining;
            current_offset = 0;
        } else if (current_offset >= sz) {
            // 所有数据都已处理
            sz = 0;
            current_offset = 0;
        }

        // 如果缓冲区已满但没有找到有效包，可能需要清空部分数据
        if (sz >= BUFSIZE - 1024) {
            // 保留最后 4KB 数据，以防包跨越边界
            size_t keep_size = 4096;
            if (sz > keep_size) {
                memmove(buf, p + sz - keep_size, keep_size);
                sz = keep_size;
                current_offset = 0;
            }
        }
    }

    std::cout << "\n========== 解析完成 ==========" << std::endl;
    std::cout << "总共找到 " << packet_count << " 个 Opus 包" << std::endl;

    // 清理资源
    free(buf);
    fclose(infile);

    return 0;
}

