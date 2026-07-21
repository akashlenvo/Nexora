# Building Nexora

## Recommended: GitHub Actions

Push the complete repository, including `.github`, then open the **Actions** tab. The Android and Windows workflows build their respective artifacts without requiring Android Studio or Visual Studio on your computer.

For a public release, create and push a tag such as `v1.0.0`. The release workflow produces:

- `Nexora-Android-v1.0.0.apk`
- `Nexora-Windows-x64.zip`
- `Nexora-v1.0.0-source.zip`
- `SHA256SUMS.txt`

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
