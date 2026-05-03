#!/usr/bin/env python3
import argparse
import os
import glob
from typing import List, Optional
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def read_coords(path: str):
    df = pd.read_csv(path, sep=';', header=None, engine='python')
    x = df.iloc[:, 0].astype(float).values
    y = df.iloc[:, 1].astype(float).values
    return np.column_stack((x, y))

def read_tour(path: str) -> List[int]:
    with open(path, 'r') as f:
        data = f.read().split()
    tour = [int(x) for x in data if x.strip() != '']
    return tour

def plot_tour(ax, coords: np.ndarray, tour: List[int], title: str):
    if len(tour) == 0:
        ax.set_title(title + " (empty)")
        return
    
    pts = coords[tour]
    loop = np.vstack((pts, pts[0]))
    
    ax.scatter(coords[:, 0], coords[:, 1], color='lightgray', s=8, alpha=0.6)
    ax.plot(loop[:, 0], loop[:, 1], '-o', color='C0', markersize=3)
    ax.scatter(loop[0, 0], loop[0, 1], color='green', s=80, marker='^', label='start')
    ax.scatter(loop[-2, 0], loop[-2, 1], color='red', s=80, marker='v', label='end')
    
    ax.set_title(title)
    ax.grid(True, linestyle='--', alpha=0.4)
    # Wymuszenie równej skali osi, aby wykres był "kwadratowy" i nieprzekłamany
    ax.set_aspect('equal', adjustable='box')

def ensure_exists(path: str) -> bool:
    if not os.path.exists(path):
        print(f"[WARN] file not found: {path}")
        return False
    return True

def plot_mlsl(coords_a, coords_b, mlsa='wyniki_tsp/MLSL_a.txt', mlsb='wyniki_tsp/MLSL_b.txt'):
    # Zmieniono figsize na kwadratowe proporcje dla całości (12x6 -> 12x12 by zapewnić miejsce)
    fig, axes = plt.subplots(1, 2, figsize=(12, 6))
    
    if ensure_exists(mlsa):
        tour_a = read_tour(mlsa)
        plot_tour(axes[0], coords_a, tour_a, 'MLSL A - final')
    else:
        axes[0].set_title('MLSL A - missing')
        
    if ensure_exists(mlsb):
        tour_b = read_tour(mlsb)
        plot_tour(axes[1], coords_b, tour_b, 'MLSL B - final')
    else:
        axes[1].set_title('MLSL B - missing')
        
    plt.tight_layout()
    out = 'mlsl_final.png'
    plt.savefig(out, dpi=200)
    print(f"Saved {out}")

def plot_three_panel(coords, start_path: Optional[str], phase_path: Optional[str], final_path: Optional[str], outname: str, instance_label: str):
    fig, axes = plt.subplots(2, 2, figsize=(10, 10))
    axes_list = [axes[0, 0], axes[0, 1], axes[1, 0]]
    titles = ['Starting', 'After initial local search', 'Final']
    paths = [start_path, phase_path, final_path]
    
    for ax, title, path in zip(axes_list, titles, paths):
        if path and ensure_exists(path):
            tour = read_tour(path)
            plot_tour(ax, coords, tour, title)
        else:
            ax.set_title(f"{title} (missing)")
    
    # Ukrycie czwartego subplotu (prawy dolny róg)
    axes[1, 1].axis('off')
    # USUNIĘTO: axes[1, 1].text(...) - brak napisów w prawym dolnym rogu
    
    plt.tight_layout()
    plt.savefig(outname, dpi=200)
    print(f"Saved {outname}")

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--coords-a', default='TSPA.csv')
    p.add_argument('--coords-b', default='TSPB.csv')
    p.add_argument('--mlsl-a', default=None)
    p.add_argument('--mlsl-b', default=None)
    p.add_argument('--solutions-dir', default='solutions_TSP')
    p.add_argument('--tours-dir', default='zadanie4_tours')
    p.add_argument('--out-dir', default='plots')
    args = p.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    coords_a = read_coords(args.coords_a)
    coords_b = read_coords(args.coords_b)

    mlsa = args.mlsl_a if args.mlsl_a else os.path.join(args.tours_dir, 'MLSL_a.txt')
    mlsb = args.mlsl_b if args.mlsl_b else os.path.join(args.tours_dir, 'MLSL_b.txt')
    
    plot_mlsl(coords_a, coords_b, mlsa=mlsa, mlsb=mlsb)
    # Przeniesienie pliku do folderu wyjściowego
    if os.path.exists('mlsl_final.png'):
        os.rename('mlsl_final.png', os.path.join(args.out_dir, 'mlsl_final.png'))

    tours = args.tours_dir
    variants = []
    
    # Definicje wariantów ILS / LNS
    variants.append((os.path.join(tours, 'ILS_a_starting_tour.txt'),
                     os.path.join(tours, 'ILS_a_after_initial_local_search.txt'),
                     os.path.join(tours, 'ILS_a_final_tour.txt'),
                     os.path.join(args.out_dir, 'ILS_A_three_panel.png'), coords_a, 'ILS A'))
    
    variants.append((os.path.join(tours, 'ILS_b_starting_tour.txt'),
                     os.path.join(tours, 'ILS_b_after_initial_local_search.txt'),
                     os.path.join(tours, 'ILS_b_final_tour.txt'),
                     os.path.join(args.out_dir, 'ILS_B_three_panel.png'), coords_b, 'ILS B'))

    variants.append((os.path.join(tours, 'LNS_subpaths_a_starting_tour.txt'),
                     os.path.join(tours, 'LNS_subpaths_a_after_initial_local_search.txt'),
                     os.path.join(tours, 'LNS_subpaths_a_final_tour.txt'),
                     os.path.join(args.out_dir, 'LNS_subpaths_local_A_three_panel.png'), coords_a, 'LNS subpaths local A'))
    
    variants.append((os.path.join(tours, 'LNS_subpaths_b_starting_tour.txt'),
                     os.path.join(tours, 'LNS_subpaths_b_after_initial_local_search.txt'),
                     os.path.join(tours, 'LNS_subpaths_b_final_tour.txt'),
                     os.path.join(args.out_dir, 'LNS_subpaths_local_B_three_panel.png'), coords_b, 'LNS subpaths local B'))
    
    variants.append((os.path.join(tours, 'LNS_subpaths_a_no_local_starting_tour.txt'),
                     os.path.join(tours, 'LNS_subpaths_a_no_local_after_initial_local_search.txt'),
                     os.path.join(tours, 'LNS_subpaths_a_no_local_final_tour.txt'),
                     os.path.join(args.out_dir, 'LNS_subpaths_no_local_A_three_panel.png'), coords_a, 'LNS subpaths no_local A'))
    
    variants.append((os.path.join(tours, 'LNS_subpaths_b_no_local_starting_tour.txt'),
                     os.path.join(tours, 'LNS_subpaths_b_no_local_after_initial_local_search.txt'),
                     os.path.join(tours, 'LNS_subpaths_b_no_local_final_tour.txt'),
                     os.path.join(args.out_dir, 'LNS_subpaths_no_local_B_three_panel.png'), coords_b, 'LNS subpaths no_local B'))

    for start_p, phase_p, final_p, outname, coords, label in variants:
        plot_three_panel(coords, start_p, phase_p, final_p, outname, label)

if __name__ == '__main__':
    main()