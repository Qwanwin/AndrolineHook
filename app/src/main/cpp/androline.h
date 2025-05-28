#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <android/log.h>
#include <dlfcn.h>
#include "AndroWriteData.hpp"


#define ANDROLINE_TAG "Androline"
#define ANDROLINE_LOGI(...) __android_log_print(ANDROID_LOG_INFO, ANDROLINE_TAG, __VA_ARGS__)
#define ANDROLINE_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, ANDROLINE_TAG, __VA_ARGS__)
#define ANDROLINE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, ANDROLINE_TAG, __VA_ARGS__)

#define OBFUSCATE(str) AndroUtils::StringUtils::xor_encrypt(str, sizeof(str) - 1, "secret_key").c_str()

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
        static std::string findLibraryPath(const char* libName);
        static std::map<std::string, uintptr_t> libraryBaseCache;
        static std::mutex memoryMutex;
    };
}
