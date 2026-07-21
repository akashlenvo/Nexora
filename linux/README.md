# Nexora for Linux Mint

This beta package targets Linux Mint 22.x and Ubuntu 24.04 on x86-64.

## Install

Extract the archive, open a terminal in the extracted folder and run:

```sh
chmod +x install.sh
./install.sh
```

The installer adds the application under `/opt/nexora`, installs ADB and
`v4l2loopback`, and configures **Nexora Virtual Camera** as `/dev/video10`.

Start Nexora from the application menu or run `nexora`. In OBS, Discord,
Chrome or another video application, select **Nexora Virtual Camera**.

For USB mode, enable Android developer options and USB debugging. Wi-Fi mode
uses the same QR code flow as Windows.

## Secure Boot

Secure Boot can prevent the DKMS-built `v4l2loopback` module from loading.
Linux Mint may ask you to enroll a Machine Owner Key during installation. If
the virtual camera is missing after reboot, check `sudo modprobe v4l2loopback`
and follow the operating system instructions for signing DKMS modules.

## Custom device

Nexora defaults to `/dev/video10`. To use another loopback device:

```sh
NEXORA_VIDEO_DEVICE=/dev/video20 nexora
```

## Uninstall

Run `./uninstall.sh`. System packages are intentionally left installed.
