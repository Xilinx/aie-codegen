# Migration Guide: AIE-RT to AIE-Codegen

## Overview

The `aie-codegen` repository is derived from `aie-rt` and provides enhanced features for AIE device management. This guide helps users migrate from `aie-rt` to `aie-codegen`.

**Device Support:** Support AIE, AIEML, AIE2, AIE2P, AIE2IPU, AIE2PS(new from aie-rt), and AIE4.

**Key Points:**
- ✅ All existing API signatures remain **unchanged**
- ✅ AIE2PS device support added
- ❌ Lite driver APIs removed (19 APIs)

---

## What Changed

### Removed from xaiengine.h

```c
// These includes are removed:
#include <xaiengine/xaie_lite.h>         // Lite driver removed
#include <xaiengine/xaie_lite_util.h>    // Lite utility removed
```

### API Changes Summary

| Change | Count | Details |
|--------|-------|---------|
| **Total APIs** | 353 → 341 | 12 fewer APIs |
| **Removed** | -19 | Lite driver APIs (not used during compilation flow to generate CDO/Txn/ASM) |
| **Added** | +6 | New functionality (optional to use) |
| **Modified** | 0 | All existing APIs unchanged |

---

## Removed APIs

### Lite Driver (19 APIs) - All Removed

## Device Support

### AIE2PS Support - New in aie-codegen

Compiling models for AIE2PS devices are fully supported with device-specific implementations:

**No code changes needed** - device type is auto-detected during initialization.

---

## Migration Steps

### 1. Check Lite Driver Usage

Search your codebase for lite driver APIs:

```bash
grep -r "XAie_L" your_code/
grep -r "xaie_lite.h" your_code/
```

If found, replace with standard APIs.

### 2. Update Header Includes

**Remove these includes** (no longer available):
- `xaie_lite.h` - lite driver removed
- `xaie_lite_util.h` - lite driver removed

**Continue using the single header:**

```c
#include <xaiengine.h>  // Single header - includes all APIs
```

### 3. Update Build System

- Update include paths: `aie-codegen/src/` instead of `aie-rt/driver/src/`
- Relink against the new `aie-codegen` library
- Recompile your application

### 4. Test

- Verify device initialization
- Test all AIE operations (DMA, events, locks, etc.)
- Validate with your hardware

---

## Example Migration

## Compatibility Note

**All existing APIs have identical signatures** - same parameters, return types, and behavior. 

**Migration impact:**
- ✅ **No lite driver usage?** → Simply update your build system
- ⚠️ **Using lite driver?** → Use lite driver APIs from client-aie-rt repo (apart from LX7 team, nobody should need this!)

---

## Resources

- **Source code:** `/scratch/repos/aie-codegen/src/`
- **Main header:** `xaiengine.h` (single include for all APIs)

For detailed API documentation, refer to the inline comments in the header files.
