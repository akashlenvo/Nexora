@echo off

set INSTALLER=..\softcam_installer.exe
set TARGET=..\softcam.dll

echo Uninstalling softcam.dll from system system using softcam_installer.exe
echo.

%INSTALLER% unregister %TARGET%

if %ERRORLEVEL% == 0 (
  echo.
  echo Successfully done.
  echo.
) else (
  echo.
  echo The process has been canceled or failed.
  echo.
)
