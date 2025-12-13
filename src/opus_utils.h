/*
 * Opus Utils
 * Opus 解析工具函数
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include "opus_types.h"

namespace opus_analyzer {

/**
 * 从配置数获取编码模式、带宽和帧长度
 * @param config 配置数 (0-31)
 * @param mode 输出：编码模式
 * @param bandwidth 输出：音频带宽
 * @param frame_size 输出：帧长度
 * @return 是否成功
 */
bool getConfigInfo(uint8_t config, OpusMode& mode, OpusBandwidth& bandwidth, OpusFrameSize& frame_size);

/**
 * 解析帧长度编码（1-2字节）
 * @param data 数据指针
 * @param length 剩余数据长度
 * @param frame_size 输出：帧长度（字节）
 * @param bytes_read 输出：读取的字节数
 * @return 是否成功
 */
bool parseFrameSizeEncoding(const uint8_t* data, size_t length, uint32_t& frame_size, size_t& bytes_read);

/**
 * 解析填充长度编码
 * @param data 数据指针
 * @param length 剩余数据长度
 * @param padding_size 输出：填充字节数
 * @param bytes_read 输出：读取的字节数
 * @return 是否成功
 */
bool parsePaddingLength(const uint8_t* data, size_t length, uint32_t& padding_size, size_t& bytes_read);

/**
 * 查找下一个 Opus 包的起始位置
 * 注意：Opus 裸流没有明确的包边界，这个函数尝试通过解析包结构来找到下一个包的起始位置
 * @param data 数据缓冲区
 * @param length 数据长度
 * @param current_offset 当前包的位置
 * @param next_offset 输出：下一个包的位置
 * @return 是否找到下一个包
 */
bool findNextPacket(const uint8_t* data, size_t length, size_t current_offset, size_t& next_offset);

} // namespace opus_analyzer

