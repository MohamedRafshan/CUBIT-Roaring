#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

def create_graphs():

    # ---------------- DATA ----------------
    cores = np.array([1, 2, 4, 8, 16])

    create_roaring = np.array([100, 180, 320, 580, 1000])
    create_wah = np.array([25, 45, 80, 150, 250])

    update_roaring = np.array([80, 150, 280, 520, 900])
    update_wah = np.array([15, 28, 50, 95, 170])

    delete_roaring = np.array([90, 170, 310, 590, 1020])
    delete_wah = np.array([20, 35, 65, 125, 220])

    # ---------------- FIGURE ----------------
    fig, axes = plt.subplots(
        2,
        3,
        figsize=(20, 12),
        constrained_layout=True
    )

    fig.suptitle(
        'CUBIT: WAH vs Roaring Performance Analysis',
        fontsize=18,
        fontweight='bold'
    )

    # ==========================================================
    # Helper function
    # ==========================================================

    def plot_throughput(ax, title, roaring, wah):

        ax.plot(
            cores,
            roaring,
            'o-',
            label='Roaring',
            linewidth=2.5,
            markersize=8,
            color='green'
        )

        ax.plot(
            cores,
            wah,
            's-',
            label='WAH',
            linewidth=2.5,
            markersize=8,
            color='red'
        )

        ax.fill_between(
            cores,
            roaring,
            wah,
            alpha=0.08,
            color='green'
        )

        ax.set_title(title, fontsize=12, fontweight='bold')
        ax.set_xlabel('Cores')
        ax.set_ylabel('Throughput (QPS)')

        ax.grid(True, linestyle='--', alpha=0.3)

        ax.set_xticks(cores)
        ax.set_xscale('log')

        ax.legend()

        # Add vertical padding
        ymax = max(roaring.max(), wah.max())
        ax.set_ylim(0, ymax * 1.25)

        # Speedup annotations
        for i, core in enumerate(cores):

            speedup = roaring[i] / wah[i]

            ax.annotate(
                f'{speedup:.1f}x',
                (core, roaring[i]),
                textcoords="offset points",
                xytext=(0, 10),
                ha='center',
                fontsize=8,
                fontweight='bold',
                color='darkgreen'
            )

    # ==========================================================
    # PLOT 1
    # ==========================================================

    plot_throughput(
        axes[0, 0],
        'CREATE Throughput',
        create_roaring,
        create_wah
    )

    # ==========================================================
    # PLOT 2
    # ==========================================================

    plot_throughput(
        axes[0, 1],
        'UPDATE Throughput',
        update_roaring,
        update_wah
    )

    # ==========================================================
    # PLOT 3
    # ==========================================================

    plot_throughput(
        axes[0, 2],
        'DELETE Throughput',
        delete_roaring,
        delete_wah
    )

    # ==========================================================
    # PLOT 4 - LATENCY
    # ==========================================================

    ax4 = axes[1, 0]

    operations = [
        'Create',
        'Upd-Del',
        'Upd-Add',
        'Delete'
    ]

    roaring_latency = np.array([15, 10, 15, 50])
    wah_latency = np.array([900, 900, 900, 700])

    x = np.arange(len(operations))
    width = 0.35

    bars1 = ax4.bar(
        x - width/2,
        roaring_latency,
        width,
        label='Roaring',
        color='green',
        alpha=0.8
    )

    bars2 = ax4.bar(
        x + width/2,
        wah_latency,
        width,
        label='WAH',
        color='red',
        alpha=0.8
    )

    ax4.set_title(
        'Operation Latency',
        fontsize=12,
        fontweight='bold'
    )

    ax4.set_ylabel('Latency (μs)')
    ax4.set_xticks(x)
    ax4.set_xticklabels(operations)

    ax4.set_yscale('log')

    ax4.grid(True, axis='y', linestyle='--', alpha=0.3)

    ax4.legend()

    # Better bar labels
    for bars in [bars1, bars2]:

        for bar in bars:

            h = bar.get_height()

            ax4.annotate(
                f'{int(h)}',
                (bar.get_x() + bar.get_width()/2, h),
                textcoords="offset points",
                xytext=(0, 5),
                ha='center',
                fontsize=8
            )

    # ==========================================================
    # PLOT 5 - EFFICIENCY
    # ==========================================================

    ax5 = axes[1, 1]

    roaring_eff = np.array([100, 90, 80, 73, 62])
    wah_eff = np.array([100, 90, 80, 63, 50])

    ax5.plot(
        cores,
        roaring_eff,
        'o-',
        label='Roaring',
        linewidth=2.5,
        markersize=8,
        color='green'
    )

    ax5.plot(
        cores,
        wah_eff,
        's-',
        label='WAH',
        linewidth=2.5,
        markersize=8,
        color='red'
    )

    ax5.axhline(
        y=100,
        linestyle=':',
        color='gray',
        linewidth=1.5
    )

    ax5.set_title(
        'Scalability Efficiency',
        fontsize=12,
        fontweight='bold'
    )

    ax5.set_xlabel('Cores')
    ax5.set_ylabel('Efficiency (%)')

    ax5.set_xticks(cores)
    ax5.set_xscale('log')

    ax5.grid(True, linestyle='--', alpha=0.3)

    ax5.set_ylim(40, 110)

    ax5.legend()

    # ==========================================================
    # PLOT 6 - SPEEDUP
    # ==========================================================

    ax6 = axes[1, 2]

    create_speedup = create_roaring / create_wah
    update_speedup = update_roaring / update_wah
    delete_speedup = delete_roaring / delete_wah

    ax6.plot(
        cores,
        create_speedup,
        'o-',
        label='Create',
        linewidth=2.5,
        markersize=8
    )

    ax6.plot(
        cores,
        update_speedup,
        's-',
        label='Update',
        linewidth=2.5,
        markersize=8
    )

    ax6.plot(
        cores,
        delete_speedup,
        '^-',
        label='Delete',
        linewidth=2.5,
        markersize=8
    )

    ax6.axhline(
        y=4,
        linestyle='--',
        color='gray',
        linewidth=1
    )

    ax6.set_title(
        'Roaring Speedup vs WAH',
        fontsize=12,
        fontweight='bold'
    )

    ax6.set_xlabel('Cores')
    ax6.set_ylabel('Speedup Factor')

    ax6.set_xticks(cores)
    ax6.set_xscale('log')

    ax6.grid(True, linestyle='--', alpha=0.3)

    ax6.legend()

    ax6.set_ylim(3.5, 6)

    # ==========================================================
    # SAVE
    # ==========================================================

    plt.savefig(
        'wah_vs_roaring_fixed.png',
        dpi=200,
        bbox_inches='tight'
    )

    print("Generated: wah_vs_roaring_fixed.png")

    plt.show()


if __name__ == '__main__':
    create_graphs()