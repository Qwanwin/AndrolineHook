#include "androline.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include "AndroUtils.hpp"
#include "AndroScanner.hpp"

namespace Androline {
    std::string Memory::originalBytes;

    bool Memory::Initialize() {
        return true;
    }

    bool Memory::isLibraryLoaded(const char* libName) {
        void* handle = dlopen(libName, RTLD_LAZY | RTLD_NOLOAD);
        if (handle) {
            dlclose(handle);
            return true;
        }
        return false;
    }

    uintptr_t Memory::getLibraryBase(const char* libName) {
        char line[512];
        FILE* fp = fopen("/proc/self/maps", "rt");
        if (fp != nullptr) {
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, libName)) {
                    fclose(fp);
                    return (uintptr_t)strtoul(line, nullptr, 16);
                }
            }
            fclose(fp);
        }
        return 0;
    }

    uintptr_t Memory::string2Offset(const char* offset) {
        return (uintptr_t)strtoul(offset, nullptr, 16);
    }

    void Memory::SaveOriginalBytes(uintptr_t address, size_t len) {
        originalBytes.clear();
        uint8_t* ptr = (uint8_t*)address;
        for (size_t i = 0; i < len; i++) {
            char byte[3];
            snprintf(byte, sizeof(byte), "%02X", ptr[i]);
            originalBytes += byte;
        }
    }

    bool Memory::WriteHex(uintptr_t address, const char* hex) {
        size_t len = strlen(hex) / 2;
        uint8_t* bytes = new uint8_t[len];
        
        if (!AndroUtils::StringUtils::str2hex(hex, bytes, len)) {
            delete[] bytes;
            return false;
        }

        if (!AndroUtils::MemoryUtils::unprotect(address, len)) {
            delete[] bytes;
            return false;
        }

        memcpy((void*)address, bytes, len);
        AndroUtils::MemoryUtils::reprotect(address, len);
        
        delete[] bytes;
        return true;
    }

    bool Memory::patchOffset(const char* libName, uintptr_t offset, const char* hex, bool restore) {
        uintptr_t base = getLibraryBase(libName);
        if (base == 0) return false;

        uintptr_t address = base + offset;
        
        if (restore) {
            SaveOriginalBytes(address, strlen(hex)/2);
            return WriteHex(address, hex);
        } else {
            return WriteHex(address, originalBytes.c_str());
        }
    }

    bool Memory::patchOffsetSym(uintptr_t addr, const char* hex, bool restore) {
        if (addr == 0) return false;

        if (restore) {
            SaveOriginalBytes(addr, strlen(hex)/2);
            return WriteHex(addr, hex);
        } else {
            return WriteHex(addr, originalBytes.c_str());
        }
    }

    std::vector<uintptr_t> Memory::scanPattern(const char* pattern, const char* mask) {
        return AndroScanner::Scanner::findPattern(pattern, mask);
    }

    std::vector<uintptr_t> Memory::scanHex(const char* hexPattern) {
        return AndroScanner::Scanner::findHex(hexPattern);
    }

    uintptr_t Memory::resolveSymbol(const char* libName, const char* symName) {
        return AndroScanner::SymbolResolver::findSymbol(libName, symName);
    }

    std::string Memory::findLibraryPath(const char* libName) {
        return AndroScanner::SymbolResolver::findLibraryPath(libName);
    }

    void* Memory::robustDlopen(const char* libName, int flags) {
        void* handle = nullptr;
        
        
        handle = dlopen(libName, flags);
        if (handle) return handle;

        std::string fullPath = findLibraryPath(libName);
        if (!fullPath.empty()) {
            handle = dlopen(fullPath.c_str(), flags);
        }
        
        return handle;
    }

    bool Memory::writeMemory(uintptr_t address, const std::vector<uint8_t>& data) {
        return AndroUtils::MemoryUtils::writeMemory(address, data);
    }

    std::vector<uint8_t> Memory::readMemory(uintptr_t address, size_t length) {
        return AndroUtils::MemoryUtils::readMemory(address, length);
    }
}
