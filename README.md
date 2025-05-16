# AndrolineHook
AndrolineHook is a lightweight C++ library for runtime memory hooking and patching in Android apps. It supports library detection, offset modification, and original byte restoration.

## 📋 Requirements

- Android NDK
- C++ Standard Library
- AndroUtils.hpp (included)

## 🔍 Example Usage

```cpp
#include <jni.h>
#include "AndroHook.hpp"

// Target library
const char* targetLibName = "libapp.so";

void UnlockPremium() {
    // Patch to bypass premium check
    ANDROHOOK("1234", "B801000000"); // mov eax, 1 (return true)
    // or
    ANDROHOOK("1234", "909090"); // NOP (skip check)
}

void RestorePremium() {
    ANDROHOOK_RESTORE("1234"); // Restore original bytes
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    while (!isLibraryLoaded(targetLibName)) {
        sleep(1);
    }
    UnlockPremium();
    return JNI_VERSION_1_6;
}
```

## 🤝 Contributing

We welcome contributions! Here's how you can help:

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct.

## ⚠️ Disclaimer

AndrolineHook is provided for **educational and research purposes only**. The authors are not responsible for any misuse or damage caused by this software. Use responsibly and ensure compliance with applicable laws and terms of service.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📬 Contact

Develop - [@Qwanwin](https://t.me/Qwanwin)

Project Link: [https://github.com/Qwanwin/AndrolineHook](https://github.com/Qwanwin/AndrolineHook)
```
