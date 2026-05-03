#!/usr/bin/env python3
"""
check_scores.py

Loads best tours from zadanie4_tours/ and compares their scores with solution_checker2.xlsx
"""
import pandas as pd
import numpy as np
import os

# Read coordinates
def read_coords(path: str):
    df = pd.read_csv(path, sep=';', header=None, engine='python')
    x = df.iloc[:, 0].astype(float).values
    y = df.iloc[:, 1].astype(float).values
    profit = df.iloc[:, 2].astype(float).values if df.shape[1] > 2 else np.zeros(len(x))
    return x, y, profit

# Calculate distance
def dist(x1, y1, x2, y2):
    return np.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)

# Calculate tour score
def calc_score(tour, x, y, profit):
    if not tour or len(tour) == 0:
        return 0
    total_profit = sum(profit[i] for i in tour)
    total_dist = 0
    for i in range(len(tour)):
        j = (i + 1) % len(tour)
        total_dist += dist(x[tour[i]], y[tour[i]], x[tour[j]], y[tour[j]])
    total_dist = int(round(total_dist))
    score = total_profit - total_dist
    return score, total_profit, total_dist

# Read tour file
def read_tour(path: str):
    if not os.path.exists(path):
        return None
    with open(path, 'r') as f:
        data = f.read().split()
    tour = [int(x) for x in data if x.strip() != '']
    return tour

# Load coordinates
print("Loading coordinates...")
x_a, y_a, profit_a = read_coords("TSPA.csv")
x_b, y_b, profit_b = read_coords("TSPB.csv")

# Load solution checker
print("Loading solution_checker2.xlsx...")
xl_file = pd.ExcelFile("solution_checker2.xlsx")
print(f"Sheet names: {xl_file.sheet_names}")

# Read both sheets
df_a = pd.read_excel("solution_checker2.xlsx", sheet_name=0)
df_b = pd.read_excel("solution_checker2.xlsx", sheet_name=1)

print("\n=== TSPA ===")
print(df_a)
print("\n=== TSPB ===")
print(df_b)

# Test tours
tours_dir = "zadanie4_tours"
tour_files = {
    "MLSL": ("MLSL_a.txt", "MLSL_b.txt"),
    "ILS": ("ILS_a_final_tour.txt", "ILS_b_final_tour.txt"),
    "LNS local": ("LNS_subpaths_a_final_tour.txt", "LNS_subpaths_b_final_tour.txt"),
    "LNS no_local": ("LNS_subpaths_a_no_local_final_tour.txt", "LNS_subpaths_b_no_local_final_tour.txt"),
}

print("\n" + "="*80)
print("SCORE COMPARISON")
print("="*80)

for algo, (file_a, file_b) in tour_files.items():
    print(f"\n{algo}:")
    
    # Instance A
    tour_a = read_tour(os.path.join(tours_dir, file_a))
    if tour_a:
        score_a, profit_a_val, dist_a = calc_score(tour_a, x_a, y_a, profit_a)
        print(f"  A: score={score_a}, profit={profit_a_val}, distance={dist_a}")
    else:
        print(f"  A: FILE NOT FOUND - {file_a}")
    
    # Instance B
    tour_b = read_tour(os.path.join(tours_dir, file_b))
    if tour_b:
        score_b, profit_b_val, dist_b = calc_score(tour_b, x_b, y_b, profit_b)
        print(f"  B: score={score_b}, profit={profit_b_val}, distance={dist_b}")
    else:
        print(f"  B: FILE NOT FOUND - {file_b}")

print("\n" + "="*80)
print("EXPECTED FROM SOLUTION CHECKER:")
print("="*80)
print("\nTSPA:")
print(df_a.to_string())
print("\nTSPB:")
print(df_b.to_string())
