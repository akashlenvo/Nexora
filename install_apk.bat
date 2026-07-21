@echo off
setlocal
echo Installing Nexora on the connected Android device
echo.

set "APK="
for %%F in ("%~dp0apk\Nexora-Android-*.apk") do if exist "%%~fF" set "APK=%%~fF"
if not defined APK (
  echo Nexora APK not found in the apk folder.
  pause
  exit /b 1
)

if not exist "%~dp0adb\adb.exe" (
  echo ADB was not found. Extract the complete Nexora ZIP first.
  pause
  exit /b 1
)

"%~dp0adb\adb.exe" install -r "%APK%"
echo.
pause
