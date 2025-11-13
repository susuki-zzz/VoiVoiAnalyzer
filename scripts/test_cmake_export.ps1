# Test script to verify CMake export/install works correctly on Windows
# Usage: .\scripts\test_cmake_export.ps1

$ErrorActionPreference = "Stop"

Write-Host "=== Testing yvc_core CMake Export/Install ===" -ForegroundColor Cyan

# Create temporary directories
$TempBase = Join-Path $env:TEMP "yvc_test_$(Get-Random)"
$BuildDir = Join-Path $TempBase "build"
$InstallDir = Join-Path $TempBase "install"
$TestProjectDir = Join-Path $TempBase "test_project"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
New-Item -ItemType Directory -Force -Path $TestProjectDir | Out-Null

Write-Host "Build directory: $BuildDir" -ForegroundColor Yellow
Write-Host "Install directory: $InstallDir" -ForegroundColor Yellow
Write-Host "Test project directory: $TestProjectDir" -ForegroundColor Yellow

try {
    # Configure and build
    Write-Host "`nStep 1: Configure yvc_core..." -ForegroundColor Green
    cmake -S . -B $BuildDir `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_INSTALL_PREFIX=$InstallDir `
        -DBUILD_TESTING=OFF `
        -DBUILD_YVC_APP=OFF `
        -DBUILD_YVC_OFFLINE=OFF `
        -GNinja

    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

    Write-Host "`nStep 2: Build yvc_core..." -ForegroundColor Green
    cmake --build $BuildDir --config Release

    if ($LASTEXITCODE -ne 0) { throw "Build failed" }

    Write-Host "`nStep 3: Install yvc_core..." -ForegroundColor Green
    cmake --install $BuildDir --config Release

    if ($LASTEXITCODE -ne 0) { throw "Install failed" }

    Write-Host "`nStep 4: Verify installation..." -ForegroundColor Green
    $ConfigFile = Join-Path $InstallDir "lib\cmake\yvc_core\yvc_coreConfig.cmake"
    $TargetsFile = Join-Path $InstallDir "lib\cmake\yvc_core\yvc_coreTargets.cmake"

    if (-not (Test-Path $ConfigFile)) {
        throw "ERROR: yvc_coreConfig.cmake not found at $ConfigFile"
    }

    if (-not (Test-Path $TargetsFile)) {
        throw "ERROR: yvc_coreTargets.cmake not found at $TargetsFile"
    }

    Write-Host "✓ Config files found" -ForegroundColor Green

    # Create test project
    Write-Host "`nStep 5: Create test project..." -ForegroundColor Green
    
    $CMakeListsContent = @"
cmake_minimum_required(VERSION 3.20)
project(yvc_core_test CXX)

find_package(yvc_core REQUIRED)

add_executable(test_app main.cpp)
target_link_libraries(test_app PRIVATE yvc::yvc_core)
target_compile_features(test_app PRIVATE cxx_std_20)
"@

    $MainCppContent = @"
#include <yvc_core/Logger.h>
#include <iostream>

int main() {
    yvc::LoggerConfig config;
    config.minLevel = yvc::LogLevel::INFO;
    config.enableConsole = true;
    config.enableFile = false;
    
    yvc::Logger::getInstance().configure(config);
    LOG_INFO("Test application using yvc_core via find_package");
    
    std::cout << "Success: yvc_core imported correctly!" << std::endl;
    return 0;
}
"@

    Set-Content -Path (Join-Path $TestProjectDir "CMakeLists.txt") -Value $CMakeListsContent
    Set-Content -Path (Join-Path $TestProjectDir "main.cpp") -Value $MainCppContent

    Write-Host "`nStep 6: Configure test project..." -ForegroundColor Green
    $TestBuildDir = Join-Path $TestProjectDir "build"
    cmake -S $TestProjectDir -B $TestBuildDir `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_PREFIX_PATH=$InstallDir `
        -GNinja

    if ($LASTEXITCODE -ne 0) { throw "Test project configure failed" }

    Write-Host "`nStep 7: Build test project..." -ForegroundColor Green
    cmake --build $TestBuildDir --config Release

    if ($LASTEXITCODE -ne 0) { throw "Test project build failed" }

    Write-Host "`nStep 8: Run test application..." -ForegroundColor Green
    $TestExe = Join-Path $TestBuildDir "test_app.exe"
    & $TestExe

    if ($LASTEXITCODE -ne 0) { throw "Test application failed" }

    Write-Host "`n=== All tests passed! ===" -ForegroundColor Green

    Write-Host "`nInstalled files:" -ForegroundColor Yellow
    Get-ChildItem -Path $InstallDir -Recurse -File | Select-Object -First 20 | ForEach-Object {
        Write-Host "  $($_.FullName.Replace($InstallDir, ''))"
    }

    Write-Host "`n✓ Export/Install test completed successfully" -ForegroundColor Green

} catch {
    Write-Host "`nERROR: $_" -ForegroundColor Red
    exit 1
} finally {
    # Cleanup
    Write-Host "`nCleaning up temporary directories..." -ForegroundColor Yellow
    if (Test-Path $TempBase) {
        Remove-Item -Path $TempBase -Recurse -Force -ErrorAction SilentlyContinue
    }
}
