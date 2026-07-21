# Building Nexora

## Recommended: GitHub Actions

Push the complete repository, including `.github`, then open the **Actions** tab. The Android, Windows and Linux Mint workflows build their respective artifacts without requiring Android Studio or Visual Studio on your computer.

For a public release, create and push a tag such as `v1.1.0`. The release workflow produces two platform downloads:

- `Nexora-v1.1.0.zip` for Windows and Android
- `Nexora-Linux-Mint-x86_64-v1.1.0.tar.gz` for Linux Mint

The package contains the Windows application, Android APK, ADB, installer scripts, licenses and per-file SHA-256 checksums. GitHub adds its standard source archives automatically.

Tags containing `-beta`, such as `v1.1.0-beta.1`, are published as prereleases.

## Android locally

Requirements: JDK 17 and Android SDK 35.

```powershell
cd android
./gradlew.bat assembleRelease
```

Output: `android/app/build/outputs/apk/release/app-release.apk`.

The repository uses debug signing for reproducible test releases. Before publishing in an app store, create a private release keystore and configure CI secrets. Never commit that keystore.

## Windows locally

Requirements: Visual Studio 2022 Build Tools with Desktop C++, Windows 10/11 SDK and vcpkg packages `asio`, `ffmpeg`, `wxwidgets`, and `qrcodegen` for x64 Windows.

Build `windows/3rdparty/softcam/softcam.sln` in `Release|x64`, then build `windows/Nexora.sln` in `Release|x64`. The project copies runtime files to `dist/`.

## Linux Mint locally

Requirements on Linux Mint 22.x or Ubuntu 24.04: CMake, Ninja, a C++17 compiler, wxWidgets 3.2, Asio and the FFmpeg development libraries. Configure and build with:

```sh
cmake -S windows -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --parallel
```

The application binary is written to `build/linux/Nexora`. The distributable installer scripts are under `linux/`.
