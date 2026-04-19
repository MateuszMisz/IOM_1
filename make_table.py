import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np

OUT = "wyniki_tsp/tabela_wynikow.png"

# ── dane ─────────────────────────────────────────────────────────────────────
# (algorytm, instancja, start, ruch, śr_czas_ms, max_czas_ms, min_czas_ms,
#  śr_wynik, max_wynik, min_wynik)

rows = [
    # ---------- Zadanie 1 – heurystyki konstrukcyjne (bez czasu walk) ----------
    ("random",        "A", "—",         "—",            "—",    "—",    "—",   -2394,   380,  -11040),
    ("random",        "B", "—",         "—",            "—",    "—",    "—",   -3399,   864,  -12010),
    ("regretGC",      "A", "determin.", "—",             3.6,    5.8,    3.3,   6192,  6985,    4810),
    ("regretGC",      "B", "determin.", "—",             3.7,    6.8,    3.3,  18056, 19486,   17086),
    # ---------- Zadanie 2 – lokalne przeszukiwanie (wybrane warianty) ----------
    ("greedy_walk",   "A", "regretGC",  "swap_edges",    1.4,    5.6,    0.4,   6334,  7265,    4996),
    ("greedy_walk",   "B", "regretGC",  "swap_edges",    1.6,    4.5,    0.4,  18391, 19548,   17201),
    ("greedy_walk",   "A", "losowy",    "swap_edges",   78.4,  148.9,    1.5,   5594,  7230,    1255),
    ("greedy_walk",   "B", "losowy",    "swap_edges",   82.8,  172.1,    5.2,  16996, 18818,    8121),
    ("steep_walk",    "A", "regretGC",  "swap_edges",    1.2,    3.6,    0.3,   6339,  7488,    4996),
    ("steep_walk",    "B", "regretGC",  "swap_edges",    1.5,    3.8,    0.3,  18454, 19548,   17247),
    ("steep_walk",    "A", "losowy",    "swap_edges",   24.7,   49.6,    0.1,   5503,  7561,     272),
    ("steep_walk",    "B", "losowy",    "swap_edges",   28.6,   52.8,    0.4,  16397, 19319,    2625),
    ("steep_walk",    "A", "regretGC",  "swap_vertices", 1.1,    3.9,    0.2,   6303,  7214,    4996),
    ("steep_walk",    "B", "regretGC",  "swap_vertices", 1.1,    3.4,    0.2,  18217, 19548,   17177),
    ("steep_walk",    "A", "losowy",    "swap_vertices",39.1,   81.7,    0.1,  -7117,  5146,  -26238),
    ("steep_walk",    "B", "losowy",    "swap_vertices",43.9,   87.3,    0.2,   1942, 16400,  -18714),
    # ---------- Zadanie 3 – LM i kandydaci (losowy start, swap_edges) ----------
    ("steep_walk",    "A", "losowy",    "swap_edges",   24.7,   49.6,    0.1,   5503,  7561,     272),
    ("steep_walk",    "B", "losowy",    "swap_edges",   28.6,   52.8,    0.4,  16397, 19319,    2625),
    ("lm_walk",       "A", "losowy",    "swap_edges",    8.6,   23.0,    0.1,   5519,  7561,     272),
    ("lm_walk",       "B", "losowy",    "swap_edges",    9.4,   39.4,    0.2,  16373, 19319,    2625),
    ("candidate_walk","A", "losowy",    "swap_edges",   22.3,   42.2,    0.6,   5693,  7490,     331),
    ("candidate_walk","B", "losowy",    "swap_edges",    23.7,  47.4,    1.4,  17034, 19141,    2768),
]

col_headers = [
    "Algorytm", "Inst.", "Start", "Ruch",
    "Śr. czas\n[ms]", "Max czas\n[ms]", "Min czas\n[ms]",
    "Śr. wynik", "Max wynik", "Min wynik",
]

# sekcje – (etykieta, indeksy wierszy)
sections = [
    ("Zadanie 1 – Heurystyki konstrukcyjne", list(range(0, 4))),
    ("Zadanie 2 – Lokalne przeszukiwanie (wybrane warianty)", list(range(4, 16))),
    ("Zadanie 3 – LM walk i Candidate walk (losowy start, swap_edges)", list(range(16, 22))),
]

# kolory bazowe
HEADER_BG  = "#1565C0"
HEADER_FG  = "white"
SEC_BG     = "#E3F2FD"
SEC_FG     = "#0D47A1"
ROW_ODD    = "#FAFAFA"
ROW_EVEN   = "#F0F4FF"
BORDER     = "#BDBDBD"

# ── heatmap: kolumny 4-6 = czas (niski = dobry → zielony), 7-9 = wynik (wysoki = dobry → zielony)
# Indeksy kolumn danych w krotce rows: 4=śr_czas, 5=max_czas, 6=min_czas, 7=śr_wynik, 8=max_wynik, 9=min_wynik
TIME_COLS  = [4, 5, 6]   # mniejszy = lepszy
SCORE_COLS = [7, 8, 9]   # większy = lepszy

cmap_good = matplotlib.colormaps["RdYlGn"]   # czerwony→żółty→zielony

def heatmap_color(value, vmin, vmax, higher_is_better=True):
    """Zwraca kolor RGB (hex) na podstawie pozycji value w [vmin, vmax]."""
    if vmin == vmax:
        return None
    t = (value - vmin) / (vmax - vmin)   # 0=min, 1=max
    if not higher_is_better:
        t = 1.0 - t                        # odwróć: min jest najlepszy
    rgba = cmap_good(t)
    # rozjaśnij kolor (mix z białym) żeby tekst pozostał czytelny
    r, g, b, _ = rgba
    r = 0.35 * r + 0.65
    g = 0.35 * g + 0.65
    b = 0.35 * b + 0.65
    return mcolors.to_hex((r, g, b))

def text_color_for_bg(hex_bg):
    """Czarny lub biały tekst w zależności od jasności tła."""
    r, g, b = mcolors.to_rgb(hex_bg)
    lum = 0.2126*r + 0.7152*g + 0.0722*b
    return "#000000" if lum > 0.45 else "#ffffff"

# Wylicz min/max dla każdej kolumny numerycznej osobno dla każdej instancji
def numeric_vals(col_idx, inst):
    vals = []
    for r in rows:
        if r[1] != inst:
            continue
        v = r[col_idx]
        if isinstance(v, (int, float)):
            vals.append(v)
    return vals

col_range = {}   # (col_idx, inst) -> (vmin, vmax)
for ci in TIME_COLS + SCORE_COLS:
    for inst in ("A", "B"):
        vs = numeric_vals(ci, inst)
        if vs:
            col_range[(ci, inst)] = (min(vs), max(vs))

col_widths = [0.14, 0.05, 0.09, 0.11, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09]

fig_w = 16
n_data_rows = len(rows)
n_sec_rows  = len(sections)
row_h = 0.38
fig_h = (n_data_rows + n_sec_rows + 1) * row_h + 1.2

fig, ax = plt.subplots(figsize=(fig_w, fig_h))
ax.set_xlim(0, 1)
ax.set_ylim(0, 1)
ax.axis("off")
fig.patch.set_facecolor("white")

def col_x(c):
    return sum(col_widths[:c])

def fmt(val):
    if val == "—":
        return "—"
    if isinstance(val, float):
        return f"{val:.1f}"
    return f"{val:,}"

total_rows = n_data_rows + n_sec_rows + 1  # +1 header
row_h_norm = 1.0 / total_rows

def draw_row(row_idx, cells, bg, fg="#000000", bold=False, fontsize=7.5, cell_colors=None):
    """cell_colors: lista hex lub None dla każdej komórki; None = użyj bg."""
    y_top = 1.0 - row_idx * row_h_norm
    y_bot = y_top - row_h_norm
    y_mid = (y_top + y_bot) / 2
    for c, (cell, w) in enumerate(zip(cells, col_widths)):
        x0 = col_x(c)
        cell_bg = (cell_colors[c] if cell_colors and cell_colors[c] else bg)
        cell_fg = (text_color_for_bg(cell_bg) if cell_colors and cell_colors[c] else fg)
        rect = plt.Rectangle((x0, y_bot), w, row_h_norm,
                               facecolor=cell_bg, edgecolor=BORDER, linewidth=0.4)
        ax.add_patch(rect)
        ax.text(x0 + w/2, y_mid, str(cell),
                ha="center", va="center", fontsize=fontsize,
                color=cell_fg, fontweight="bold" if bold else "normal",
                fontfamily="monospace" if c >= 4 else "sans-serif")

def draw_section_header(row_idx, label):
    y_top = 1.0 - row_idx * row_h_norm
    y_bot = y_top - row_h_norm
    y_mid = (y_top + y_bot) / 2
    rect = plt.Rectangle((0, y_bot), 1.0, row_h_norm,
                           facecolor=SEC_BG, edgecolor=BORDER, linewidth=0.6)
    ax.add_patch(rect)
    ax.text(0.01, y_mid, label, ha="left", va="center",
            fontsize=8.5, color=SEC_FG, fontweight="bold")

# ── nagłówek ─────────────────────────────────────────────────────────────────
draw_row(0, col_headers, HEADER_BG, HEADER_FG, bold=True, fontsize=7.5)

cur_row = 1
for sec_label, sec_indices in sections:
    draw_section_header(cur_row, sec_label)
    cur_row += 1
    for local_i, data_i in enumerate(sec_indices):
        r = rows[data_i]
        bg = ROW_ODD if local_i % 2 == 0 else ROW_EVEN
        cells = [r[0], r[1], r[2], r[3],
                 fmt(r[4]), fmt(r[5]), fmt(r[6]),
                 fmt(r[7]), fmt(r[8]), fmt(r[9])]

        # buduj kolory komórek (indeks w cells = indeks w rows + 0..3 tekstowe)
        inst = r[1]
        cell_colors = [None, None, None, None]  # pierwsze 4 tekstowe
        for ci in TIME_COLS:
            v = r[ci]
            key = (ci, inst)
            if isinstance(v, (int, float)) and key in col_range:
                vmin, vmax = col_range[key]
                cell_colors.append(heatmap_color(v, vmin, vmax, higher_is_better=False))
            else:
                cell_colors.append(None)
        for ci in SCORE_COLS:
            v = r[ci]
            key = (ci, inst)
            if isinstance(v, (int, float)) and key in col_range:
                vmin, vmax = col_range[key]
                cell_colors.append(heatmap_color(v, vmin, vmax, higher_is_better=True))
            else:
                cell_colors.append(None)

        draw_row(cur_row, cells, bg, cell_colors=cell_colors)
        cur_row += 1

# ── tytuł ────────────────────────────────────────────────────────────────────
fig.text(0.5, 0.995, "Wyniki eksperymentów — IOM Zadania 1–3 (100 uruchomień, TSP A i B)",
         ha="center", va="top", fontsize=11, fontweight="bold")
fig.text(0.5, 0.972,
         "Czas w ms (µs/1000). Wynik = zysk − dystans. Lokalne przeszukiwanie: swap_edges lub swap_vertices, start deterministyczny (regretGC) lub losowy.",
         ha="center", va="top", fontsize=7.5, color="#555555")

fig.tight_layout(rect=[0, 0, 1, 0.97])
fig.savefig(OUT, dpi=160, bbox_inches="tight")
plt.close(fig)
print(f"Zapisano: {OUT}")
