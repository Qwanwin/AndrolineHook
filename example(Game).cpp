#include <jni.h>
#include <pthread.h>
#include "AndroHook.hpp"

// Target library name
const char* targetLibName = "libgame.so";

// Patch 
bool godModeEnabled = false;
bool infiniteAmmoEnabled = false;
bool speedHackEnabled = false;
bool noRecoilEnabled = false;
bool unlockAllEnabled = false;

// Offsets (dont use this offset because example)
const char* HEALTH_OFFSET = "1234";
const char* AMMO_OFFSET = "5678";
const char* SPEED_OFFSET = "9ABC";
const char* RECOIL_OFFSET = "DEF0";
const char* UNLOCK_OFFSET = "4321";


void ApplyGodMode(bool enable) {
    if (enable) {
        
        ANDROHOOK(HEALTH_OFFSET, "C7400864000000"); 
    } else {
        
        ANDROHOOK_RESTORE(HEALTH_OFFSET);
    }
    godModeEnabled = enable;
}


void ApplyInfiniteAmmo(bool enable) {
    if (enable) {
        
        ANDROHOOK(AMMO_OFFSET, "909090909090"); 
    } else {
        
        ANDROHOOK_RESTORE(AMMO_OFFSET);
    }
    infiniteAmmoEnabled = enable;
}


void ApplySpeedHack(bool enable) {
    if (enable) {
       
        ANDROHOOK(SPEED_OFFSET, "C743087F431B40"); 
    } else {
       
        ANDROHOOK_RESTORE(SPEED_OFFSET);
    }
    speedHackEnabled = enable;
}


void ApplyNoRecoil(bool enable) {
    if (enable) {
       
        ANDROHOOK(RECOIL_OFFSET, "0F57C0"); 
    } else {
        
        ANDROHOOK_RESTORE(RECOIL_OFFSET);
    }
    noRecoilEnabled = enable;
}


void ApplyUnlockAll(bool enable) {
    if (enable) {
        
        ANDROHOOK(UNLOCK_OFFSET, "B801000000"); 
    } else {
       
        ANDROHOOK_RESTORE(UNLOCK_OFFSET);
    }
    unlockAllEnabled = enable;
}

void* hack_thread(void*) {
   
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }
    
    LOGI("Target library loaded: %s", targetLibName);
    return NULL;
}

// JNI Functions
extern "C" {
    
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_initializeMod(JNIEnv* env, jobject thiz) {
        pthread_t ptid;
        pthread_create(&ptid, NULL, hack_thread, NULL);
    }
    
   
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_toggleGodMode(JNIEnv* env, jobject thiz, jboolean enable) {
        ApplyGodMode(enable);
    }
    
    
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_toggleInfiniteAmmo(JNIEnv* env, jobject thiz, jboolean enable) {
        ApplyInfiniteAmmo(enable);
    }
    
    
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_toggleSpeedHack(JNIEnv* env, jobject thiz, jboolean enable) {
        ApplySpeedHack(enable);
    }
    
    
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_toggleNoRecoil(JNIEnv* env, jobject thiz, jboolean enable) {
        ApplyNoRecoil(enable);
    }
    
    /
    JNIEXPORT void JNICALL
    Java_com_example_modproject_MainActivity_toggleUnlockAll(JNIEnv* env, jobject thiz, jboolean enable) {
        ApplyUnlockAll(enable);
    }
    
    
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_isGodModeEnabled(JNIEnv* env, jobject thiz) {
        return godModeEnabled;
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_isInfiniteAmmoEnabled(JNIEnv* env, jobject thiz) {
        return infiniteAmmoEnabled;
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_isSpeedHackEnabled(JNIEnv* env, jobject thiz) {
        return speedHackEnabled;
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_isNoRecoilEnabled(JNIEnv* env, jobject thiz) {
        return noRecoilEnabled;
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_isUnlockAllEnabled(JNIEnv* env, jobject thiz) {
        return unlockAllEnabled;
    }
    
    // Custom patch function
    JNIEXPORT jboolean JNICALL
    Java_com_example_modproject_MainActivity_applyCustomPatch(
        JNIEnv* env, 
        jobject thiz, 
        jstring offset, 
        jstring hex
    ) {
        const char* offsetStr = env->GetStringUTFChars(offset, 0);
        const char* hexStr = env->GetStringUTFChars(hex, 0);
        
        bool result = ANDROHOOK(offsetStr, hexStr);
        
        env->ReleaseStringUTFChars(offset, offsetStr);
        env->ReleaseStringUTFChars(hex, hexStr);
        
        return result;
    }
    
    
    JNIEXPORT jint JNICALL
    JNI_OnLoad(JavaVM* vm, void* reserved) {
        return JNI_VERSION_1_6;
    }
}