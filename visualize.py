import os
import math
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

# ── helpers ──────────────────────────────────────────────────────────────────

def load_instance(path):
    nodes = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            x, y, p = line.split(";")
            nodes.append((int(x), int(y), int(p)))
    return nodes  # list of (x, y, profit)

def load_tour(path):
    with open(path) as f:
        return [int(l.strip()) for l in f if l.strip()]

def tour_distance(nodes, tour):
    def dist(a, b):
        return round(math.sqrt((nodes[a][0]-nodes[b][0])**2 + (nodes[a][1]-nodes[b][1])**2))
    return sum(dist(tour[i], tour[(i+1) % len(tour)]) for i in range(len(tour)))

def tour_profit(nodes, tour):
    return sum(nodes[v][2] for v in tour)

def tour_score(nodes, tour):
    return tour_profit(nodes, tour) - tour_distance(nodes, tour)

def load_stats_csv(path):
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f, delimiter=";")
        for row in reader:
            rows.append(row)
    return rows


# ── single tour plot ──────────────────────────────────────────────────────────

def plot_tour(ax, nodes, tour, title, stats_rows=None, algo_key=None, instance_key=None):
    xs = [n[0] for n in nodes]
    ys = [n[1] for n in nodes]
    in_set = set(tour)

    # background nodes
    out_x = [nodes[i][0] for i in range(len(nodes)) if i not in in_set]
    out_y = [nodes[i][1] for i in range(len(nodes)) if i not in in_set]
    ax.scatter(out_x, out_y, s=8, c="#cccccc", zorder=2, label="poza trasą")

    # tour nodes
    in_x = [nodes[v][0] for v in tour]
    in_y = [nodes[v][1] for v in tour]
    ax.scatter(in_x, in_y, s=18, c="#2196F3", zorder=4, label="w trasie")

    # edges
    for i in range(len(tour)):
        a, b = tour[i], tour[(i+1) % len(tour)]
        ax.plot([nodes[a][0], nodes[b][0]], [nodes[a][1], nodes[b][1]],
                c="#1565C0", lw=0.8, zorder=3, alpha=0.7)

    # start node highlight
    ax.scatter([nodes[tour[0]][0]], [nodes[tour[0]][1]],
               s=60, c="#F44336", zorder=5, label=f"start (#{tour[0]})")

    score  = tour_score(nodes, tour)
    profit = tour_profit(nodes, tour)
    dist   = tour_distance(nodes, tour)
    n_tour = len(tour)
    n_all  = len(nodes)

    info_lines = [
        f"Wierzchołki: {n_tour} / {n_all}",
        f"Wynik: {score:,}",
        f"Zysk: {profit:,}",
        f"Dystans: {dist:,}",
    ]

    if stats_rows and algo_key and instance_key:
        for row in stats_rows:
            if row.get("algorytm") == algo_key and row.get("instancja") == instance_key:
                info_lines += [
                    f"Śr. wynik: {float(row['sredni wynik']):.0f}",
                    f"Max wynik: {row['max wynik']}",
                    f"Min wynik: {row['min wynik']}",
                    f"Śr. czas:  {float(row['sredni czas'])/1000:.1f} ms",
                    f"Max czas:  {int(row['max czas'])//1000} ms",
                ]
                break

    textstr = "\n".join(info_lines)
    ax.text(0.02, 0.98, textstr, transform=ax.transAxes, fontsize=7,
            verticalalignment="top", fontfamily="monospace",
            bbox=dict(boxstyle="round,pad=0.4", facecolor="white", alpha=0.85))

    ax.set_title(title, fontsize=9, fontweight="bold")
    ax.set_aspect("equal")
    ax.legend(fontsize=6, loc="lower right")
    ax.set_xlabel("x"); ax.set_ylabel("y")


# ── comparison bar chart ──────────────────────────────────────────────────────

ALGO_LABELS = {
    "regret_gc":     "regretGC\n(konstr.)",
    "steep_walk":    "Steepest\n(ref.)",
    "lm_walk":       "LM walk",
    "candidate_walk":"Candidate\nwalk",
}
ALGO_ORDER = ["regret_gc", "steep_walk", "lm_walk", "candidate_walk"]
ALGO_COLORS = ["#78909C", "#1565C0", "#43A047", "#E53935"]

def plot_comparison(ax_score, ax_time, stats_rows, instance_key, title_suffix=""):
    present = {r["algorytm"]: r for r in stats_rows if r.get("instancja") == instance_key}
    rows = [present[a] for a in ALGO_ORDER if a in present]
    if not rows:
        return

    labels  = [ALGO_LABELS.get(r["algorytm"], r["algorytm"]) for r in rows]
    avgs    = [float(r["sredni wynik"]) for r in rows]
    maxs    = [int(r["max wynik"]) for r in rows]
    mins    = [int(r["min wynik"]) for r in rows]
    t_avgs  = [float(r["sredni czas"])/1000 for r in rows]
    t_maxs  = [int(r["max czas"])/1000 for r in rows]
    t_mins  = [int(r["min czas"])/1000 for r in rows]

    colors = ["#1565C0", "#43A047", "#E53935"]
    x = np.arange(len(labels))
    w = 0.25

    bars = []
    for i, (vals, lbl) in enumerate(zip([avgs, maxs, mins], ["śr.", "max", "min"])):
        b = ax_score.bar(x + i*w, vals, w, label=lbl, color=colors[i], alpha=0.85)
        bars.append(b)
    ax_score.set_xticks(x + w); ax_score.set_xticklabels(labels, fontsize=8)
    ax_score.set_ylabel("Wynik"); ax_score.set_title(f"Wynik — TSP{title_suffix}", fontsize=9, fontweight="bold")
    ax_score.legend(fontsize=7)
    # wartości nad słupkami (śr.)
    for rect, val in zip(bars[0], avgs):
        ax_score.text(rect.get_x() + rect.get_width()/2, rect.get_height() + 20,
                      f"{val:.0f}", ha="center", va="bottom", fontsize=6)

    bars_t = []
    for i, (vals, lbl) in enumerate(zip([t_avgs, t_maxs, t_mins], ["śr.", "max", "min"])):
        b = ax_time.bar(x + i*w, vals, w, label=lbl, color=colors[i], alpha=0.85)
        bars_t.append(b)
    ax_time.set_xticks(x + w); ax_time.set_xticklabels(labels, fontsize=8)
    ax_time.set_ylabel("Czas [ms]"); ax_time.set_title(f"Czas — TSP{title_suffix}", fontsize=9, fontweight="bold")
    ax_time.legend(fontsize=7)
    for rect, val in zip(bars_t[0], t_avgs):
        ax_time.text(rect.get_x() + rect.get_width()/2, rect.get_height() + 0.1,
                     f"{val:.1f}", ha="center", va="bottom", fontsize=6)


# ── main ──────────────────────────────────────────────────────────────────────

OUT_DIR = "wyniki_tsp"
os.makedirs(OUT_DIR, exist_ok=True)

nodesA = load_instance("TSPA.csv")
nodesB = load_instance("TSPB.csv")

# ── task 3 tours ──────────────────────────────────────────────────────────────

task3_tours = [
    ("regret_gc_a_best.txt",              nodesA, "regretGC — TSPA (najlepszy konstr. z zad. 1)",     "regret_gc",      "A"),
    ("regret_gc_b_best.txt",              nodesB, "regretGC — TSPB (najlepszy konstr. z zad. 1)",     "regret_gc",      "B"),
    ("steep_walk_a_random_edges.txt",     nodesA, "Steepest walk — TSPA (losowy start, swap_edges)",  "steep_walk",     "A"),
    ("steep_walk_b_random_edges.txt",     nodesB, "Steepest walk — TSPB (losowy start, swap_edges)",  "steep_walk",     "B"),
    ("lm_walk_a_random_edges.txt",        nodesA, "LM walk — TSPA (losowy start, swap_edges)",        "lm_walk",        "A"),
    ("lm_walk_b_random_edges.txt",        nodesB, "LM walk — TSPB (losowy start, swap_edges)",        "lm_walk",        "B"),
    ("candidate_walk_a_random_edges.txt", nodesA, "Candidate walk — TSPA (losowy start, swap_edges)", "candidate_walk", "A"),
    ("candidate_walk_b_random_edges.txt", nodesB, "Candidate walk — TSPB (losowy start, swap_edges)", "candidate_walk", "B"),
]

stats3 = []
if os.path.exists("wyniki3.csv"):
    stats3 = load_stats_csv("wyniki3.csv")

# individual tour plots (task 3)
for fname, nodes, title, algo_key, inst_key in task3_tours:
    fpath = os.path.join(OUT_DIR, fname)
    if not os.path.exists(fpath):
        print(f"  brak: {fpath}")
        continue
    tour = load_tour(fpath)
    fig, ax = plt.subplots(figsize=(9, 8))
    plot_tour(ax, nodes, tour, title, stats3, algo_key, inst_key)
    fig.tight_layout()
    out = os.path.join(OUT_DIR, fname.replace(".txt", ".png"))
    fig.savefig(out, dpi=140)
    plt.close(fig)
    print(f"Zapisano: {out}")

# ── task 3 comparison (all 3 algorithms side by side) ────────────────────────

if stats3:
    fig, axes = plt.subplots(2, 2, figsize=(14, 9))
    fig.suptitle("Porównanie algorytmów — Zadanie 3 (losowy start, swap_edges)", fontsize=11, fontweight="bold")
    plot_comparison(axes[0][0], axes[1][0], stats3, "A", "A")
    plot_comparison(axes[0][1], axes[1][1], stats3, "B", "B")
    fig.tight_layout()
    out = os.path.join(OUT_DIR, "task3_comparison.png")
    fig.savefig(out, dpi=140)
    plt.close(fig)
    print(f"Zapisano: {out}")

# ── task 2: best tours (all algorithms, both instances) ──────────────────────

task2_tours = [
    ("steep_walk_a_best_edges.txt",      nodesA, "Steepest — TSPA, regretGC, swap_edges"),
    ("steep_walk_b_best_edges.txt",      nodesB, "Steepest — TSPB, regretGC, swap_edges"),
    ("steep_walk_a_random_edges.txt",    nodesA, "Steepest — TSPA, losowy, swap_edges"),
    ("steep_walk_b_random_edges.txt",    nodesB, "Steepest — TSPB, losowy, swap_edges"),
    ("steep_walk_a_best_vertices.txt",   nodesA, "Steepest — TSPA, regretGC, swap_vertices"),
    ("steep_walk_b_best_vertices.txt",   nodesB, "Steepest — TSPB, regretGC, swap_vertices"),
    ("steep_walk_a_random_vertices.txt", nodesA, "Steepest — TSPA, losowy, swap_vertices"),
    ("steep_walk_b_random_vertices.txt", nodesB, "Steepest — TSPB, losowy, swap_vertices"),
    ("greedy_walk_a_best_edges.txt",     nodesA, "Greedy — TSPA, regretGC, swap_edges"),
    ("greedy_walk_b_best_edges.txt",     nodesB, "Greedy — TSPB, regretGC, swap_edges"),
    ("greedy_walk_a_random_edges.txt",   nodesA, "Greedy — TSPA, losowy, swap_edges"),
    ("greedy_walk_b_random_edges.txt",   nodesB, "Greedy — TSPB, losowy, swap_edges"),
    ("greedy_walk_a_best_vertices.txt",  nodesA, "Greedy — TSPA, regretGC, swap_vertices"),
    ("greedy_walk_b_best_vertices.txt",  nodesB, "Greedy — TSPB, regretGC, swap_vertices"),
    ("greedy_walk_a_random_vertices.txt",nodesA, "Greedy — TSPA, losowy, swap_vertices"),
    ("greedy_walk_b_random_vertices.txt",nodesB, "Greedy — TSPB, losowy, swap_vertices"),
    ("random_walk_a_best_edges.txt",     nodesA, "Random — TSPA, regretGC, swap_edges"),
    ("random_walk_b_best_edges.txt",     nodesB, "Random — TSPB, regretGC, swap_edges"),
    ("random_walk_a_random_edges.txt",   nodesA, "Random — TSPA, losowy, swap_edges"),
    ("random_walk_b_random_edges.txt",   nodesB, "Random — TSPB, losowy, swap_edges"),
]

for fname, nodes, title in task2_tours:
    fpath = os.path.join(OUT_DIR, fname)
    if not os.path.exists(fpath):
        continue
    tour = load_tour(fpath)
    fig, ax = plt.subplots(figsize=(9, 8))
    plot_tour(ax, nodes, tour, title)
    fig.tight_layout()
    out = os.path.join(OUT_DIR, fname.replace(".txt", ".png"))
    fig.savefig(out, dpi=140)
    plt.close(fig)
    print(f"Zapisano: {out}")

# ── task 3: side-by-side A vs B for each algorithm ───────────────────────────

task3_pairs = [
    ("lm_walk_a_random_edges.txt",        "lm_walk_b_random_edges.txt",
     "LM walk — losowy start, swap_edges",        "lm_walk",        "lm_walk_AB.png"),
    ("candidate_walk_a_random_edges.txt", "candidate_walk_b_random_edges.txt",
     "Candidate walk — losowy start, swap_edges", "candidate_walk", "candidate_walk_AB.png"),
    ("steep_walk_a_random_edges.txt",     "steep_walk_b_random_edges.txt",
     "Steepest walk — losowy start, swap_edges",  "steep_walk",     "steep_walk_AB.png"),
]

for fA, fB, title_base, algo_key, out_name in task3_pairs:
    pathA = os.path.join(OUT_DIR, fA)
    pathB = os.path.join(OUT_DIR, fB)
    if not os.path.exists(pathA) or not os.path.exists(pathB):
        continue
    tourA = load_tour(pathA)
    tourB = load_tour(pathB)
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(17, 8))
    fig.suptitle(title_base, fontsize=11, fontweight="bold")
    plot_tour(ax1, nodesA, tourA, "TSPA", stats3, algo_key, "A")
    plot_tour(ax2, nodesB, tourB, "TSPB", stats3, algo_key, "B")
    fig.tight_layout()
    out = os.path.join(OUT_DIR, out_name)
    fig.savefig(out, dpi=140)
    plt.close(fig)
    print(f"Zapisano: {out}")

# ── before/after: jeden start → 3 algorytmy ──────────────────────────────────

for inst, nodes, inst_label in [("a", nodesA, "TSPA"), ("b", nodesB, "TSPB")]:
    before_path = os.path.join(OUT_DIR, f"demo_before_{inst}.txt")
    if not os.path.exists(before_path):
        print(f"  brak demo_before_{inst}.txt — skipped (uruchom najpierw tsp.exe)")
        continue

    before   = load_tour(before_path)
    steep    = load_tour(os.path.join(OUT_DIR, f"demo_steep_after_{inst}.txt"))
    lm       = load_tour(os.path.join(OUT_DIR, f"demo_lm_after_{inst}.txt"))
    cand     = load_tour(os.path.join(OUT_DIR, f"demo_cand_after_{inst}.txt"))

    fig, axes = plt.subplots(1, 4, figsize=(28, 8))
    fig.suptitle(f"Przed i po lokalnym przeszukiwaniu — {inst_label} (ten sam losowy start)",
                 fontsize=12, fontweight="bold")

    plot_tour(axes[0], nodes, before, f"Start losowy\nn={len(before)}, wynik={tour_score(nodes,before):,}")
    plot_tour(axes[1], nodes, steep,  f"Steepest walk\nn={len(steep)}, wynik={tour_score(nodes,steep):,}")
    plot_tour(axes[2], nodes, lm,     f"LM walk\nn={len(lm)}, wynik={tour_score(nodes,lm):,}")
    plot_tour(axes[3], nodes, cand,   f"Candidate walk\nn={len(cand)}, wynik={tour_score(nodes,cand):,}")

    for ax in axes:
        ax.legend(fontsize=5, loc="lower right")

    fig.tight_layout()
    out = os.path.join(OUT_DIR, f"before_after_{inst}.png")
    fig.savefig(out, dpi=130)
    plt.close(fig)
    print(f"Zapisano: {out}")

# ── before/after: zoom na różnicę (2 panele: przed vs najlepszy po) ───────────

for inst, nodes, inst_label in [("a", nodesA, "TSPA"), ("b", nodesB, "TSPB")]:
    before_path = os.path.join(OUT_DIR, f"demo_before_{inst}.txt")
    lm_path     = os.path.join(OUT_DIR, f"demo_lm_after_{inst}.txt")
    if not os.path.exists(before_path) or not os.path.exists(lm_path):
        continue

    before = load_tour(before_path)
    lm     = load_tour(lm_path)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(17, 8))
    fig.suptitle(f"Przed / Po LM walk — {inst_label}", fontsize=12, fontweight="bold")
    plot_tour(ax1, nodes, before,
              f"Start losowy  |  n={len(before)}/{len(nodes)}  |  wynik={tour_score(nodes,before):,}\n"
              f"zysk={tour_profit(nodes,before):,}  dystans={tour_distance(nodes,before):,}")
    plot_tour(ax2, nodes, lm,
              f"Po LM walk  |  n={len(lm)}/{len(nodes)}  |  wynik={tour_score(nodes,lm):,}\n"
              f"zysk={tour_profit(nodes,lm):,}  dystans={tour_distance(nodes,lm):,}")
    fig.tight_layout()
    out = os.path.join(OUT_DIR, f"before_after_lm_{inst}.png")
    fig.savefig(out, dpi=140)
    plt.close(fig)
    print(f"Zapisano: {out}")

print("\nGotowe.")
