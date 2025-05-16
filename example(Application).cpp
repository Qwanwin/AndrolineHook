#include <jni.h>
#include "AndroHook.hpp"

// lib target
const char* targetLibName = "libapp.so";


void UnlockPremium() {
    // Patch bypass check premium
    ANDROHOOK("1234", "B801000000"); // mov eax, 1 (return true)
    // or
    ANDROHOOK("1234", "909090"); // NOP (skip check)
}

// Function  restore
void RestorePremium() {
    ANDROHOOK_RESTORE("1234");
}


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }
    
   
    UnlockPremium();
    
    return JNI_VERSION_1_6;
}