# Alternative Approach: Embed kissfft directly into yvc_core

This document describes an alternative approach to resolve the CMake export dependency issue
by embedding kissfft directly into yvc_core instead of treating it as a separate library.

## Current Issue

The error occurs because:
1. `yvc_core` depends on `kissfft` (linked as PRIVATE)
2. `yvc_core` is exported via `install(EXPORT yvc_coreTargets ...)`
3. CMake requires all dependencies to be part of the export set

## Solution 1: Export kissfft (Implemented)

We've added kissfft to the same export set as yvc_core:

```cmake
# In third_party/kissfft/CMakeLists.txt
install(TARGETS kissfft
    EXPORT yvc_coreTargets  # Same export set
    ...
)
```

## Solution 2: Embed kissfft (Alternative)

If you prefer to keep kissfft completely internal to yvc_core:

### Step 1: Modify yvc_core/CMakeLists.txt

```cmake
# Add kissfft sources directly to yvc_core
set(KISSFFT_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/kiss_fftr.c
)

set(YVC_CORE_SOURCES
    src/AudioBuffer.cpp
    # ... other sources ...
    ${KISSFFT_SOURCES}  # Add kissfft sources
)

# Create library
add_library(yvc_core STATIC ${YVC_CORE_SOURCES} ${YVC_CORE_HEADERS})

# Include kissfft headers privately
target_include_directories(yvc_core
    PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/kissfft
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# No need to link kissfft library
target_link_libraries(yvc_core
    PUBLIC
        Threads::Threads
)
```

### Step 2: Remove kissfft from third_party/CMakeLists.txt

Or keep it but don't require it for yvc_core.

### Pros and Cons

**Solution 1 (Export kissfft - Current):**
- ✅ Maintains separation of concerns
- ✅ kissfft can be shared by multiple targets
- ✅ Easier to update kissfft independently
- ❌ Exposes kissfft in public interface

**Solution 2 (Embed kissfft):**
- ✅ Complete encapsulation (no exposed dependencies)
- ✅ Single library to distribute
- ✅ No export dependency issues
- ❌ kissfft code duplicated if used by multiple libraries
- ❌ Harder to update kissfft

## Recommendation

Use Solution 1 (current implementation) unless:
- You need to distribute yvc_core as a single self-contained library
- You want zero external dependencies in the exported interface
- kissfft will never be used by other components

For most cases, Solution 1 provides better modularity and maintainability.
