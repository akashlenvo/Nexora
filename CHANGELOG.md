# Changelog

## Nexora 1.1.1 — 2026-07-21

- Refreshed the Nexora brand mark and removed the green accent circle
- Updated the Windows executable icon and every Android launcher icon
- Updated the Linux package logo and application menu artwork
- Added an animated Nexora Studio preview to the project README
- Redesigned the README with clearer features, supported systems and installation instructions
- No protocol or compatibility changes; desktop clients and Android APK remain fully compatible within this release

## Nexora 1.1.0 — 2026-07-21

- Added stable Linux Mint 22.x and Ubuntu 24.04 support for x86-64
- Added a native V4L2 output backend using `v4l2loopback` and `/dev/video10`
- Added Wi-Fi and USB/ADB transport support to the Linux desktop client
- Added a CMake build, Linux CI workflow and automated release packaging
- Added a Linux installer that configures the virtual camera and installs wxWidgets, FFmpeg, ADB and V4L2 runtime dependencies
- Added `install_apk.sh` and bundled the matching Android APK in the Linux release
- Added Linux application-menu integration, launcher, uninstaller and SHA-256 checksums
- Added compatibility with wxWidgets 3.2 and explicit portable C++ dependencies
- Preserved the existing Windows DirectShow backend and verified Windows builds after the cross-platform changes
- Release downloads are now separated into one Windows+Android ZIP and one Linux Mint+Android archive

## Nexora 1.0.0

- Complete product rename to Nexora by Yves Godoy
- New dark desktop dashboard with branded header, connection status, larger preview and grouped camera controls
- New Android onboarding screen, package identifier and Nexora icon set
- Fixed the upstream Windows/Android release mismatch that caused access violation `0xc0000005` in `VCRUNTIME140.dll`
- Added bounds checking for device descriptors and error reports
- Added support for TCP descriptors arriving in multiple segments
- Added GitHub Actions builds for Android, Windows and tagged releases
- Added Portuguese publishing guide, build documentation and preserved MIT attribution
