#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map> 
#include <mutex>
#include <android/log.h>
#include "AndroUtils.hpp"


#define WRITER_TAG "AndroWriteData"
#define WRITER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, WRITER_TAG, __VA_ARGS__)
#define WRITER_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, WRITER_TAG, __VA_ARGS__)
#define WRITER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, WRITER_TAG, __VA_ARGS__)

namespace AndroWriteData {
    class Writer {
    public:
        static bool writeHex(uintptr_t address, const char* hex, bool saveOriginal = true);
        static bool writeBytes(uintptr_t address, const std::vector<uint8_t>& data, bool saveOriginal = true);
        static bool restoreBytes(uintptr_t address);
        static bool isAddressWritable(uintptr_t address, size_t size);

    private:
        static void saveOriginalBytes(uintptr_t address, size_t len);
        static std::map<uintptr_t, std::string> originalBytesMap;
        static std::mutex writerMutex;
    };
}