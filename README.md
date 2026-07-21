# Nexora

Use your Android phone as a low-latency virtual camera on Windows or Linux Mint — over USB or Wi-Fi.

> Your phone. Your camera. Connected.

Nexora is maintained by **Yves Godoy** and is distributed under the MIT License. It is a renamed and redesigned continuation of the MIT-licensed VCamdroid project; see [ATTRIBUTION.md](ATTRIBUTION.md).

## Features

- USB connection through ADB or wireless connection through QR code
- Hardware-accelerated Android camera streaming over RTSP
- Windows preview with DirectShow virtual camera output
- Linux Mint preview with V4L2 virtual camera output
- Front/back camera switching, flash, zoom, rotation, mirroring and snapshots
- Resolution, frame rate, bitrate, stabilization and image-filter controls
- Defensive protocol parsing that rejects old or malformed clients instead of crashing

## Download and install

Open the repository's **Releases** page and download the package for your computer. Each package contains everything needed for that platform.

### 1. Windows

1. Extract the Windows ZIP to a permanent folder.
2. Open `install.bat` as administrator. This registers the virtual camera.
3. Start `Nexora.exe` and allow it through Windows Firewall when asked.

### 2. Android

You can copy the APK from the `apk` folder to the phone and open it, or connect the phone with USB debugging enabled and run `install_apk.bat`.

### Linux Mint

Download `Nexora-Linux-Mint-x86_64-*.tar.gz`, extract it and run `./install.sh` in a terminal. The installer adds Nexora to the application menu and configures `/dev/video10` as **Nexora Virtual Camera** using `v4l2loopback`.

The Linux package targets Linux Mint 22.x and Ubuntu 24.04 on x86-64. Secure Boot may require approval of the DKMS module during installation; see [linux/README.md](linux/README.md).

### 3. Connect

**Wi-Fi:** keep both devices on the same network, click **Connect with QR** in Nexora Studio and scan the code with the Android app.

**USB:** enable Developer options and USB debugging, connect the cable, accept the authorization prompt and open both apps. Nexora detects the USB route automatically.

Choose the phone under **Video source**. In OBS, Discord, Zoom or another camera app, select the registered virtual camera.

## Important compatibility note

Use the Windows client and Android APK from the **same Nexora release**. An older upstream bundle mixed two incompatible protocol versions and could crash in `VCRUNTIME140.dll` while reading the phone model. Nexora includes the matching Android descriptor and bounds-checked Windows parser.

## Build

You do not need Android Studio or Visual Studio to create official release artifacts locally: GitHub Actions builds Android, Windows and Linux after the source is uploaded. See [docs/BUILDING.md](docs/BUILDING.md).

- Android project: `android/`
- Windows project: `windows/Nexora.sln`
- Linux project: `windows/CMakeLists.txt` and `linux/`
- Brand source and asset generator: `assets/brand/`
- Release workflows: `.github/workflows/`

## License and credits

Nexora modifications: Copyright © 2026 Yves Godoy.

Original VCamdroid code: Copyright © 2026 darusc. DirectShow support uses the separately licensed [softcam](https://github.com/tshino/softcam) submodule by Toshihiko Shimizu. All required notices are preserved in [LICENSE](LICENSE), [ATTRIBUTION.md](ATTRIBUTION.md), and the third-party source tree.
