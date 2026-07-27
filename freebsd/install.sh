#!/bin/sh
set -eu

PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
DESKTOP_USER=${1:-${SUDO_USER:-${DOAS_USER:-}}}

if [ "$(uname -s)" != "FreeBSD" ]; then
    echo "This installer requires FreeBSD."
    exit 1
fi

if [ "$(id -u)" -ne 0 ]; then
    echo "Run this installer as root and pass the desktop username:"
    echo "  su -"
    echo "  cd $PACKAGE_DIR"
    echo "  ./install.sh $(id -un)"
    exit 1
fi

if [ -z "$DESKTOP_USER" ] || ! id "$DESKTOP_USER" >/dev/null 2>&1; then
    echo "Usage: ./install.sh <desktop-user>"
    echo "Example: ./install.sh junior"
    exit 1
fi

echo "Installing Nexora runtime dependencies..."
pkg install -y webcamd android-tools wx32-gtk3 ffmpeg

if ! kldstat -q -m cuse; then
    kldload cuse
fi
sysrc kld_list+="cuse" >/dev/null
sysrc webcamd_enable="YES" >/dev/null

INSTANCE=0
while sysrc -n "webcamd_${INSTANCE}_flags" >/dev/null 2>&1; do
    INSTANCE=$((INSTANCE + 1))
done
sysrc "webcamd_${INSTANCE}_flags=-c v4l2loopback -v 10" >/dev/null
printf '%s\n' "$INSTANCE" > /usr/local/etc/nexora-webcamd-instance

pw groupmod webcamd -m "$DESKTOP_USER"

service webcamd restart >/dev/null 2>&1 || service webcamd start

install -d /usr/local/libexec/nexora
install -m 0755 "$PACKAGE_DIR/Nexora" "$PACKAGE_DIR/nexora" /usr/local/libexec/nexora/
cp -R "$PACKAGE_DIR/res" /usr/local/libexec/nexora/
install -m 0644 "$PACKAGE_DIR/nexora-mark.svg" /usr/local/libexec/nexora/
install -m 0644 "$PACKAGE_DIR/nexora.desktop" /usr/local/share/applications/nexora.desktop
ln -sfn /usr/local/libexec/nexora/nexora /usr/local/bin/nexora

echo
echo "Nexora installed successfully."
echo "Desktop user: $DESKTOP_USER"
echo "Virtual camera: /dev/video10"
echo "Log out and back in once so the webcamd group membership takes effect."

