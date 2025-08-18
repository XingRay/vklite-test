//
// Created by leixing on 2025-07-24.
//

#include "FileUtil.h"

#include <fstream>
#include <ios>
#include <iosfwd>
#include <stdexcept>

namespace util {
    std::vector<uint32_t> FileUtil::loadSpvFile(const char *filePath) {
        // 打开二进制文件
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + std::string(filePath));
        }

        // 获取文件大小并验证对齐
        const size_t fileSize = file.tellg();
        if (fileSize == 0) {
            throw std::runtime_error("File is empty: " + std::string(filePath));
        }
        if (fileSize % sizeof(uint32_t) != 0) {
            throw std::runtime_error("SPIR-V file size not aligned to 4 bytes: " + std::string(filePath));
        }

        // 准备缓冲区并读取数据
        file.seekg(0);
        std::vector<uint32_t> spvData(fileSize / sizeof(uint32_t));
        if (!file.read(reinterpret_cast<char *>(spvData.data()), fileSize)) {
            throw std::runtime_error("Failed to read SPIR-V data: " + std::string(filePath));
        }

        // 验证SPIR-V魔数（小端序0x07230203）
        if (spvData.empty() || spvData[0] != 0x07230203) {
            throw std::runtime_error("Invalid SPIR-V magic number: " + std::string(filePath));
        }

        return spvData;
    }
} // util
