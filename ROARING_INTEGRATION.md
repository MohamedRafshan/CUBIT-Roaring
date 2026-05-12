# CUBIT-Roaring Integration

## Status: Adapter Layer Complete ✓

This document describes the Roaring bitmap integration for CUBIT's value bitvector (VB) component.

## What Has Been Implemented

### 1. Roaring-Compatible Bitmap Adapter

- **Location**: `src/bitmap/roaring_vb.h` and `src/bitmap/roaring_vb.cpp`
- **Purpose**: Provides a unified interface for bitmap operations (add, remove, contains, AND, OR, cardinality, serialize/deserialize)
- **Key Features**:
  - **Dual Backend**:
    - Uses CRoaring library when available (`CUBIT_HAS_CROARING` preprocessor flag)
    - Falls back to an efficient sorted vector implementation when CRoaring is unavailable
  - **API**: Standard bitmap operations compatible with the existing WAH-based `ibis::bitvector` interface
  - **Thread-safe by design**: Immutable semantics for readers when used with atomic pointer swaps

### 2. Demo and Validation

- **Location**: `src/bitmap/roaring_bitmap_demo.cpp`
- **Status**: Successfully compiled and tested with g++
- **Output**: `roaring_bitmap_demo ok cardinality=10 bytes=40`
- **Tests**:
  - Intersection (AND) operations
  - Union (OR) operations
  - Serialization/deserialization round-trip
  - Cardinality counting

### 3. CMake Integration

- **Location**: Updated `CMakeLists.txt`
- **Changes**:
  - Added `WITH_CROARING` option (default: ON)
  - Added CRoaring library detection (`find_library`, `find_path`)
  - Added `CUBIT_HAS_CROARING=1` preprocessor definition when library found
  - Added `roaring_bitmap_demo` target
  - Updated `bitmap` library to include adapter files and link against CRoaring (if found)

## Next Steps: Integration with Table Implementations

To use the RoaringVB adapter in the actual CUBIT table implementations, follow these patterns:

### Option 1: Minimal Wrapper (Recommended for UpBit+/UCB+)

Create a thin wrapper that replaces direct `ibis::bitvector*` usage:

```cpp
// In src/ub/table.h or src/ucb/table.h
#ifdef USE_ROARING_BACKEND
    std::vector<cubit::RoaringVB*> bitmaps;
#else
    std::vector<ibis::bitvector*> bitmaps;
#endif
```

Then add conditional compilation branches in the append/update/evaluate methods.

### Option 2: Full Pointer Abstraction (Recommended for Future Refactoring)

Replace all `ibis::bitvector*` with:

```cpp
std::atomic<std::shared_ptr<cubit::RoaringVB>> base_vb;
std::atomic<std::shared_ptr<cubit::RoaringVB>> update_vb;
```

This enables lock-free snapshot semantics and atomic merges (similar to CUBIT v2's approach).

## Building the Project

### On Linux (Recommended)

```bash
# Install dependencies
sudo apt-get install liburcu-dev libboost-all-dev build-essential cmake

# Build
./build.sh
./run.sh core
```

### With CRoaring Support

```bash
# Install CRoaring
sudo apt-get install libroaring-dev

# Build with Roaring enabled
./build.sh  # Will auto-detect and use Roaring
```

### Without External Roaring (Fallback)

```bash
# Build with fallback backend (no external dependency)
./build.sh
```

The build system will automatically detect CRoaring and use it if available; otherwise, it uses the built-in fallback.

## Architecture: How RoaringVB Fits

```
┌─────────────────────────────────────────────────┐
│  Query Execution (table.evaluate)               │
│  - AND/OR operations on bitmaps                 │
│  - Cardinality counting                         │
└────────────────────┬────────────────────────────┘
                     │
                ┌────▼─────────────────────┐
                │  RoaringVB API Layer     │  ← NEW
                │  (cubit::RoaringVB)      │
                ├──────────────────────────┤
                │ add(row_id)              │
                │ remove(row_id)           │
                │ contains(row_id)         │
                │ AND/OR operations        │
                │ serialize/deserialize    │
                └────┬──────────────────┬──┘
                     │                  │
          ┌──────────▼──┐   ┌──────────▼──┐
          │ CRoaring    │   │ Fallback    │
          │ (if avail)  │   │ (sorted vec)│
          └─────────────┘   └─────────────┘
                │                  │
                │   (conditional compilation)
                │
        ┌───────▼──────────────────────┐
        │ Bitmap Storage & Compression │
        │ (byte-level encoding)        │
        └──────────────────────────────┘
```

## Expected Performance Improvements (Over WAH)

Based on typical Roaring vs WAH comparisons:

| Scenario                       | Roaring        | WAH      |
| ------------------------------ | -------------- | -------- |
| Mixed density (sparse + dense) | ✓ Better       | -        |
| Random bit distribution        | ✓ Better       | -        |
| Long runs (RLE-friendly)       | -              | ✓ Better |
| In-memory size                 | ~5-30% smaller | -        |
| Set operation speed (AND/OR)   | ✓ 2-10x faster | -        |
| Serialization overhead         | Small          | -        |

The key win for CUBIT is faster merge operations (avoid decompress-modify-recompress cycles).

## Validation Checklist

- [x] Adapter compiles with fallback backend
- [x] Demo passes unit tests (intersection, union, serialization)
- [x] CMakeLists.txt integrates correctly
- [x] CRoaring detection works (graceful fallback)
- [ ] UpBit+ table uses RoaringVB in place of ibis::bitvector\*
- [ ] UCB+ table uses RoaringVB in place of ibis::bitvector\*
- [ ] Merge performance benchmark shows improvement
- [ ] Memory footprint benchmark shows improvement

## Files Modified/Created

### New Files

- `src/bitmap/roaring_vb.h` - Header with RoaringVB class definition
- `src/bitmap/roaring_vb.cpp` - Implementation (dual backend)
- `src/bitmap/roaring_bitmap_demo.cpp` - Unit test/demo program

### Modified Files

- `CMakeLists.txt` - CRoaring detection and build integration

### Unchanged (Ready for adapter integration)

- `src/ub/table.h` and `src/ub/table.cpp` (UpBit+)
- `src/ucb/table.h` and `src/ucb/table.cpp` (UCB+)
- `src/cubit/table.h` and `src/cubit/table.cpp` (CUBIT v2)
- All FastBit WAH code

## Quick Reference: Adapter API

```cpp
#include "bitmap/roaring_vb.h"
using RoaringVB = cubit::RoaringVB;

// Create
RoaringVB vb;

// Add/Remove/Query
vb.add(42);           // Add bit at position 42
vb.remove(42);        // Remove bit
bool has = vb.contains(42);  // Check membership

// Set Operations
auto result = vb1 & vb2;     // Intersection (AND)
auto result = vb1 | vb2;     // Union (OR)
vb1 &= vb2;                   // In-place AND
vb1 |= vb2;                   // In-place OR

// Metadata
uint64_t count = vb.cardinality();   // Number of set bits
uint64_t bytes = vb.sizeInBytes();   // Serialized size

// I/O
std::vector<uint8_t> data = vb.serialize();   // Serialize
RoaringVB vb2;
vb2.deserialize(data);                        // Deserialize

// Utility
std::vector<uint32_t> bits = vb.toVector();   // Extract all bits
vb.clear();                                    // Clear all bits
```

## Summary

The Roaring bitmap adapter layer is **ready for integration** into CUBIT's table implementations. The dual-backend design ensures compatibility across different deployment scenarios (with or without external CRoaring library). The next phase is to wire this adapter into UpBit+ and UCB+ table code paths and measure the performance improvements.

For a full integration tutorial, see the "Next Steps" section above.
