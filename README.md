# Opus Analyzer

A standalone library and toolset for parsing configuration information of each frame from Opus format files. Supports parsing TOC bytes, encoding modes, audio bandwidth, frame sizes, and other parameters of Opus packets.

## Features

Extract and parse configuration information of each frame from Opus files, outputting relevant information. Current version supports:
- **TOC Byte Parsing**: Parse configuration number, stereo flag, frame count code
- **Encoding Mode Recognition**: SILK-only, Hybrid, CELT-only
- **Audio Bandwidth Recognition**: NB, MB, WB, SWB, FB
- **Frame Size Recognition**: 2.5ms, 5ms, 10ms, 20ms, 40ms, 60ms
- **Packet Type Recognition**: Type 0 (single frame), Type 1 (two equal-sized frames), Type 2 (two different-sized frames), Type 3 (multiple frames)
- **Self-Delimiting Packet Support**: Supports parsing self-delimiting format Opus packets
- **CBR/VBR Support**: Supports both Constant Bitrate (CBR) and Variable Bitrate (VBR) packets
- **Padding Support**: Supports parsing padding bytes in Type 3 packets

## Project Structure

```
opus_analyzer/
├── src/                      # Core parsing code
│   ├── opus_types.h          # Opus data structure definitions
│   ├── opus_utils.h/cpp      # Opus parsing utility functions
│   └── opus_frame_parser.h/cpp # Opus frame parser
├── sample/                   # Sample program
│   ├── opus_sample.cpp       # Opus parsing sample
│   └── CMakeLists.txt
├── script/                   # Build scripts
│   └── build_sample.sh       # Script to build sample program
├── CMakeLists.txt            # Root CMake (build library)
└── README.md
```

## Dependencies

- CMake 3.10+
- C++11 compiler
- No other external dependencies

## Quick Start

### Build Sample Program

Using the provided build script:

```bash
cd opus_analyzer
./script/build_sample.sh
```

Or manually:

```bash
cd opus_analyzer
mkdir build && cd build
cmake ..
make
```

### Run Sample

```bash
cd build/sample
./opus_sample ../../../test.opus
```

## Integration into Other Projects

If you need to integrate the parsing functionality into your own project, you can copy the files from the `src/` directory:

- `src/opus_types.h` - Data structure definitions
- `src/opus_utils.h/cpp` - Utility functions (configuration info retrieval, frame size encoding parsing, etc.)
- `src/opus_frame_parser.h/cpp` - Opus packet parser

Usage example:

```cpp
#include "opus_frame_parser.h"
#include "opus_types.h"

using namespace opus_analyzer;

// Parse Opus packet
OpusFrameInfo frame_info;
if (parseOpusPacket(data, data_size, frame_info)) {
    // Use parsed data
    std::cout << "Encoding Mode: " << getModeString(frame_info.mode) << std::endl;
    std::cout << "Audio Bandwidth: " << getBandwidthString(frame_info.bandwidth) << std::endl;
    std::cout << "Frame Size: " << getFrameSizeString(frame_info.frame_size) << std::endl;
    std::cout << "Frame Count: " << frame_info.frame_count << std::endl;
}
```

## Data Format Description

For detailed information about Opus data structures, please refer to:
- [Opus Audio Encoding Format - Chen Liang's Blog](https://chenliang.org/2020/03/15/opus-format/)
- [Opus Audio Encoding Format - CSDN Blog](https://blog.csdn.net/u011487024/article/details/155133281)

Main field descriptions:
- **Configuration Number (config)**: 0-31, defines encoding mode, audio bandwidth, and frame size
- **Stereo Flag (s)**: 0 for mono, 1 for stereo
- **Frame Count Code (c)**: 0-3, indicates the type of frames contained in the packet
- **Frame Size**: Maximum length of each frame is 1275 bytes

## Notes

- The program supports parsing raw Opus streams
- Supports both self-delimiting packets and regular packets
- For raw Opus streams, there are no explicit boundary markers between packets; the program determines boundaries by parsing packet structures
- If the file is encapsulated in a container format (such as Ogg), you need to extract the raw Opus stream first
- The parsing logic is based on the official libopus implementation to ensure compatibility and correctness
