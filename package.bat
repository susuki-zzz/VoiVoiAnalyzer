@echo off
setlocal enabledelayedexpansion

echo ==================================================
echo VoiVoi Analyzer Package Script
echo ==================================================

:: Parse command line arguments
set BUILD_TYPE=x64-release
set OUTPUT_DIR=%~dp0dist
set INCLUDE_SOURCES=false
set PACKAGE_NAME=VoiVoiAnalyzer
set VERSION=0.1.0

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--debug" (
    set BUILD_TYPE=x64-debug
) else if /i "%~1"=="--release" (
    set BUILD_TYPE=x64-release
) else if /i "%~1"=="--vs-release" (
    set BUILD_TYPE=vs2022-release
) else if /i "%~1"=="--output" (
    set OUTPUT_DIR=%~2
    shift
) else if /i "%~1"=="--include-sources" (
    set INCLUDE_SOURCES=true
) else if /i "%~1"=="--version" (
    set VERSION=%~2
    shift
) else if /i "%~1"=="--help" (
    goto :show_help
) else (
    echo Unknown argument: %~1
    goto :show_help
)
shift
goto :parse_args

:args_done

set PACKAGE_FULL_NAME=!PACKAGE_NAME!-!VERSION!-win64
set BUILD_DIR=out\build\!BUILD_TYPE!

echo Package configuration:
echo - Build type: !BUILD_TYPE!
echo - Version: !VERSION!
echo - Output directory: !OUTPUT_DIR!
echo - Package name: !PACKAGE_FULL_NAME!
echo.

:: Check if build exists
if not exist "!BUILD_DIR!" (
    echo ERROR: Build directory !BUILD_DIR! not found
    echo Please run build.bat first to create the build
    pause
    exit /b 1
)

:: Create package directory
if exist "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!" (
    echo Cleaning existing package directory...
    rmdir /s /q "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!"
)
mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!" 2>nul

:: Copy executables
echo Copying executables...
if "!BUILD_TYPE!"=="vs2022-release" (
    if exist "!BUILD_DIR!\yvc_app\Release\yvc_app.exe" (
        copy "!BUILD_DIR!\yvc_app\Release\yvc_app.exe" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_app.exe copied
    )
    if exist "!BUILD_DIR!\yvc_offline\Release\yvc_offline.exe" (
        copy "!BUILD_DIR!\yvc_offline\Release\yvc_offline.exe" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_offline.exe copied
    )
    if exist "!BUILD_DIR!\yvc_core\Release\yvc_core.lib" (
        copy "!BUILD_DIR!\yvc_core\Release\yvc_core.lib" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_core.lib copied
    )
) else (
    if exist "!BUILD_DIR!\yvc_app\yvc_app.exe" (
        copy "!BUILD_DIR!\yvc_app\yvc_app.exe" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_app.exe copied
    )
    if exist "!BUILD_DIR!\yvc_offline\yvc_offline.exe" (
        copy "!BUILD_DIR!\yvc_offline\yvc_offline.exe" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_offline.exe copied
    )
    if exist "!BUILD_DIR!\yvc_core\yvc_core.lib" (
        copy "!BUILD_DIR!\yvc_core\yvc_core.lib" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\"
        echo - yvc_core.lib copied
    )
)

:: Copy documentation
echo Copying documentation...
copy "README.md" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\" >nul 2>&1
copy "LICENSE" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\" >nul 2>&1
if exist "docs\" (
    mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\docs" 2>nul
    xcopy "docs\*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\docs\" /s /q >nul 2>&1
    echo - Documentation copied
)

:: Copy headers for core library
echo Copying headers...
if exist "yvc_core\include\" (
    mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\include" 2>nul
    xcopy "yvc_core\include\*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\include\" /s /q >nul 2>&1
    echo - Core library headers copied
)

:: Copy example files
if exist "examples\" (
    echo Copying examples...
    mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\examples" 2>nul
    xcopy "examples\*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\examples\" /s /q >nul 2>&1
    echo - Examples copied
)

:: Copy third-party licenses
if exist "third_party\" (
    echo Copying third-party licenses...
    mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\third_party_licenses" 2>nul
    for /d %%d in (third_party\*) do (
        if exist "%%d\LICENSE*" (
            copy "%%d\LICENSE*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\third_party_licenses\%%~nxd-LICENSE.txt" >nul 2>&1
        )
    )
    echo - Third-party licenses copied
)

:: Include sources if requested
if "!INCLUDE_SOURCES!"=="true" (
    echo Including source code...
    mkdir "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\src" 2>nul
    xcopy "yvc_core\src\*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\src\yvc_core\" /s /q >nul 2>&1
    xcopy "yvc_core\include\*" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\src\yvc_core\include\" /s /q >nul 2>&1
    copy "CMakeLists.txt" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\src\" >nul 2>&1
    copy "yvc_core\CMakeLists.txt" "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\src\yvc_core\" >nul 2>&1
    echo - Source code included
)

:: Create version info
echo Creating version information...
echo VoiVoi Analyzer v!VERSION! > "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\VERSION.txt"
echo Build type: !BUILD_TYPE! >> "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\VERSION.txt"
echo Build date: %DATE% %TIME% >> "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\VERSION.txt"
echo Platform: Windows x64 >> "!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\VERSION.txt"

:: Create ZIP archive
echo Creating ZIP archive...
set ZIP_NAME=!PACKAGE_FULL_NAME!.zip
if exist "!OUTPUT_DIR!\!ZIP_NAME!" del "!OUTPUT_DIR!\!ZIP_NAME!"

:: Use PowerShell to create ZIP (available on Windows 10+)
powershell -Command "Compress-Archive -Path '!OUTPUT_DIR!\!PACKAGE_FULL_NAME!\*' -DestinationPath '!OUTPUT_DIR!\!ZIP_NAME!'" >nul 2>&1

if exist "!OUTPUT_DIR!\!ZIP_NAME!" (
    echo ZIP archive created: !ZIP_NAME!
    
    :: Get file size
    for %%A in ("!OUTPUT_DIR!\!ZIP_NAME!") do set SIZE=%%~zA
    echo Archive size: !SIZE! bytes
) else (
    echo WARNING: Failed to create ZIP archive. Files are available in: !OUTPUT_DIR!\!PACKAGE_FULL_NAME!\
)

echo.
echo ==================================================
echo Package created successfully!
echo Package directory: !OUTPUT_DIR!\!PACKAGE_FULL_NAME!\
if exist "!OUTPUT_DIR!\!ZIP_NAME!" echo ZIP archive: !OUTPUT_DIR!\!ZIP_NAME!
echo ==================================================

pause
exit /b 0

:show_help
echo Usage: package.bat [options]
echo.
echo Options:
echo   --debug              Package Debug build
echo   --release            Package Release build (default)
echo   --vs-release         Package Visual Studio Release build
echo   --output DIR         Set output directory (default: dist)
echo   --version VERSION    Set package version (default: 0.1.0)
echo   --include-sources    Include source code in package
echo   --help               Show this help message
echo.
echo Examples:
echo   package.bat --release
echo   package.bat --vs-release --output C:\releases
echo   package.bat --release --include-sources --version 1.0.0
echo.
pause
exit /b 0
