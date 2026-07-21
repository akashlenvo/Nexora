#!/bin/sh
set -eu

if command -v sudo >/dev/null 2>&1; then
    SUDO=sudo
elif [ "$(id -u)" -eq 0 ]; then
    SUDO=
else
    echo "sudo is required to uninstall Nexora."
    exit 1
fi

$SUDO rm -f /usr/local/bin/nexora
$SUDO rm -f /usr/share/applications/nexora.desktop
$SUDO rm -f /etc/modules-load.d/nexora.conf
$SUDO rm -f /etc/modprobe.d/nexora.conf
$SUDO rm -rf /opt/nexora

echo "Nexora was removed. The system v4l2loopback and ADB packages were preserved."
