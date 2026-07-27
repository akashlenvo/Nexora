#!/bin/sh
set -eu

PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if ! command -v pacman >/dev/null 2>&1; then
    echo "This installer requires Arch Linux or an Arch-based distribution."
    exit 1
fi

if command -v sudo >/dev/null 2>&1; then
    SUDO=sudo
elif [ "$(id -u)" -eq 0 ]; then
    SUDO=
else
    echo "sudo is required to install Nexora and v4l2loopback."
    exit 1
fi

case "$(uname -r)" in
    *-lts*) KERNEL_HEADERS=linux-lts-headers ;;
    *-zen*) KERNEL_HEADERS=linux-zen-headers ;;
    *-hardened*) KERNEL_HEADERS=linux-hardened-headers ;;
    *) KERNEL_HEADERS=linux-headers ;;
esac

echo "Installing Nexora dependencies..."
$SUDO pacman -Syu --needed --noconfirm \
    "$KERNEL_HEADERS" v4l2loopback-dkms v4l2loopback-utils \
    android-tools wxwidgets-gtk3 ffmpeg

echo 'v4l2loopback' | $SUDO tee /etc/modules-load.d/nexora.conf >/dev/null
echo 'options v4l2loopback video_nr=10 card_label="Nexora Virtual Camera" exclusive_caps=1' \
    | $SUDO tee /etc/modprobe.d/nexora.conf >/dev/null

if ! grep -q '^v4l2loopback ' /proc/modules 2>/dev/null; then
    $SUDO modprobe v4l2loopback video_nr=10 card_label="Nexora Virtual Camera" exclusive_caps=1
elif [ ! -e /dev/video10 ]; then
    echo
    echo "v4l2loopback is already loaded with a different configuration."
    echo "Reboot to create /dev/video10 using the Nexora configuration."
fi

$SUDO install -d /opt/nexora
$SUDO install -m 0755 "$PACKAGE_DIR/Nexora" "$PACKAGE_DIR/nexora" /opt/nexora/
$SUDO cp -R "$PACKAGE_DIR/res" /opt/nexora/
$SUDO install -m 0644 "$PACKAGE_DIR/nexora-mark.svg" /opt/nexora/
$SUDO install -m 0644 "$PACKAGE_DIR/nexora.desktop" /usr/share/applications/nexora.desktop
$SUDO ln -sfn /opt/nexora/nexora /usr/local/bin/nexora

echo
echo "Nexora installed successfully."
echo "Run it from the application menu or execute: nexora"
echo "Virtual camera device: /dev/video10"

