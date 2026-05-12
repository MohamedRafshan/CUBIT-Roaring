# Quick Start: Building CUBIT-Roaring on Linux

This guide shows how to build and run CUBIT with the new Roaring bitmap adapter.

## Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    liburcu-dev \
    python3 \
    git

# Optional: Install CRoaring for native Roaring support
# (Without it, the fallback backend will be used)
sudo apt-get install -y libroaring-dev
```

## Building CUBIT with Roaring Adapter

```bash
cd CUBIT-main

# Clean and build
./build.sh

# Or manually:
rm -rf build && mkdir build && cd build
cmake ..
make -j4 VERBOSE=1
cd ..
```

The build system will automatically:

- Detect CRoaring library (if installed)
- Use it if found, or compile with the fallback backend
- Print status: `Found CRoaring:` or `CRoaring not found; building the compatibility adapter fallback`

## Running Benchmarks

```bash
# Run a quick benchmark (core dataset)
./run.sh core

# Generate plots
./plot.sh core

# Run all benchmarks (takes many hours on first run)
./run.sh all
./plot.sh all

# Available datasets: core, 1B, earth, MT, Para, Seg, WPMT, zipf
```

## Validating the Roaring Adapter

The demo was already tested during development. To re-run it:

```bash
cd build
./roaring_bitmap_demo
# Expected output: roaring_bitmap_demo ok cardinality=10 bytes=40
```

## Benchmark Output Location

Results are written to `graphs_X/` folders (where X is trial ID):

- Memory footprint: `graphs_X/M_*.txt`
- Query latency: `graphs_X/Q_*.txt`
- Update latency: `graphs_X/U_*.txt`
- Plots: `graphs_X/*.png`

## Next: Integrating RoaringVB into Table Implementations

Once the build is successful, you can integrate RoaringVB into the actual table implementations by:

1. **UpBit+ Integration** (simplest, start here):
   - Modify `src/ub/table.h` to use `cubit::RoaringVB` instead of `ibis::bitvector`
   - Update `append()`, `update()`, `remove()`, `evaluate()` methods

2. **UCB+ Integration** (similar):
   - Modify `src/ucb/table.h` and related code

3. **Benchmark Comparison**:
   - Run `./run.sh` with original WAH backend
   - Switch to RoaringVB, rebuild, and run again
   - Compare memory, latency, and throughput metrics

See `ROARING_INTEGRATION.md` for detailed integration patterns and architecture overview.

## Troubleshooting

### "cmake: command not found"

```bash
sudo apt-get install cmake
```

### "urcu.h: No such file or directory"

```bash
sudo apt-get install liburcu-dev
```

### "boost/array.hpp: No such file or directory"

```bash
sudo apt-get install libboost-all-dev
```

### "Could not find Boost"

```bash
# On some systems, you may need to set:
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
```

## Docker Alternative

If you prefer a containerized environment:

```dockerfile
FROM ubuntu:20.04
RUN apt-get update && apt-get install -y \
    build-essential cmake libboost-all-dev liburcu-dev python3 git
WORKDIR /root
COPY CUBIT-main /root/CUBIT-main
WORKDIR /root/CUBIT-main
RUN ./build.sh
```

Build and run:

```bash
docker build -t cubit-roaring .
docker run -it cubit-roaring bash
cd /root/CUBIT-main
./run.sh core
```
