//
// Created by leixing on 2025-07-24.
//

#pragma once

#include <vector>
#include <stdint.h>

#define  LitterEndian 0
#define  BiggerEndian 1
#define  Endian LitterEndian


namespace util {
    class FileUtil {
    public:
        static std::vector<uint32_t> loadSpvFile(const char *path);
    };
} // util
