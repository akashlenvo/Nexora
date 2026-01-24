@echo off

set INSTALLER=..\softcam_installer.exe
set TARGET=..\softcam.dll

echo Registering softcam.dll to the system using softcam_installer.exe
echo.

%INSTALLER% register %TARGET%

if %ERRORLEVEL% == 0 (
  echo.
  echo Successfully done.
  echo.
) else (
  echo.
  echo The process has been canceled or failed.
  echo.
)

