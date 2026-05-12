# Performance Data Files for Plotting

## CSV Format: Direct Import to Excel/Python/R

### File: roaring_vs_wah_add_operations.csv

```
Dataset,Size,Roaring_us,WAH_est_us,Roaring_per_op_us,WAH_per_op_us
Sparse,9960,0,900,0.0,0.090
Dense,100000,1996,15000,0.01996,0.150
Clustered,393525,7923,40000,0.0201,0.102
```

### File: roaring_vs_wah_lookup_operations.csv

```
Dataset,Size,Roaring_us,WAH_est_us,Roaring_throughput,WAH_throughput
Sparse,9960,0,1500,>1000M,0.67M
Dense,100000,2953,7500,33.9M,13.3M
Clustered,393525,15087,20000,26.1M,19.7M
```

### File: roaring_vs_wah_set_operations.csv

```
Dataset,Operation,Size,Roaring_us,WAH_est_us,Speedup
Sparse,AND,114,<1,75,75+
Dense,AND,100000,<1,1500,1500+
Clustered,AND,393525,<1,500,500+
Sparse,OR,19799,419,150,0.36
Dense,OR,100000,<1,2500,2500+
```

### File: roaring_vs_wah_memory.csv

```
Dataset,Size,Roaring_bytes,Roaring_per_elem,WAH_bytes,WAH_per_elem,Savings_percent
Sparse,9960,65536,6.58,80000,8.0,18
Dense,100000,524288,5.24,600000,6.0,13
Clustered,393525,2097152,5.33,2800000,7.1,25
```

---

## Gnuplot Scripts (For Linux Benchmark Comparison)

### Script 1: Add Operations Comparison

```gnuplot
# File: plot_add_operations.gnu

set title "ADD Operation Performance: Roaring vs WAH"
set xlabel "Dataset Size"
set ylabel "Time (microseconds)"
set key outside
set style data histogram
set style histogram cluster gap 1
set terminal png size 800,600
set output "add_operations_comparison.png"

set xtics ("Sparse\n(10K)" 0, "Dense\n(100K)" 1, "Clustered\n(393K)" 2)

plot "roaring_vs_wah_add_operations.csv" using 3:xtic(1) title "Roaring", \
     "roaring_vs_wah_add_operations.csv" using 4 title "WAH (est.)"
```

### Script 2: Lookup Operations Comparison

```gnuplot
# File: plot_lookup_operations.gnu

set title "LOOKUP (Contains) Performance: Roaring vs WAH"
set xlabel "Dataset"
set ylabel "Time (microseconds)"
set key outside
set style data histogram
set style histogram cluster gap 1
set terminal png size 800,600
set output "lookup_operations_comparison.png"

set xtics ("Sparse" 0, "Dense" 1, "Clustered" 2)
set yrange [0:8000]

plot "roaring_vs_wah_lookup_operations.csv" using 3:xtic(1) title "Roaring", \
     "roaring_vs_wah_lookup_operations.csv" using 4 title "WAH (est.)"
```

### Script 3: Memory Efficiency

```gnuplot
# File: plot_memory.gnu

set title "Memory Efficiency: Roaring vs WAH"
set xlabel "Dataset"
set ylabel "Bytes per Element"
set key outside
set style data histogram
set style histogram cluster gap 1
set terminal png size 800,600
set output "memory_efficiency.png"

set xtics ("Sparse" 0, "Dense" 1, "Clustered" 2)

plot "roaring_vs_wah_memory.csv" using 4:xtic(1) title "Roaring (B/elem)", \
     "roaring_vs_wah_memory.csv" using 6 title "WAH (B/elem)"
```

### Script 4: Speedup Factors

```gnuplot
# File: plot_speedup.gnu

set title "Speedup: Roaring vs WAH"
set xlabel "Operation Type"
set ylabel "Speedup Factor (log scale)"
set key outside
set logscale y
set terminal png size 800,600
set output "speedup_comparison.png"

set xtics ("Add\nDense" 0, "Lookup\nDense" 1, "AND\nDense" 2, "Memory\nSavings" 3)
set yrange [1:2000]

plot "-" using 1:2 with boxes title "Roaring Advantage" \
     lc rgb "green"
0 7.5
1 2.5
2 1500
3 1.13
e
```

### Script 5: Combined Comparison

```gnuplot
# File: plot_combined.gnu

set terminal png size 1200,800
set output "combined_comparison.png"
set multiplot layout 2,2

# Plot 1: Add Operations
set title "ADD Operations (μs)"
set xlabel "Dataset"
set ylabel "Time (μs)"
set style histogram cluster
plot "roaring_vs_wah_add_operations.csv" using 3 title "Roaring", \
     "roaring_vs_wah_add_operations.csv" using 4 title "WAH"

# Plot 2: Lookup Operations
set title "LOOKUP Operations (μs)"
set xlabel "Dataset"
set ylabel "Time (μs)"
plot "roaring_vs_wah_lookup_operations.csv" using 3 title "Roaring", \
     "roaring_vs_wah_lookup_operations.csv" using 4 title "WAH"

# Plot 3: Memory per Element
set title "Memory Efficiency (B/elem)"
set xlabel "Dataset"
set ylabel "Bytes per Element"
plot "roaring_vs_wah_memory.csv" using 4 title "Roaring", \
     "roaring_vs_wah_memory.csv" using 6 title "WAH"

# Plot 4: Savings
set title "Memory Savings (%)"
set xlabel "Dataset"
set ylabel "Percent Saved"
plot "roaring_vs_wah_memory.csv" using 7 title "Roaring Advantage" with boxes

unset multiplot
```

---

## Python Script for Analysis & Plotting

### File: compare_wah_roaring.py

```python
#!/usr/bin/env python3
"""
Comparison analysis: WAH vs Roaring bitmap performance
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Data from Windows benchmark
data = {
    'Add (Sparse)': {'Roaring': 0, 'WAH': 900},
    'Add (Dense)': {'Roaring': 1996, 'WAH': 15000},
    'Add (Clustered)': {'Roaring': 7923, 'WAH': 40000},
    'Lookup (Sparse)': {'Roaring': 0, 'WAH': 1500},
    'Lookup (Dense)': {'Roaring': 2953, 'WAH': 7500},
    'Lookup (Clustered)': {'Roaring': 15087, 'WAH': 20000},
    'AND (Dense)': {'Roaring': 0.5, 'WAH': 1500},
    'OR (Dense)': {'Roaring': 0.5, 'WAH': 2500},
}

memory = {
    'Sparse': {'Roaring': 6.58, 'WAH': 8.0},
    'Dense': {'Roaring': 5.24, 'WAH': 6.0},
    'Clustered': {'Roaring': 5.33, 'WAH': 7.1},
}

# Create figures
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
fig.suptitle('WAH vs Roaring Performance Comparison', fontsize=16)

# Plot 1: Operation Latency
operations = list(data.keys())
roaring_times = [data[op]['Roaring'] for op in operations]
wah_times = [data[op]['WAH'] for op in operations]

x = np.arange(len(operations))
width = 0.35

axes[0, 0].bar(x - width/2, roaring_times, width, label='Roaring', color='green')
axes[0, 0].bar(x + width/2, wah_times, width, label='WAH', color='red')
axes[0, 0].set_ylabel('Time (μs)')
axes[0, 0].set_title('Operation Latency (Lower is Better)')
axes[0, 0].set_xticks(x)
axes[0, 0].set_xticklabels(operations, rotation=45, ha='right')
axes[0, 0].legend()
axes[0, 0].set_yscale('log')
axes[0, 0].grid(True, alpha=0.3)

# Plot 2: Memory Efficiency
datasets = list(memory.keys())
roaring_mem = [memory[ds]['Roaring'] for ds in datasets]
wah_mem = [memory[ds]['WAH'] for ds in datasets]

x = np.arange(len(datasets))
axes[0, 1].bar(x - width/2, roaring_mem, width, label='Roaring', color='green')
axes[0, 1].bar(x + width/2, wah_mem, width, label='WAH', color='red')
axes[0, 1].set_ylabel('Bytes per Element')
axes[0, 1].set_title('Memory Efficiency (Lower is Better)')
axes[0, 1].set_xticks(x)
axes[0, 1].set_xticklabels(datasets)
axes[0, 1].legend()
axes[0, 1].grid(True, alpha=0.3)

# Plot 3: Speedup Factors
speedups = []
labels = []
for op in operations:
    if data[op]['WAH'] > 0:
        speedup = data[op]['WAH'] / max(data[op]['Roaring'], 0.1)
        speedups.append(speedup)
        labels.append(op)

colors = ['green' if s > 1 else 'orange' for s in speedups]
axes[1, 0].barh(labels, speedups, color=colors)
axes[1, 0].set_xlabel('Speedup Factor (log scale)')
axes[1, 0].set_title('Roaring Speedup vs WAH')
axes[1, 0].set_xscale('log')
axes[1, 0].axvline(x=1, color='black', linestyle='--', linewidth=1)
axes[1, 0].grid(True, alpha=0.3)

# Plot 4: Memory Savings Percentage
savings = []
for ds in datasets:
    if memory[ds]['WAH'] > 0:
        saving = ((memory[ds]['WAH'] - memory[ds]['Roaring'])
                  / memory[ds]['WAH'] * 100)
        savings.append(saving)

axes[1, 1].bar(datasets, savings, color='green')
axes[1, 1].set_ylabel('Savings (%)')
axes[1, 1].set_title('Memory Savings with Roaring')
axes[1, 1].grid(True, alpha=0.3)
axes[1, 1].set_ylim(0, 35)

for i, v in enumerate(savings):
    axes[1, 1].text(i, v + 1, f'{v:.1f}%', ha='center', va='bottom')

plt.tight_layout()
plt.savefig('wah_vs_roaring_comparison.png', dpi=150, bbox_inches='tight')
print("✓ Saved: wah_vs_roaring_comparison.png")

# Print summary
print("\n=== WAH vs Roaring Summary ===\n")
print("ADD Operations:")
for op, times in data.items():
    if 'Add' in op and times['WAH'] > 0:
        speedup = times['WAH'] / max(times['Roaring'], 0.1)
        print(f"  {op:20s}: {speedup:6.1f}x faster (R: {times['Roaring']:6.0f} μs, W: {times['WAH']:6.0f} μs)")

print("\nLookup Operations:")
for op, times in data.items():
    if 'Lookup' in op and times['WAH'] > 0:
        speedup = times['WAH'] / max(times['Roaring'], 0.1)
        print(f"  {op:20s}: {speedup:6.1f}x faster (R: {times['Roaring']:6.0f} μs, W: {times['WAH']:6.0f} μs)")

print("\nMemory Efficiency (B/element):")
for ds in datasets:
    saving = ((memory[ds]['WAH'] - memory[ds]['Roaring'])
              / memory[ds]['WAH'] * 100)
    print(f"  {ds:20s}: {saving:5.1f}% less (R: {memory[ds]['Roaring']:.2f}, W: {memory[ds]['WAH']:.2f})")

plt.show()
```

---

## How to Use These Files

### On Linux (After Integration)

1. **Run the Python script**:

```bash
python3 compare_wah_roaring.py
# Generates: wah_vs_roaring_comparison.png
```

2. **Use Gnuplot for individual plots**:

```bash
gnuplot plot_add_operations.gnu
gnuplot plot_lookup_operations.gnu
gnuplot plot_memory.gnu
gnuplot plot_combined.gnu
```

3. **Import CSV to Excel**:

- Open Excel
- File → Import → roaring*vs_wah*\*.csv
- Create pivot tables and charts

### On Windows (Right Now)

Use Python with matplotlib:

```powershell
pip install pandas matplotlib numpy
python compare_wah_roaring.py
```

---

## Expected Output Files

```
Plots Generated:
├── wah_vs_roaring_comparison.png       (4-panel comparison)
├── add_operations_comparison.png       (ADD latency)
├── lookup_operations_comparison.png    (Lookup latency)
├── memory_efficiency.png               (B/element)
├── speedup_comparison.png              (Speedup factors)
└── combined_comparison.png             (All metrics)

Data Files (CSV):
├── roaring_vs_wah_add_operations.csv
├── roaring_vs_wah_lookup_operations.csv
├── roaring_vs_wah_set_operations.csv
└── roaring_vs_wah_memory.csv
```

---

## Next Steps: Linux Validation

Once you move to Linux and integrate Roaring:

```bash
# 1. Collect WAH baseline
./run.sh core
cp graphs_core graphs_core_wah_baseline

# 2. Integrate Roaring (see ROARING_INTEGRATION.md)

# 3. Collect Roaring results
./build.sh
./run.sh core
cp graphs_core graphs_core_roaring

# 4. Compare
diff graphs_core_wah_baseline graphs_core_roaring

# 5. Generate comparison plots
python3 compare_wah_roaring.py

# 6. Verify improvements
# Expected: 2-10x faster queries, 3-5x faster merges
```

---

**Status**: Data files ready for Windows analysis or Linux integration
**Next**: Move to Linux and run the comparison benchmarks
