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
            std::vector<MemoryRegion> regions;
            FILE* fp = fopen("/proc/self/maps", "rt");
            if (fp) {
                char line[512];
                while (fgets(line, sizeof(line), fp)) {
                    MemoryRegion region;
                    char perms[5];
                    char path[256];
                    sscanf(line, "%lx-%lx %4s %*s %*s %*s %255s",
                           &region.start, &region.end,
                           perms, path);
                    region.permissions = perms;
                    region.path = path;
                    regions.push_back(region);
                }
                fclose(fp);
            }
            return regions;
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
    };

    class SymbolResolver {
    public:
        static uintptr_t findSymbol(const char* libName, const char* symName) {
            void* handle = dlopen(libName, RTLD_NOW);
            if (!handle) return 0;

            Dl_info info;
            void* symbol = dlsym(handle, symName);
            if (!symbol) {
                dlclose(handle);
                return 0;
            }

            if (dladdr(symbol, &info)) {
                dlclose(handle);
                return (uintptr_t)symbol;
            }

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
            return "";
        }
    };
}