#pragma once
#include "tsp.h"
#include <vector>
#include <random>
#include <functional>
#include "neighbourhood.h"
#include <climits>

using AlgoFunc = std::function<std::vector<int>(int, const TSPInstance&, bool)>;
using AlgoFuncWalk = std::function<std::vector<int>( const TSPInstance&,std::vector<int>, InTourMoveType,std::mt19937&)>;
/**
 * double avg, 
 * int min, max
 */
enum class RemovalMode {
    WorstVertex,   // Usuwa wierzchołki o najwyższym koszcie (dist(a,v) + dist(v,b))
    WorstEdge,     // Usuwa oba końce najdłuższych krawędzi
    Random,        // Usuwa losowe wierzchołki (dobre na ucieczkę z minimów lokalnych)
    RandomSubpath, // Usuwa jeden losowy spójny podciąg wierzchołków
    AllRandom      // Losuje jedną z powyższych strategii przy każdym wywołaniu destroy
};
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
struct PerturbationStats{
    double avg;
    int min,max, best;
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
    PerturbationStats perturbation_stats; // statystyki perturbacji (średnia, min, max, najlepsza)
    AlgoStatsTimed(): time_stats({0.0, LLONG_MAX, LLONG_MIN}), score_stats({0.0, INT_MAX, INT_MIN}), bestTour({}), perturbation_stats({0, INT_MAX, INT_MIN, 0}) {}
};

std::vector<int> randomSolution(int n, std::mt19937& rng);

std::vector<int> greedyNN(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> greedyGC(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> regretGC(int start, const TSPInstance& tsp, bool useProfit);
std::vector<int> weightedRegretGC(int start, const TSPInstance& tsp, bool useProfit,
                                  double wRegret = 1.0, double wCost = 1.0);

std::vector<int> phaseII(std::vector<int> tour, const TSPInstance& tsp);

// Uruchamia algorytm ze wszystkich startów, zbiera statystyki dla fazy I i II.
AlgoStats collectStats(AlgoFunc algo, const TSPInstance& tsp, bool useProfit,int n=-1);

// Uruchamia losowe rozwiązanie `runs` razy, zbiera statystyki (przed i po fazie II).
AlgoStats collectRandomStats(const TSPInstance& tsp, int runs, std::mt19937& rng);

enum class BaseSolutionType { best, random };
/**
 * zwraca rozwiązanie bazowe ( wygenerowane przez regretGC bez uwzględniania profitów).
 * dla runs > 1, zwraca najlepsze rozwiązanie spośród `runs` uruchomień (różne starty).
 */
std::pair<AlgoStats,std::vector<std::vector<int>>> getBaseSolutions(AlgoFunc algo, TSPInstance& tsp, BaseSolutionType type, int n);
std::pair<AlgoStats,std::vector<std::vector<int>>> getBaseSolutionsRandom(const TSPInstance& tsp, int runs, std::mt19937& rng);

AlgoStatsTimed collectStatsWalk(AlgoFuncWalk algo, std::vector<int> base_solution,const TSPInstance& tsp, InTourMoveType move_type, int n = -1);
AlgoStatsTimed collectRandomWalkStats(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, int runs, std::mt19937& rng,double one_run_time_limit);

std::vector<int> steepestWalk(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng);
std::vector<int> steepestWalkLM(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng);
std::vector<int> steepestWalkCandidate(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng);
std::vector<int> greedyWalk(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng);
std::vector<int> randomWalk(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, double time_limit, std::mt19937& rng);
Neighbour generateRandomNeigbour(const std::vector<int>& tour, const TSPInstance& tsp,std::mt19937& rng);
AlgoStatsTimed MLSL(const TSPInstance& tsp, std::mt19937& rng, int runs=20, int iterations_per_run=200, AlgoFuncWalk local_search = steepestWalkLM);
std::tuple<std::vector<int>, int, long long, int>collectMLSLOneRun(AlgoFuncWalk algo,const TSPInstance& tsp, InTourMoveType move_type, int n, std::mt19937& rng); ;
AlgoStatsTimed ILS(const TSPInstance& tsp,std::mt19937& rng,int runs ,long long time_limit, int moves_in_perturbation, AlgoFuncWalk local_search = steepestWalkLM);
std::tuple<std::vector<int>,int,long long,int> ILSOneRun(const TSPInstance& tsp, std::mt19937& rng, long long time_limit, int moves_in_perturbation, AlgoFuncWalk local_search);
std::vector<int> perturbation(std::vector<int> tour, const TSPInstance& tsp, int moves_in_perturbation, std::mt19937& rng, MoveType move_type);    
Neighbour generateRandomMove(const std::vector<int>& tour,std::vector<bool>&inTour, const TSPInstance& tsp,std::mt19937& rng,MoveType moveType);
AlgoStatsTimed LNS(const TSPInstance& tsp,std::mt19937& rng,int runs ,long long time_limit, float destruction_rate, RemovalMode mode, bool use_local_search, AlgoFuncWalk local_search = steepestWalkLM);
std::tuple<std::vector<int>,int,long long,int> LNSOneRun(const TSPInstance& tsp, std::mt19937& rng, long long time_limit, float destruction_rate, RemovalMode mode, bool use_local_search, AlgoFuncWalk local_search);
std::vector<int> destroy(const std::vector<int>& tour, const TSPInstance& tsp, float destruction_rate, RemovalMode mode, std::mt19937& rng);
std::vector<int> repair(const std::vector<int>& partial_tour, const TSPInstance& tsp, std::mt19937& rng);