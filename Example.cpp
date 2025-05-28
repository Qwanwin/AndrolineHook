#include <jni.h>
#include <string>
#include "androline.h"
#include "AndroHook.hpp"
#include "AndroWriteData.hpp"
#include <android/log.h>

#define EXAMPLE_TAG "AndrolineExample"
#define EXAMPLE_LOGI(...) __android_log_print(ANDROID_LOG_INFO, EXAMPLE_TAG, __VA_ARGS__)
#define EXAMPLE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, EXAMPLE_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapp_MainActivity_testAndroline(
        JNIEnv* env,
        jobject /* this */) {
    std::string result = "Androline v0.3.0 Test Results:\n";

    // 1. Initilize Androline
    if (!Androline::Memory::Initialize()) {
        EXAMPLE_LOGE("Failed Androline");
        return env->NewStringUTF("Failed Androline");
    }
    result += "✓ Initialized Androline\n";

    // 2. Obfuscation: Use OBFUSCATE for library names
    const char* targetLibName = OBFUSCATE("libtarget.so");
    EXAMPLE_LOGI("Testing with library: %s", targetLibName);

    // 3. Check if the library is loaded
    if (!Androline::Memory::isLibraryLoaded(targetLibName)) {
        EXAMPLE_LOGE("Library %s not loaded", targetLibName);
        result += "✗ Library not loaded\n";
        return env->NewStringUTF(result.c_str());
    }
    result += "✓ Library loaded\n";

    // 4. Get the base address of the library
    uintptr_t baseAddr = Androline::Memory::getLibraryBase(targetLibName);
    if (baseAddr == 0) {
        EXAMPLE_LOGE("Failed address for %s", targetLibName);
        result += "✗ Failet address\n";
        return env->NewStringUTF(result.c_str());
    }
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "✓ Base address: 0x%" PRIxPTR "\n", baseAddr);
    result += buffer;

    // 5. find path library
    std::string libPath = Androline::Memory::findLibraryPath(targetLibName);
    if (!libPath.empty()) {
        EXAMPLE_LOGI("Library path: %s", libPath.c_str());
        result += "✓ Library path: " + libPath + "\n";
    } else {
        EXAMPLE_LOGE("Failed to find library path for %s", targetLibName);
        result += "✗ Failed to find library path\n";
    }

    // 6. Robust library loading
    void* handle = Androline::Memory::robustDlopen(targetLibName, RTLD_LAZY);
    if (handle) {
        EXAMPLE_LOGI("Successfully opened %s", targetLibName);
        result += "✓ Library opened with robustDlopen\n";
        dlclose(handle);
    } else {
        EXAMPLE_LOGE("Failed to open %s", targetLibName);
        result += "✗ Failed to open library\n";
    }

    // 7. Memory patch with offset
    uintptr_t targetAddr = baseAddr + 0x1234; // offset
    if (AndroWriteData::Writer::isAddressWritable(targetAddr, 2)) {
        if (ANDROHOOK("0x1234", "9090", targetLibName)) { // Patch NOP
            EXAMPLE_LOGI("Successfully patched NOP at 0x%" PRIxPTR, targetAddr);
            result += "✓ Patched NOP at offset 0x1234\n";
        } else {
            EXAMPLE_LOGE("Failed to patch NOP at 0x%" PRIxPTR, targetAddr);
            result += "✗ Failed to patch NOP\n";
        }

        // 8. Restore bytes
        if (AndroWriteData::Writer::restoreBytes(targetAddr)) {
            EXAMPLE_LOGI("Successfully restored bytes at 0x%" PRIxPTR, targetAddr);
            result += "✓ Restored original bytes\n";
        } else {
            EXAMPLE_LOGE("Failed to restore bytes at 0x%" PRIxPTR, targetAddr);
            result += "✗ Failed to restore bytes\n";
        }
    } else {
        EXAMPLE_LOGE("Address 0x%" PRIxPTR " is not writable", targetAddr);
        result += "✗ Address not writable\n";
    }

    // 9. Scan pattern in memory
    std::vector<uintptr_t> scanResults = ANDROHOOK_SCAN("\xDE\xAD\xBE\xEF", "xxxx");
    if (!scanResults.empty()) {
        snprintf(buffer, sizeof(buffer), "✓ Found %zu pattern matches\n", scanResults.size());
        result += buffer;
        for (uintptr_t addr : scanResults) {
            snprintf(buffer, sizeof(buffer), "  - Match at 0x%" PRIxPTR "\n", addr);
            result += buffer;
        }
    } else {
        result += "✗ No pattern matches found\n";
    }

    // 10. Refresh memory regions
    AndroScanner::Scanner::refreshMemoryRegions();
    result += "✓ Refreshed memory regions\n";

    // 11. Resolve simbol
    uintptr_t symAddr = Androline::Memory::resolveSymbol(targetLibName, "someFunction");
    if (symAddr) {
        snprintf(buffer, sizeof(buffer), "✓ Symbol someFunction at 0x%" PRIxPTR "\n", symAddr);
        result += buffer;
    } else {
        result += "✗ Failed to resolve symbol\n";
    }

    return env->NewStringUTF(result.c_str());
}