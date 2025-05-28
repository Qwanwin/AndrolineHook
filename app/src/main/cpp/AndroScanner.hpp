#pragma once
#include <vector>
#include <string>
#include <functional>
#include <link.h>
#include <android/log.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cinttypes> 


#define SCANNER_TAG "AndroScanner"
#define SCANNER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, SCANNER_TAG, __VA_ARGS__)
#define SCANNER_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, SCANNER_TAG, __VA_ARGS__)
#define SCANNER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, SCANNER_TAG, __VA_ARGS__)

namespace AndroScanner {
    struct MemoryRegion {
        uintptr_t start;
        uintptr_t end;
        std::string permissions;
        std::string path;
    };

    class Scanner {
    public:
        static std::vector<MemoryRegion> getMemoryRegions() {
            static std::vector<MemoryRegion> cachedRegions;
            static bool regionsCached = false;

            if (!cachedRegions.empty() && regionsCached) {
                return cachedRegions;
            }

            cachedRegions.clear();
            FILE* fp = fopen("/proc/self/maps", "rt");
            if (fp) {
                char line[512];
                while (fgets(line, sizeof(line), fp)) {
                    MemoryRegion region;
                    char perms[5] = {0};
                    char path[256] = {0};
                    sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %4s %*s %*s %*s %255s",
                           &region.start, &region.end, perms, path);
                    region.permissions = perms;
                    region.path = path;
                    cachedRegions.push_back(region);
                }
                fclose(fp);
            } else {
                SCANNER_LOGE("Failed to open /proc/self/maps: %s", strerror(errno));
            }
            regionsCached = true;
            return cachedRegions;
        }

        static void refreshMemoryRegions() {
            std::vector<MemoryRegion> newRegions;
            FILE* fp = fopen("/proc/self/maps", "rt");
            if (fp) {
                char line[512];
                while (fgets(line, sizeof(line), fp)) {
                    MemoryRegion region;
                    char perms[5] = {0};
                    char path[256] = {0};
                    sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %4s %*s %*s %*s %255s",
                           &region.start, &region.end, perms, path);
                    region.permissions = perms;
                    region.path = path;
                    newRegions.push_back(region);
                }
                fclose(fp);
            } else {
                SCANNER_LOGE("Failed to open /proc/self/maps: %s", strerror(errno));
            }
            cachedRegions = std::move(newRegions);
            regionsCached = true;
        }

        static std::vector<uintptr_t> findPattern(const std::string& pattern, const std::string& mask) {
            std::vector<uintptr_t> results;
            auto regions = getMemoryRegions();
            
            for (const auto& region : regions) {
                if (region.permissions.find('r') != std::string::npos) {
                    for (uintptr_t addr = region.start; addr < region.end - pattern.length(); addr++) {
                        bool found = true;
                        for (size_t i = 0; i < pattern.length(); i++) {
                            if (mask[i] == 'x' && *(char*)(addr + i) != pattern[i]) {
                                found = false;
                                break;
                            }
                        }
                        if (found) {
                            results.push_back(addr);
                        }
                    }
                }
            }
            return results;
        }

        static std::vector<uintptr_t> findHex(const std::string& hexString) {
            std::vector<uintptr_t> results;
            std::string pattern;
            std::string mask;
            
            for (size_t i = 0; i < hexString.length(); i += 2) {
                std::string byte = hexString.substr(i, 2);
                if (byte == "??") {
                    pattern += '\x00';
                    mask += '?';
                } else {
                    pattern += (char)std::stoi(byte, nullptr, 16);
                    mask += 'x';
                }
            }
            
            return findPattern(pattern, mask);
        }

    private:
        static std::vector<MemoryRegion> cachedRegions;
        static bool regionsCached;
    };

    
    std::vector<MemoryRegion> Scanner::cachedRegions;
    bool Scanner::regionsCached = false;

    class SymbolResolver {
    public:
        static uintptr_t findSymbol(const char* libName, const char* symName) {
            void* handle = dlopen(libName, RTLD_NOW);
            if (!handle) {
                SCANNER_LOGE("dlopen failed for %s: %s", libName, dlerror());
                return 0;
            }

            Dl_info info;
            void* symbol = dlsym(handle, symName);
            if (!symbol) {
                SCANNER_LOGE("dlsym failed for %s: %s", symName, dlerror());
                dlclose(handle);
                return 0;
            }

            if (dladdr(symbol, &info)) {
                dlclose(handle);
                return (uintptr_t)symbol;
            }

            SCANNER_LOGE("dladdr failed for %s", symName);
            dlclose(handle);
            return 0;
        }

        static bool isValidLibrary(const char* path) {
            struct stat st;
            if (stat(path, &st) != 0) return false;
            return (st.st_mode & S_IFREG) && (access(path, R_OK) == 0);
        }

        static std::string findLibraryPath(const char* libName) {
            std::vector<std::string> searchPaths = {
                "/system/lib64/",
                "/system/lib/",
                "/vendor/lib64/",
                "/vendor/lib/",
                "/data/app/",
                "/data/data/"
            };

            for (const auto& path : searchPaths) {
                std::string fullPath = path + libName;
                if (isValidLibrary(fullPath.c_str())) {
                    return fullPath;
                }
            }
            SCANNER_LOGE("Library %s not found in search paths", libName);
            return "";
        }
    };
}
