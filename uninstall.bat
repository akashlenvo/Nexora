@echo off
setlocal

set "INSTALLER=%~dp0softcam_installer.exe"
set "TARGET=%~dp0softcam.dll"

echo Uninstalling the Nexora virtual camera...
echo.

"%INSTALLER%" unregister "%TARGET%"

if %ERRORLEVEL% == 0 (
  echo.
  echo Successfully done.
  echo.
) else (
  echo.
  echo The process has been canceled or failed.
  echo.
)

pause
