#include "androline.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <cinttypes>
#include "AndroUtils.hpp"
#include "AndroScanner.hpp"
#include "AndroWriteData.hpp"

namespace Androline {
    
    std::map<std::string, uintptr_t> Memory::libraryBaseCache;
    std::mutex Memory::memoryMutex;

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
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        auto it = libraryBaseCache.find(libName);
        if (it != libraryBaseCache.end()) {
            return it->second;
        }

        char line[512];
        FILE* fp = fopen("/proc/self/maps", "rt");
        if (fp != nullptr) {
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, libName)) {
                    uintptr_t base = (uintptr_t)strtoul(line, nullptr, 16);
                    fclose(fp);
                    libraryBaseCache[libName] = base;
                    return base;
                }
            }
            fclose(fp);
        }
        ANDROLINE_LOGE("Failed to find base address for %s", libName);
        return 0;
    }

    uintptr_t Memory::string2Offset(const char* offset) {
        if (!offset) {
            ANDROLINE_LOGE("Invalid offset string");
            return 0;
        }
        return (uintptr_t)strtoul(offset, nullptr, 16);
    }

    bool Memory::patchOffset(const char* libName, uintptr_t offset, const char* hex, bool restore) {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        uintptr_t base = getLibraryBase(libName);
        if (base == 0) {
            ANDROLINE_LOGE("Failed to get base address for %s", libName);
            return false;
        }

        uintptr_t address = base + offset;
        
        if (restore) {
            return AndroWriteData::Writer::writeHex(address, hex, true);
        } else {
            return AndroWriteData::Writer::restoreBytes(address);
        }
    }

    bool Memory::patchOffsetSym(uintptr_t addr, const char* hex, bool restore) {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        if (addr == 0) {
            ANDROLINE_LOGE("Invalid symbol address");
            return false;
        }

        if (restore) {
            return AndroWriteData::Writer::writeHex(addr, hex, true);
        } else {
            return AndroWriteData::Writer::restoreBytes(addr);
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
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        void* handle = nullptr;
        dlerror(); 
        handle = dlopen(libName, flags);
        if (handle) {
            return handle;
        }

        ANDROLINE_LOGE("dlopen failed for %s: %s", libName, dlerror());
        std::string fullPath = AndroScanner::SymbolResolver::findLibraryPath(libName);
        if (!fullPath.empty()) {
            handle = dlopen(fullPath.c_str(), flags);
            if (!handle) {
                ANDROLINE_LOGE("dlopen failed for %s: %s", fullPath.c_str(), dlerror());
            }
        }
        
        return handle;
    }

    bool Memory::writeMemory(uintptr_t address, const std::vector<uint8_t>& data) {
        return AndroWriteData::Writer::writeBytes(address, data, true);
    }

    std::vector<uint8_t> Memory::readMemory(uintptr_t address, size_t length) {
        return AndroUtils::MemoryUtils::readMemory(address, length);
    }
}
