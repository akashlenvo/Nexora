@echo off
echo Installing Nexora on the connected Android device

set "APK="
for %%F in ("..\apk\Nexora-Android-*.apk") do set "APK=%%~fF"
if not defined APK (
  echo Nexora APK not found in the apk folder.
  exit /b 1
)

..\adb\adb.exe install -r "%APK%"
