#pragma once
#include "androline.h"
#include <android/log.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>


#define TAG "AndroHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)


#define OBFUSCATE(str) str


#define ANDROHOOK(offset, hex) Androline::Memory::patchOffset(targetLibName, Androline::Memory::string2Offset(OBFUSCATE(offset)), OBFUSCATE(hex), true)
#define ANDROHOOK_LIB(lib, offset, hex) Androline::Memory::patchOffset(OBFUSCATE(lib), Androline::Memory::string2Offset(OBFUSCATE(offset)), OBFUSCATE(hex), true)

#define ANDROHOOK_SYM(sym, hex) Androline::Memory::patchOffset(dlsym(dlopen(targetLibName, 4), OBFUSCATE(sym)), OBFUSCATE(hex), true)
#define ANDROHOOK_LIB_SYM(lib, sym, hex) Androline::Memory::patchOffset(dlsym(dlopen(lib, 4), OBFUSCATE(sym)), OBFUSCATE(hex), true)

#define ANDROHOOK_SWITCH(offset, hex, boolean) Androline::Memory::patchOffset(targetLibName, Androline::Memory::string2Offset(OBFUSCATE(offset)), OBFUSCATE(hex), boolean)
#define ANDROHOOK_LIB_SWITCH(lib, offset, hex, boolean) Androline::Memory::patchOffset(OBFUSCATE(lib), Androline::Memory::string2Offset(OBFUSCATE(offset)), OBFUSCATE(hex), boolean)

#define ANDROHOOK_SYM_SWITCH(sym, hex, boolean) Androline::Memory::patchOffsetSym((uintptr_t)dlsym(dlopen(targetLibName, 4), OBFUSCATE(sym)), OBFUSCATE(hex), boolean)
#define ANDROHOOK_LIB_SYM_SWITCH(lib, sym, hex, boolean) Androline::Memory::patchOffsetSym((uintptr_t)dlsym(dlopen(lib, 4), OBFUSCATE(sym)), OBFUSCATE(hex), boolean)

#define ANDROHOOK_RESTORE(offset) Androline::Memory::patchOffset(targetLibName, Androline::Memory::string2Offset(OBFUSCATE(offset)), "", false)
#define ANDROHOOK_RESTORE_LIB(lib, offset) Androline::Memory::patchOffset(OBFUSCATE(lib), Androline::Memory::string2Offset(OBFUSCATE(offset)), "", false)

#define ANDROHOOK_RESTORE_SYM(sym) Androline::Memory::patchOffsetSym((uintptr_t)dlsym(dlopen(targetLibName, 4), OBFUSCATE(sym)), "", false)
#define ANDROHOOK_RESTORE_LIB_SYM(lib, sym) Androline::Memory::patchOffsetSym((uintptr_t)dlsym(dlopen(lib, 4), OBFUSCATE(sym)), "", false)


inline bool isLibraryLoaded(const char* libName) {
    return Androline::Memory::isLibraryLoaded(libName);
}

inline uintptr_t getLibraryBase(const char* libName) {
    return Androline::Memory::getLibraryBase(libName);
}