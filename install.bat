@echo off
setlocal

set "INSTALLER=%~dp0softcam_installer.exe"
set "TARGET=%~dp0softcam.dll"

if not exist "%INSTALLER%" (
  echo ERROR: softcam_installer.exe was not found.
  echo Extract the complete Nexora ZIP before running this file.
  pause
  exit /b 1
)

echo Installing the Nexora virtual camera...
echo.

"%INSTALLER%" register "%TARGET%"

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

