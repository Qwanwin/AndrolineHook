#include "AndroWriteData.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <cinttypes>

namespace AndroWriteData {
    // Inisialisasi variabel statis
    std::map<uintptr_t, std::string> Writer::originalBytesMap;
    std::mutex Writer::writerMutex;

    bool Writer::writeHex(uintptr_t address, const char* hex, bool saveOriginal) {
        std::lock_guard<std::mutex> lock(writerMutex);

        if (!hex) {
            WRITER_LOGE("Invalid hex string: nullptr");
            return false;
        }

        std::vector<uint8_t> bytes = AndroUtils::StringUtils::parseHexString(hex);
        if (bytes.empty()) {
            WRITER_LOGE("Invalid hex string: %s", hex);
            return false;
        }

        if (saveOriginal) {
            saveOriginalBytes(address, bytes.size());
        }

        if (!AndroUtils::MemoryUtils::unprotect(address, bytes.size())) {
            WRITER_LOGE("Failed to unprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
            return false;
        }

        memcpy((void*)address, bytes.data(), bytes.size());
        bool success = AndroUtils::MemoryUtils::reprotect(address, bytes.size());
        if (!success) {
            WRITER_LOGE("Failed to reprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
        }

        return success;
    }

    bool Writer::writeBytes(uintptr_t address, const std::vector<uint8_t>& data, bool saveOriginal) {
        std::lock_guard<std::mutex> lock(writerMutex);

        if (data.empty()) {
            WRITER_LOGE("Empty data provided for write at 0x%" PRIxPTR, address);
            return false;
        }

        if (saveOriginal) {
            saveOriginalBytes(address, data.size());
        }

        if (!AndroUtils::MemoryUtils::unprotect(address, data.size())) {
            WRITER_LOGE("Failed to unprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
            return false;
        }

        memcpy((void*)address, data.data(), data.size());
        bool success = AndroUtils::MemoryUtils::reprotect(address, data.size());
        if (!success) {
            WRITER_LOGE("Failed to reprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
        }

        return success;
    }

    bool Writer::restoreBytes(uintptr_t address) {
        std::lock_guard<std::mutex> lock(writerMutex);

        auto it = originalBytesMap.find(address);
        if (it == originalBytesMap.end()) {
            WRITER_LOGE("No original bytes found for address 0x%" PRIxPTR, address);
            return false;
        }

        std::vector<uint8_t> bytes = AndroUtils::StringUtils::parseHexString(it->second);
        if (bytes.empty()) {
            WRITER_LOGE("Failed to parse original bytes for address 0x%" PRIxPTR, address);
            return false;
        }

        if (!AndroUtils::MemoryUtils::unprotect(address, bytes.size())) {
            WRITER_LOGE("Failed to unprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
            return false;
        }

        memcpy((void*)address, bytes.data(), bytes.size());
        bool success = AndroUtils::MemoryUtils::reprotect(address, bytes.size());
        if (!success) {
            WRITER_LOGE("Failed to reprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
        }

        return success;
    }

    bool Writer::isAddressWritable(uintptr_t address, size_t size) {
        std::lock_guard<std::mutex> lock(writerMutex);

        if (!AndroUtils::MemoryUtils::unprotect(address, size)) {
            WRITER_LOGD("Address 0x%" PRIxPTR " is not writable: %s", address, strerror(errno));
            return false;
        }

        bool success = AndroUtils::MemoryUtils::reprotect(address, size);
        if (!success) {
            WRITER_LOGE("Failed to reprotect memory at 0x%" PRIxPTR ": %s", address, strerror(errno));
        }

        return true;
    }

    void Writer::saveOriginalBytes(uintptr_t address, size_t len) {
        std::lock_guard<std::mutex> lock(writerMutex);
        std::vector<uint8_t> bytes = AndroUtils::MemoryUtils::readMemory(address, len);
        originalBytesMap[address] = AndroUtils::StringUtils::hex2str(bytes.data(), bytes.size());
    }
}