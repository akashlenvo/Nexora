#!/bin/sh
set -eu

PACKAGE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
APK=

for candidate in "$PACKAGE_DIR"/apk/Nexora-Android-*.apk; do
    if [ -f "$candidate" ]; then
        APK=$candidate
        break
    fi
done

if [ -z "$APK" ]; then
    echo "Nexora Android APK was not found in the apk folder."
    exit 1
fi

if ! command -v adb >/dev/null 2>&1; then
    echo "ADB is not installed. Run install.sh first."
    exit 1
fi

adb start-server >/dev/null
if ! adb devices | awk 'NR > 1 && $2 == "device" { found=1 } END { exit !found }'; then
    echo "No authorized Android device found."
    echo "Attach the phone to the FreeBSD guest, enable USB debugging and accept the phone prompt."
    exit 1
fi

adb install -r "$APK"
echo "Nexora Android installed successfully."

