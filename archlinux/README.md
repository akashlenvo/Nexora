# Nexora for Arch Linux (Beta)

This untested beta package targets current Arch Linux on x86-64. The source
build is validated automatically, but the installer, virtual camera and
Wi-Fi/USB transports have not yet been tested manually on Arch hardware.

## Install

Extract the archive, open a terminal in the extracted directory and run:

```sh
chmod +x install.sh
./install.sh
```

The installer uses `pacman` to install the matching kernel headers,
`v4l2loopback-dkms`, wxWidgets, FFmpeg and Android tools. It configures
**Nexora Virtual Camera** as `/dev/video10`.

The default Arch, LTS, Zen and Hardened kernels are detected automatically.
Custom kernels require their matching headers to be installed before running
the installer.

Start Nexora from the application menu or execute `nexora`. Install the bundled
Android application with `./install_apk.sh`, or copy the APK from `apk/` to the
phone.

## Diagnostics

```sh
ls -l /dev/video10
v4l2-ctl --list-devices
adb devices
```

If the DKMS module was installed for the first time or was already loaded with
another configuration, reboot once before testing.

## Uninstall

Run `./uninstall.sh`. System packages are intentionally preserved.

