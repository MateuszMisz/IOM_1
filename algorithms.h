#pragma once
#include "tsp.h"
#include <vector>
#include <random>
#include <functional>
#include "neighbourhood.h"

using AlgoFunc = std::function<std::vector<int>(int, const TSPInstance&, bool)>;
using AlgoFuncWalk = std::function<std::vector<int>(int, const TSPInstance&,std::vector<int>, InTourMoveType)>;
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
/**
 * double avg; // średni czas wykonania
 * long long min, max; // minimalny i maksymalny czas wykonania (w mikrosekundach)
 */
struct TimeStats {
    double avg;
    long long min, max;
};
/**
 * pola:
 *  - TimeStats time_stats; // statystyki czasu wykonania
 *  - Stats score_stats; // statystyki wyników (po fazie I)
 *  - std::vector<int> bestTour; // najlepsze rozwiązanie końcowe
 */
struct AlgoStatsTimed {
    TimeStats time_stats;
    Stats score_stats;// wyniki po fazie I (pełny cykl Hamiltona)
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

enum class BaseSolutionType { best, random };
/**
 * zwraca rozwiązanie bazowe ( wygenerowane przez regretGC bez uwzględniania profitów).
 * dla runs > 1, zwraca najlepsze rozwiązanie spośród `runs` uruchomień (różne starty).
 */
std::vector<int> getBaseSolution(const TSPInstance& tsp, BaseSolutionType type, int runs = 1);

AlgoStatsTimed collectStatsWalk(AlgoFuncWalk algo, const TSPInstance& tsp, InTourMoveType move_type, int n = -1);
AlgoStatsTimed collectRandomWalkStats(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, int runs, std::mt19937& rng,double one_run_time_limit);

std::vector<int> steepestWalk(int start, const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type);
std::vector<int> greedyWalk(int start, const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type);
std::vector<int> randomWalk(int start, const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, double time_limit);
