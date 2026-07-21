# Nexora

Use your Android phone as a low-latency virtual camera on Windows — over USB or Wi-Fi.

> Your phone. Your camera. Connected.

Nexora is maintained by **Yves Godoy** and is distributed under the MIT License. It is a renamed and redesigned continuation of the MIT-licensed VCamdroid project; see [ATTRIBUTION.md](ATTRIBUTION.md).

## Features

- USB connection through ADB or wireless connection through QR code
- Hardware-accelerated Android camera streaming over RTSP
- Windows preview and DirectShow virtual camera output
- Front/back camera switching, flash, zoom, rotation, mirroring and snapshots
- Resolution, frame rate, bitrate, stabilization and image-filter controls
- Defensive protocol parsing that rejects old or malformed clients instead of crashing

## Download and install

Download `Nexora-Windows-x64.zip` and `Nexora-Android-v1.0.0.apk` from the repository's **Releases** page.

### 1. Windows

1. Extract the Windows ZIP to a permanent folder.
2. Open `scripts/install.bat` as administrator. This registers the virtual camera.
3. Start `Nexora.exe` and allow it through Windows Firewall when asked.

### 2. Android

You can copy the APK to the phone and open it, or connect the phone with USB debugging enabled and run `scripts/install_apk.bat`.

### 3. Connect

**Wi-Fi:** keep both devices on the same network, click **Connect with QR** in Nexora Studio and scan the code with the Android app.

**USB:** enable Developer options and USB debugging, connect the cable, accept the authorization prompt and open both apps. Nexora detects the USB route automatically.

Choose the phone under **Video source**. In OBS, Discord, Zoom or another camera app, select the registered virtual camera.

## Important compatibility note

Use the Windows client and Android APK from the **same Nexora release**. An older upstream bundle mixed two incompatible protocol versions and could crash in `VCRUNTIME140.dll` while reading the phone model. Nexora v1.0.0 includes the matching Android descriptor and bounds-checked Windows parser.

## Build

You do not need Android Studio or Visual Studio to create official release artifacts locally: GitHub Actions builds both platforms after the source is uploaded. See [docs/BUILDING.md](docs/BUILDING.md).

- Android project: `android/`
- Windows project: `windows/Nexora.sln`
- Brand source and asset generator: `assets/brand/`
- Release workflows: `.github/workflows/`

## License and credits

Nexora modifications: Copyright © 2026 Yves Godoy.

Original VCamdroid code: Copyright © 2026 darusc. DirectShow support uses the separately licensed [softcam](https://github.com/tshino/softcam) submodule by Toshihiko Shimizu. All required notices are preserved in [LICENSE](LICENSE), [ATTRIBUTION.md](ATTRIBUTION.md), and the third-party source tree.
