#!/bin/sh
set -eu

PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
APK=$(find "$PACKAGE_DIR/apk" -maxdepth 1 -type f -name 'Nexora-Android-*.apk' -print -quit 2>/dev/null || true)

if [ -z "$APK" ]; then
    echo "Nexora Android APK was not found in the apk folder."
    exit 1
fi

if ! command -v adb >/dev/null 2>&1; then
    echo "ADB is not installed. Run ./install.sh first."
    exit 1
fi

adb start-server >/dev/null
if ! adb devices | awk 'NR > 1 && $2 == "device" { found=1 } END { exit !found }'; then
    echo "No authorized Android device found."
    echo "Enable USB debugging, reconnect the cable and accept the prompt on the phone."
    exit 1
fi

adb install -r "$APK"
echo "Nexora Android installed successfully."
