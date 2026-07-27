#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
    echo "Run this uninstaller as root."
    exit 1
fi

if [ -f /usr/local/etc/nexora-webcamd-instance ]; then
    INSTANCE=$(sed -n '1p' /usr/local/etc/nexora-webcamd-instance)
    case "$INSTANCE" in
        *[!0-9]*|'') ;;
        *) sysrc -x "webcamd_${INSTANCE}_flags" >/dev/null 2>&1 || true ;;
    esac
    rm -f /usr/local/etc/nexora-webcamd-instance
    service webcamd restart >/dev/null 2>&1 || true
fi

rm -f /usr/local/bin/nexora
rm -f /usr/local/share/applications/nexora.desktop
rm -rf /usr/local/libexec/nexora

echo "Nexora was removed. webcamd, CUSE, FFmpeg and ADB were preserved."

