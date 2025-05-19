#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <android/log.h>
#include <dlfcn.h>

#define TAG "Androline"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define OBFUSCATE(str) str

namespace Androline {
    class Memory {
    public:
        static bool Initialize();
        static bool isLibraryLoaded(const char* libName);
        static uintptr_t getLibraryBase(const char* libName);
        static uintptr_t string2Offset(const char* offset);
        static bool patchOffset(const char* libName, uintptr_t offset, const char* hex, bool restore);
        static bool patchOffsetSym(uintptr_t addr, const char* hex, bool restore);
        static std::vector<uintptr_t> scanPattern(const char* pattern, const char* mask);
        static std::vector<uintptr_t> scanHex(const char* hexPattern);
        static uintptr_t resolveSymbol(const char* libName, const char* symName);
        static void* robustDlopen(const char* libName, int flags);
        static bool writeMemory(uintptr_t address, const std::vector<uint8_t>& data);
        static std::vector<uint8_t> readMemory(uintptr_t address, size_t length);
        
    private:
        static bool WriteHex(uintptr_t address, const char* hex);
        static void SaveOriginalBytes(uintptr_t address, size_t len);
        static std::string originalBytes;
        static std::string findLibraryPath(const char* libName);
    };
}
