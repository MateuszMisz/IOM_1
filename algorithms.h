#pragma once
#include "tsp.h"
#include <vector>
#include <random>
#include <functional>


using AlgoFunc = std::function<std::vector<int>(int, const TSPInstance&, bool)>;
/**
 * double avg, 
 * int min, max
 */
struct Stats {
    double avg;
    int    min, max;
};
/**
 * Zbiera statystyki dla algorytmów fazy I i II. Zawiera też najlepsze rozwiązanie końcowe (po fazie II) spośród wszystkich startów
 * Stats(double avg, int min, int max) phaseI
 * StatsphaseII
 * std::vector<int>bestTour
 */
struct AlgoStats {
    Stats phaseI;   // wyniki po fazie I (pełny cykl Hamiltona)
    Stats phaseII;  // wyniki po fazie II (po usunięciu wierzchołków)
    std::vector<int> bestTour;  // najlepsze rozwiązanie końcowe
};

std::vector<int> randomSolution(int n, std::mt19937& rng);

std::vector<int> greedyNN(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> greedyGC(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> regretGC(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> weightedRegretGC(int start, const TSPInstance& tsp, bool useProfit,
                                  double wRegret = 1.0, double wCost = 1.0);

std::vector<int> phaseII(std::vector<int> tour, const TSPInstance& tsp);

// Uruchamia algorytm ze wszystkich startów, zbiera statystyki dla fazy I i II.
AlgoStats collectStats(AlgoFunc algo, const TSPInstance& tsp, bool useProfit);

// Uruchamia losowe rozwiązanie `runs` razy, zbiera statystyki (przed i po fazie II).
AlgoStats collectRandomStats(const TSPInstance& tsp, int runs, std::mt19937& rng);
