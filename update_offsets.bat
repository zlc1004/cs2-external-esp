@echo off
REM CS2 Offset Updater
REM Downloads cs2-dumper, dumps offsets, and generates offsets.json

setlocal enabledelayedexpansion

echo ========================================
echo CS2 Offset Updater
echo ========================================
echo.

REM Define paths
set "DUMPER_DIR=tmp\cs2-dumper"
set "DUMPER_EXE=%DUMPER_DIR%\cs2-dumper.exe"
set "DUMPER_URL=https://github.com/a2x/cs2-dumper/releases/latest/download/cs2-dumper.exe"
set "OUTPUT_DIR=output"
set "SCRIPTS_DIR=scripts"

REM Create directories if they don't exist
if not exist "tmp" mkdir tmp
if not exist "%DUMPER_DIR%" mkdir "%DUMPER_DIR%"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if not exist "%SCRIPTS_DIR%" mkdir "%SCRIPTS_DIR%"

REM Check if cs2-dumper exists
if not exist "%DUMPER_EXE%" (
    echo [1/4] Downloading cs2-dumper...
    echo URL: %DUMPER_URL%
    
    REM Download using PowerShell
    powershell -Command "try { Invoke-WebRequest -Uri '%DUMPER_URL%' -OutFile '%DUMPER_EXE%' -UseBasicParsing; Write-Host 'Downloaded successfully' } catch { Write-Host 'Error: Failed to download cs2-dumper'; exit 1 }"
    
    if !errorlevel! neq 0 (
        echo Error: Failed to download cs2-dumper!
        echo Please download manually from: https://github.com/a2x/cs2-dumper/releases
        pause
        exit /b 1
    )
    
    echo Done!
    echo.
) else (
    echo [1/4] cs2-dumper already exists, skipping download
    echo.
)

REM Check if CS2 is running
echo [2/4] Checking if CS2 is running...
tasklist /FI "IMAGENAME eq cs2.exe" 2>NUL | find /I /N "cs2.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo CS2 is running - ready to dump!
) else (
    echo WARNING: CS2 is not running!
    echo Please start CS2 before running the dumper.
    echo.
    set /p "CONTINUE=Continue anyway? (y/n): "
    if /i "!CONTINUE!" neq "y" (
        echo Cancelled by user
        pause
        exit /b 1
    )
)
echo.

REM Run cs2-dumper
echo [3/4] Running cs2-dumper...
echo Output directory: %OUTPUT_DIR%
echo.

REM Change to dumper directory and run
cd "%DUMPER_DIR%"
cs2-dumper.exe
set "DUMP_RESULT=!errorlevel!"
cd ..\..

if !DUMP_RESULT! neq 0 (
    echo.
    echo Warning: cs2-dumper exited with code !DUMP_RESULT!
    echo This might be normal if CS2 isn't running.
    echo.
)

REM Check if output files exist
if not exist "%OUTPUT_DIR%\offsets.json" (
    echo Error: Output files not found!
    echo cs2-dumper may have failed to generate offsets.
    pause
    exit /b 1
)

echo Done!
echo.

REM Run Python script to extract offsets
echo [4/4] Extracting offsets with Python...
echo.

python --version >NUL 2>&1
if !errorlevel! neq 0 (
    echo Error: Python not found!
    echo Please install Python 3 and add it to PATH.
    echo Download from: https://www.python.org/downloads/
    pause
    exit /b 1
)

python "%SCRIPTS_DIR%\extract_offsets.py"
if !errorlevel! neq 0 (
    echo.
    echo Error: Failed to extract offsets!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Offset update complete!
echo ========================================
echo.
echo Generated files:
echo   - %OUTPUT_DIR%\*.json (cs2-dumper output)
echo   - .github\offsets.json (extracted offsets)
echo.
echo You can now commit .github\offsets.json to your repository.
echo.

pause
