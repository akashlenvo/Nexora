@echo off
setlocal enabledelayedexpansion

:: --- Configuration ---
set "SOURCE_DIST=dist"
set "SOURCE_ADB=windows\adb"
set "OUTPUT_DIR=VCamdroid"

:: List of scripts to copy (located in the current root dir)
set "SCRIPTS_TO_COPY=install.bat install_apk.bat uninstall.bat"

echo [INFO] Starting Package Build...

:: 1. Clean previous build
if exist "%OUTPUT_DIR%" (
    echo [INFO] Cleaning old output directory...
    rmdir /s /q "%OUTPUT_DIR%"
)

:: 2. Create new directory structure
echo [INFO] Creating directory structure...
mkdir "%OUTPUT_DIR%"
mkdir "%OUTPUT_DIR%\scripts"
mkdir "%OUTPUT_DIR%\adb"

:: 3. Copy App Executables (contents of /dist -> /vcamdroid)
if exist "%SOURCE_DIST%" (
    echo [INFO] Copying application executables...
    xcopy /s /e /y /q "%SOURCE_DIST%\*" "%OUTPUT_DIR%\"
) else (
    echo [ERROR] Dist folder not found at: %SOURCE_DIST%
    goto :Error
)

:: 4. Copy ADB folder (/windows/adb -> /vcamdroid/adb)
if exist "%SOURCE_ADB%" (
    echo [INFO] Copying ADB binaries...
    xcopy /s /e /y /q "%SOURCE_ADB%\*" "%OUTPUT_DIR%\adb\"
) else (
    echo [ERROR] ADB folder not found at: %SOURCE_ADB%
    goto :Error
)

:: 5. Copy Batch Scripts (root -> /vcamdroid/scripts/)
echo [INFO] Copying batch scripts...
for %%f in (%SCRIPTS_TO_COPY%) do (
    if exist "%%f" (
        copy /y "%%f" "%OUTPUT_DIR%\scripts\%%f" >nul
        echo    - Copied %%f
    ) else (
        echo [WARNING] Script not found: %%f
    )
)

echo.
echo ===================================================
echo [SUCCESS] Package created successfully in: %OUTPUT_DIR%
echo ===================================================
pause
exit /b 0

:Error
echo.
echo [FAIL] An error occurred during packaging.
pause
exit /b 1