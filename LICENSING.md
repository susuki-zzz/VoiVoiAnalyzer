# Licensing Information

VoiVoiAnalyzer uses a dual licensing model to maximize flexibility while complying with third-party license requirements.

## Core Library (yvc_core)

**License**: MIT License  
**Location**: `yvc_core/LICENSE`  
**Applies to**:
- `/yvc_core/include/`
- `/yvc_core/src/`
- All core DSP and analysis code

### Why MIT?

The core library is MIT-licensed to:
- Allow integration into any project (commercial or open-source)
- Maximum flexibility for developers
- No viral copyleft requirements
- Compatible with all common open-source licenses

### Permitted Uses

✅ Use in commercial applications  
✅ Use in closed-source software  
✅ Modification and redistribution  
✅ Static or dynamic linking  
✅ No obligation to release source code  

**Requirements**: Attribution required (preserve copyright notice and license text)

## GUI Application (yvc_app)

**License**: GNU General Public License v3.0 (GPLv3)  
**Location**: `LICENSE` (root)  
**Applies to**:
- `/yvc_app/`
- GUI application code
- JUCE-dependent components

### Why GPLv3?

The GUI application uses JUCE Framework, which requires GPLv3 for open-source projects.

### Permitted Uses

✅ Use for any purpose  
✅ Study and modify the source code  
✅ Distribute copies  
✅ Distribute modified versions  

**Requirements**:
- Source code must be made available
- Modifications must be licensed under GPLv3
- Must preserve copyright and license notices
- Must state changes made

### Commercial Use

For commercial distribution **without GPLv3 restrictions**:
1. Obtain a JUCE commercial license from JUCE.com
2. Remove GPLv3 from yvc_app
3. Apply appropriate commercial license terms
4. Core library (yvc_core) remains MIT licensed

## Offline Tool (yvc_offline)

**License**: GPLv3 (currently)  
**Location**: `LICENSE` (root)  
**Rationale**: Consistent with main application licensing

*Note: Could be relicensed to MIT in future if JUCE dependency is removed*

## Third-Party Dependencies

All dependencies are compatible with our dual licensing:

- **KissFFT**: BSD-3-Clause (compatible with MIT and GPLv3)
- **JUCE**: GPLv3 / Commercial (requires GPLv3 for yvc_app)
- **Eigen** (optional): MPL-2.0 (compatible with MIT and GPLv3)

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for details.

## License Compatibility Matrix

| Component | Your Use Case | License Required | Can Use yvc_core? | Can Use yvc_app? |
|-----------|---------------|------------------|-------------------|------------------|
| yvc_core | Any project | MIT | ✅ Yes | N/A |
| yvc_core | Commercial closed-source | MIT | ✅ Yes | N/A |
| yvc_app | Open-source GPLv3 | GPLv3 | ✅ Yes | ✅ Yes |
| yvc_app | Commercial closed-source | JUCE Commercial | ✅ Yes (MIT) | ✅ Yes (w/ JUCE license) |
| Full stack | Open-source GPLv3 | GPLv3 | ✅ Yes | ✅ Yes |
| Full stack | Commercial | JUCE Commercial | ✅ Yes (MIT) | ✅ Yes (w/ JUCE license) |

## Distribution Guidelines

### Distributing Core Library Only (MIT)

1. Include `yvc_core/LICENSE` file
2. Preserve copyright notices in source files
3. No requirement to distribute modifications
4. No requirement for source code availability

### Distributing GUI Application (GPLv3)

1. Include full `LICENSE` (GPLv3) file
2. Make source code available
3. Document any modifications
4. License your modifications under GPLv3
5. Include install/build instructions

### Distributing Both

1. Include both licenses
2. Clearly document which code is under which license
3. Respect GPLv3 requirements for yvc_app
4. Core library remains MIT even when distributed with GPLv3 app

## JUCE Licensing Note

VoiVoiAnalyzer uses JUCE Framework for the GUI application.

**For Open Source (current)**:
- JUCE is used under GPLv3
- yvc_app must be GPLv3
- Source code freely available

**For Commercial Distribution**:
- Obtain JUCE commercial license from juce.com
- yvc_app can use non-GPL license
- yvc_core remains MIT (independent of JUCE)

JUCE License Info: https://juce.com/juce-7-licence/

## Questions?

- **Core library licensing**: MIT allows maximum flexibility
- **GUI app licensing**: GPLv3 (or JUCE commercial for closed-source)
- **Need commercial license?**: Contact JUCE.com for JUCE license
- **Want to use core in proprietary app?**: Yes, MIT allows this
- **Want to modify and not share?**: Core (MIT) yes, App (GPLv3) no

## Summary

- **Maximum Freedom**: Use yvc_core (MIT) in any project
- **Open Source App**: Use yvc_app under GPLv3
- **Commercial App**: Get JUCE license, core stays MIT
- **Clear Separation**: Licensing is modular and clearly defined
