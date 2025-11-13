# CMake Export Error Fix

## Problem

```
CMake Error: install(EXPORT "yvc_coreTargets" ...) includes target "yvc_core" 
which requires target "kissfft" that is not in any export set.
```

## Root Cause

When exporting a CMake target, all its dependencies (even PRIVATE ones) must also be part of an export set. Since `yvc_core` links to `kissfft`, CMake requires `kissfft` to be either:

1. Part of the same export set, or
2. A system/external dependency that can be found via `find_dependency()`

## Solution Implemented

Added `kissfft` to the same export set (`yvc_coreTargets`).

### Changes Made

#### 1. `third_party/kissfft/CMakeLists.txt`

Added installation and export configuration:

```cmake
# Add alias for consistent namespacing
add_library(yvc::kissfft ALIAS kissfft)

# Installation (if yvc_core is installed)
include(GNUInstallDirs)

install(TARGETS kissfft
    EXPORT yvc_coreTargets  # Same export set as yvc_core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kissfft
)

install(FILES ${KISSFFT_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kissfft
)
```

#### 2. `yvc_core/CMakeLists.txt`

Updated linking to use generator expressions:

```cmake
target_link_libraries(yvc_core
    PRIVATE
        $<BUILD_INTERFACE:kissfft>
        $<INSTALL_INTERFACE:yvc::kissfft>
    PUBLIC
        Threads::Threads
)
```

This ensures:
- During build: Links to `kissfft` target directly
- After install: Links to `yvc::kissfft` from the exported targets

#### 3. `yvc_core/cmake/yvc_coreConfig.cmake.in`

Added comment clarifying kissfft inclusion:

```cmake
# Note: kissfft is included in the same export set
include("${CMAKE_CURRENT_LIST_DIR}/yvc_coreTargets.cmake")
```

## Verification

### Test the Fix

Run the provided test scripts:

**Linux/macOS:**
```bash
chmod +x scripts/test_cmake_export.sh
./scripts/test_cmake_export.sh
```

**Windows (PowerShell):**
```powershell
.\scripts\test_cmake_export.ps1
```

These scripts will:
1. Build and install yvc_core
2. Create a test project that uses `find_package(yvc_core)`
3. Build and run the test project
4. Verify everything works correctly

### Manual Testing

```bash
# Build and install
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
cmake --install build

# Verify exported files exist
ls /path/to/install/lib/cmake/yvc_core/

# Should see:
# - yvc_coreConfig.cmake
# - yvc_coreConfigVersion.cmake
# - yvc_coreTargets.cmake
# - yvc_coreTargets-*.cmake (per configuration)
```

## Alternative Solutions

See `docs/CMAKE_EXPORT_ALTERNATIVES.md` for other approaches, including:
- Embedding kissfft directly into yvc_core
- Making kissfft a system dependency

## Impact

### Positive
- ✅ CMake install/export now works correctly
- ✅ External projects can use `find_package(yvc_core)`
- ✅ Proper namespacing with `yvc::` prefix

### Considerations
- kissfft is now part of the public interface (but still PRIVATE linked)
- Installing yvc_core also installs kissfft headers to `include/kissfft/`
- Users who only want yvc_core get kissfft as well (minimal overhead)

## Related Files

- `yvc_core/CMakeLists.txt` - Core library build config
- `third_party/kissfft/CMakeLists.txt` - KissFFT build config
- `yvc_core/cmake/yvc_coreConfig.cmake.in` - Package config template
- `scripts/test_cmake_export.sh` - Linux/macOS test script
- `scripts/test_cmake_export.ps1` - Windows test script
- `docs/CMAKE_EXPORT_ALTERNATIVES.md` - Alternative solutions

## References

- [CMake install(EXPORT) documentation](https://cmake.org/cmake/help/latest/command/install.html#export)
- [CMake Packages documentation](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html)
- [Generator expressions](https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html)
