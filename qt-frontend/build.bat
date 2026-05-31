@echo off
REM CINEBASE Qt C++ Build and Run Script (Windows)

setlocal enabledelayedexpansion

REM Get script directory
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set BUILD_DIR=%SCRIPT_DIR%build

echo.
echo ╔════════════════════════════════════╗
echo ║         CINEBASE Build Script      ║
echo ║           (Windows)                ║
echo ╚════════════════════════════════════╝
echo.

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo CMake not found. Please install CMake from https://cmake.org/
    echo.
    pause
    exit /b 1
)

REM Create build directory
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

REM Configure CMake
echo Configuring CMake...
cd /d "%BUILD_DIR%"
cmake .. -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

REM Build
echo.
echo Building CINEBASE...
cmake --build . --config Release --parallel

if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo Build successful!
echo.
echo Build artifacts:
echo   Executable: %BUILD_DIR%\Release\CINEBASE.exe
echo.

REM Ask to run
set /p RUN="Would you like to run CINEBASE now? (y/n) "
if /i "%RUN%"=="y" (
    echo Starting CINEBASE...
    "%BUILD_DIR%\Release\CINEBASE.exe"
)
