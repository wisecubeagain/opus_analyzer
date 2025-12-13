/*
 * Opus Frame Parser
 * Opus 帧解析器
 */

#pragma once

#include "opus_types.h"
#include <stdint.h>
#include <stddef.h>

namespace opus_analyzer {

/**
 * 解析 Opus 包
 * @param data Opus 包数据
 * @param length 数据长度
 * @param frame_info 输出：解析后的帧信息
 * @return 是否解析成功
 */
bool parseOpusPacket(const uint8_t* data, size_t length, OpusFrameInfo& frame_info);

/**
 * 解析 TOC 字节
 * @param toc TOC 字节
 * @param config 输出：配置数
 * @param stereo 输出：是否立体声
 * @param frame_count_code 输出：帧数代码
 * @return 是否成功
 */
bool parseTOC(uint8_t toc, uint8_t& config, bool& stereo, uint8_t& frame_count_code);

} // namespace opus_analyzer

