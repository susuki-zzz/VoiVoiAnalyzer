@echo off
setlocal enabledelayedexpansion

echo ==================================================
echo VoiVoi Analyzer Build Script
echo ==================================================

:: Check if CMake is available
cmake --version >nul 2>&1
if !errorlevel! neq 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.20+ and ensure it's in your PATH
    pause
    exit /b 1
)

:: Parse command line arguments
set BUILD_TYPE=x64-debug
set CLEAN_BUILD=false
set RUN_TESTS=false
set USE_VISUAL_STUDIO=false
set OPEN_VS=false
set CREATE_PACKAGE=false

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--release" (
    set BUILD_TYPE=x64-release
) else if /i "%~1"=="--debug" (
    set BUILD_TYPE=x64-debug
) else if /i "%~1"=="--relwithdebinfo" (
    set BUILD_TYPE=x64-relwithdebinfo
) else if /i "%~1"=="--core-only" (
    set BUILD_TYPE=x64-core-only
) else if /i "%~1"=="--vs" (
    set USE_VISUAL_STUDIO=true
) else if /i "%~1"=="--vs-debug" (
    set BUILD_TYPE=vs2022-debug
    set USE_VISUAL_STUDIO=true
) else if /i "%~1"=="--vs-release" (
    set BUILD_TYPE=vs2022-release
    set USE_VISUAL_STUDIO=true
) else if /i "%~1"=="--open" (
    set OPEN_VS=true
) else if /i "%~1"=="--clean" (
    set CLEAN_BUILD=true
) else if /i "%~1"=="--test" (
    set RUN_TESTS=true
) else if /i "%~1"=="--package" (
    set CREATE_PACKAGE=true
) else if /i "%~1"=="--help" (
    goto :show_help
) else (
    echo Unknown argument: %~1
    goto :show_help
)
shift
goto :parse_args

:args_done

echo Building with preset: !BUILD_TYPE!

:: Clean build if requested
if "!CLEAN_BUILD!"=="true" (
    echo Cleaning build directory...
    if exist "out\build\!BUILD_TYPE!" (
        rmdir /s /q "out\build\!BUILD_TYPE!"
    )
)

:: Configure
echo Configuring project...
cmake --preset=!BUILD_TYPE!
if !errorlevel! neq 0 (
    echo ERROR: Configuration failed
    pause
    exit /b 1
)

:: Build
echo Building project...
if "!USE_VISUAL_STUDIO!"=="true" (
    echo Using MSBuild for Visual Studio projects...
    cmake --build --preset=!BUILD_TYPE! --config Debug
    if !errorlevel! neq 0 (
        echo ERROR: Debug build failed
        pause
        exit /b 1
    )
    cmake --build --preset=!BUILD_TYPE! --config Release
    if !errorlevel! neq 0 (
        echo ERROR: Release build failed
        pause
        exit /b 1
    )
) else (
    cmake --build --preset=!BUILD_TYPE!
    if !errorlevel! neq 0 (
        echo ERROR: Build failed
        pause
        exit /b 1
    )
)

:: Open Visual Studio if requested
if "!OPEN_VS!"=="true" (
    echo Opening Visual Studio...
    if exist "out\build\!BUILD_TYPE!\VoiVoiAnalyzer.sln" (
        start "" "out\build\!BUILD_TYPE!\VoiVoiAnalyzer.sln"
    ) else (
        echo Solution file not found. Opening folder in Visual Studio...
        start devenv .
    )
)

:: Run tests if requested
if "!RUN_TESTS!"=="true" (
    echo Running tests...
    if "!USE_VISUAL_STUDIO!"=="true" (
        ctest --preset=vs-core-tests
    ) else (
        ctest --preset=core-tests
    )
    if !errorlevel! neq 0 (
        echo WARNING: Some tests failed
    )
)

:: Create package if requested
if "!CREATE_PACKAGE!"=="true" (
    echo Creating package using CPack...
    cd out\build\!BUILD_TYPE!
    cpack -G ZIP
    if !errorlevel! equ 0 (
        echo Package created successfully in out\build\!BUILD_TYPE!
    ) else (
        echo WARNING: Package creation failed
    )
    cd ..\..\..
)

echo.
echo ==================================================
echo Build completed successfully!
echo Build type: !BUILD_TYPE!
echo Output directory: out\build\!BUILD_TYPE!
echo ==================================================

if "!BUILD_TYPE!"=="x64-release" (
    echo.
    echo Built executables:
    if exist "out\build\!BUILD_TYPE!\yvc_offline\yvc_offline.exe" (
        echo - yvc_offline.exe ^(Offline analysis tool^)
    )
    if exist "out\build\!BUILD_TYPE!\yvc_app\yvc_app.exe" (
        echo - yvc_app.exe ^(GUI application^)
    )
) else if "!BUILD_TYPE!"=="vs2022-release" (
    echo.
    echo Built executables (Visual Studio):
    if exist "out\build\!BUILD_TYPE!\yvc_offline\Release\yvc_offline.exe" (
        echo - yvc_offline.exe ^(Offline analysis tool^)
    )
    if exist "out\build\!BUILD_TYPE!\yvc_app\Release\yvc_app.exe" (
        echo - yvc_app.exe ^(GUI application^)
    )
)

if "!CREATE_PACKAGE!"=="true" (
    echo.
    echo Package files available in: out\build\!BUILD_TYPE!\
)

pause
exit /b 0

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --debug          Build Debug configuration with Ninja (default)
echo   --release        Build Release configuration with Ninja
echo   --relwithdebinfo Build RelWithDebInfo configuration with Ninja
echo   --core-only      Build only yvc_core library with Ninja
echo   --vs             Use Visual Studio generator (MSBuild)
echo   --vs-debug       Build Debug configuration with Visual Studio
echo   --vs-release     Build Release configuration with Visual Studio
echo   --open           Open project in Visual Studio after build
echo   --clean          Clean build directory before building
echo   --test           Run tests after building
echo   --package        Create package using CPack after successful build
echo   --help           Show this help message
echo.
echo Examples:
echo   build.bat --vs-release --open    ^(Visual Studio Release + open IDE^)
echo   build.bat --release --test       ^(Ninja Release + run tests^)
echo   build.bat --release --package    ^(Build + create package^)
echo   build.bat --vs-debug             ^(Visual Studio Debug^)
echo   build.bat --core-only --clean    ^(Clean + core library only^)
echo.
echo Package creation:
echo   Use --package flag to create ZIP package using CPack
echo   Alternatively, use package.bat for custom packaging with more options
echo.
pause
exit /b 0
