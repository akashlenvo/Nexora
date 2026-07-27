# Nexora for FreeBSD

This package targets FreeBSD 15.1 on amd64.

## Install

Extract the archive and open a terminal in the extracted directory. Become
root, return to that directory and run the installer with your desktop
username:

```sh
su -
cd /path/to/Nexora-FreeBSD-amd64
./install.sh junior
```

The installer adds the application under `/usr/local/libexec/nexora`, installs
wxWidgets, FFmpeg, ADB and webcamd, loads CUSE, and creates a V4L2 loopback
camera as `/dev/video10`. Log out and back in once after installation so the
new `webcamd` group membership takes effect.

Start Nexora from the application menu or run `nexora`. Select the device named
**Nexora Virtual Camera** or `/dev/video10` in a compatible application.

## WiFi

Use bridged networking when testing in VirtualBox. Keep the phone and FreeBSD
guest on the same local network, open **Connect with QR** and scan the QR code
with the matching Nexora Android application.

## USB

Enable Android developer options and USB debugging. Attach the phone to the
FreeBSD guest from VirtualBox's **Devices > USB** menu, accept the authorization
prompt on the phone and verify it with:

```sh
adb devices
```

Install the bundled Android application with `./install_apk.sh`. Nexora creates
the required ADB tunnels when USB mode is selected.

## Diagnostics

Verify the virtual camera and its permissions with:

```sh
ls -l /dev/video10
service webcamd status
kldstat -q -m cuse
```

Use another V4L2 output device by setting `NEXORA_VIDEO_DEVICE` before starting
the application.

## Uninstall

Run `./uninstall.sh` as root. Runtime packages are intentionally preserved.
