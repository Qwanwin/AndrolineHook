#pragma once
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <string>
#include <vector>
#include <sstream>

namespace AndroUtils {
    struct ElfW_Sym {
        ElfW(Word) st_name;
        unsigned char st_info;
        unsigned char st_other;
        ElfW(Section) st_shndx;
        ElfW(Addr) st_value;
        ElfW(Xword) st_size;
    };

    class ElfUtils {
    public:
        static uint32_t gnu_hash(const char* name) {
            uint32_t h = 5381;
            for (unsigned char c = *name; c != '\0'; c = *++name) {
                h = h * 33 + c;
            }
            return h;
        }

        static ElfW_Sym* gnu_lookup(const char* name, uint32_t hash, ElfW_Sym* symtab, 
                                  const char* strtab, uint32_t* bucket, uint32_t* chain,
                                  uint32_t gnu_nbucket, uint32_t gnu_symndx) {
            uint32_t n = bucket[hash % gnu_nbucket];
            if (n == 0) {
                return nullptr;
            }

            uint32_t *hasharr = chain + gnu_symndx;
            do {
                if (((hasharr[n] ^ hash) >> 1) == 0) {
                    const char *symname = strtab + symtab[n].st_name;
                    if (strcmp(symname, name) == 0) {
                        return &symtab[n];
                    }
                }
            } while ((hasharr[n++] & 1) == 0);

            return nullptr;
        }
    };

    class MemoryUtils {
    public:
        static bool protect(uintptr_t addr, size_t len, int prot) {
            uintptr_t pageStart = addr & ~(getpagesize() - 1);
            return mprotect((void*)pageStart, len + (addr - pageStart), prot) == 0;
        }

        static bool unprotect(uintptr_t addr, size_t len) {
            return protect(addr, len, PROT_READ | PROT_WRITE | PROT_EXEC);
        }

        static bool reprotect(uintptr_t addr, size_t len) {
            return protect(addr, len, PROT_READ | PROT_EXEC);
        }

        static std::vector<uint8_t> readMemory(uintptr_t address, size_t length) {
            std::vector<uint8_t> buffer(length);
            memcpy(buffer.data(), (void*)address, length);
            return buffer;
        }

        static bool writeMemory(uintptr_t address, const std::vector<uint8_t>& data) {
            if (!unprotect(address, data.size())) return false;
            memcpy((void*)address, data.data(), data.size());
            return reprotect(address, data.size());
        }
    };

    class StringUtils {
    public:
        static std::string hex2str(const uint8_t* data, size_t len) {
            std::string result;
            char hex[3];
            for (size_t i = 0; i < len; i++) {
                snprintf(hex, sizeof(hex), "%02X", data[i]);
                result += hex;
            }
            return result;
        }

        static bool str2hex(const char* hex, uint8_t* output, size_t len) {
            if (strlen(hex) != len * 2) return false;
            
            for (size_t i = 0; i < len; i++) {
                char byte[3] = {hex[i*2], hex[i*2+1], '\0'};
                output[i] = (uint8_t)strtol(byte, nullptr, 16);
            }
            return true;
        }

        static std::vector<uint8_t> parseHexString(const std::string& hex) {
            std::vector<uint8_t> bytes;
            for (size_t i = 0; i < hex.length(); i += 2) {
                std::string byteString = hex.substr(i, 2);
                uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
                bytes.push_back(byte);
            }
            return bytes;
        }
    };
}
