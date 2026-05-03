#include "algorithms.h"
#include <algorithm>
#include <numeric>
#include <climits>
#include <iostream>
#include <chrono>
#include "neighbourhood.h"
#include <random>
#include <stdexcept>
#include <string>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <optional>
// std::vector<int> free_vertices;
// std::vector<std::pair<MoveType,std::pair<int,int>>>roll_back_neighbours;
// Delta funkcji celu przy wstawieniu v między a i b:
// delta = profit[v] + dist[a][b] - dist[a][v] - dist[v][b]
// Jeśli useProfit=false, pomijamy profit (faza I nie uwzględnia zysku).
extern std::vector<AlgoStats> phaseIA_length;
extern std::vector<AlgoStats> phaseIB_length;
extern bool useA;
static inline int insertionDelta(int previous, int inserted, int latter,
                                 const TSPInstance& tsp, bool useProfit) {
    int d = tsp.dist[previous][latter] - tsp.dist[previous][inserted] - tsp.dist[inserted][latter];
    if (useProfit) d += tsp.nodes[inserted].profit;
    return d;
}
static inline int removalDelta(int previous, int removed, int latter,
                                 const TSPInstance& tsp, bool useProfit) {
    int d = -tsp.dist[previous][latter] + tsp.dist[previous][removed] + tsp.dist[removed][latter];
    if (useProfit) d -= tsp.nodes[removed].profit;
    return d;
}
static inline int swapVerticesDelta(int prev_1, int to_swap_1, int latter_1,
                                   int prev_2, int to_swap_2, int latter_2,
                                   const TSPInstance& tsp) {
    int d = 0;
    if(to_swap_1 == prev_2){
        d+=tsp.dist[prev_1][to_swap_1]+tsp.dist[to_swap_2][latter_2] - tsp.dist[prev_1][to_swap_2] - tsp.dist[to_swap_1][latter_2];
    }
    else if (to_swap_1 == latter_2){
        d+=tsp.dist[prev_2][to_swap_2]+tsp.dist[to_swap_1][latter_1]  - tsp.dist[to_swap_2][latter_1] - tsp.dist[prev_2][to_swap_1];
    }
    else{
    // Usuwamy to_swap_1 i wstawiamy to_swap_2
    d += tsp.dist[prev_1][to_swap_1] + tsp.dist[to_swap_1][latter_1] - tsp.dist[prev_1][to_swap_2] - tsp.dist[to_swap_2][latter_1];
    // Usuwamy to_swap_2 i wstawiamy to_swap_1
    d += tsp.dist[prev_2][to_swap_2] + tsp.dist[to_swap_2][latter_2] - tsp.dist[prev_2][to_swap_1] - tsp.dist[to_swap_1][latter_2];
    }
    return d;
}

static inline int swapEdgesDelta(int previous, int first_to_swap, int last_to_swap,int latter, const TSPInstance& tsp) {
    int d = 0;
    // Usuwamy krawędzie (previous, to_reverse[0]), (to_reverse[i], to_reverse[i+1]) dla i=0..k-2 oraz (to_reverse[k-1], latter)
    // i wstawiamy krawędzie (previous, to_reverse[k-1]), (to_reverse[i], to_reverse[i-1]) dla i=k-1..1 oraz (to_reverse[0], latter)
    d += tsp.dist[previous][first_to_swap] + tsp.dist[last_to_swap][latter] - tsp.dist[previous][last_to_swap] - tsp.dist[first_to_swap][latter];
    return d;
}

// Delta funkcji celu przy dołączeniu v jako kolejnego wierzchołka w NN:
// delta = profit[v] - dist[curr][v]
static inline int nnDelta(int curr, int v,
                           const TSPInstance& tsp, bool useProfit) {
    int d = -tsp.dist[curr][v];
    if (useProfit) d += tsp.nodes[v].profit;
    return d;
}

/**
 * @return std::vector<int> zawierający indeksy kolejnych wierzcholkow w losowej sciezce
 */
std::vector<int> randomSolution(int n, std::mt19937& rng) {
    std::uniform_int_distribution<int> sizeDist(2, n);
    int k = sizeDist(rng);

    std::vector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);

    return std::vector<int>(indices.begin(), indices.begin() + k);
}


std::vector<int> greedyNN(int start, const TSPInstance& tsp, bool useProfit) {
    int n = tsp.size();
    std::vector<bool> visited(n, false);
    std::vector<int> tour;
    tour.reserve(n);

    tour.push_back(start);
    visited[start] = true;

    for (int step = 1; step < n; ++step) {
        int curr      = tour.back();
        int bestNext  = -1;
        int bestDelta = INT_MIN;

        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            int delta = nnDelta(curr, j, tsp, useProfit);
            if (delta > bestDelta) {
                bestDelta = delta;
                bestNext  = j;
            }
        }
        tour.push_back(bestNext);
        visited[bestNext] = true;
    }
    return tour;
}


// Inicjalizacja: dodaje drugi wierzchołek do cyklu (najlepsza delta).
static void addSecondNode(std::vector<int>& tour, std::vector<bool>& inTour,
                          const TSPInstance& tsp, bool useProfit) {
    int start     = tour[0];
    int best      = -1;
    int bestDelta = INT_MIN;
    for (int j = 0; j < tsp.size(); ++j) {
        if (inTour[j]) continue;
        // Przy k=1 "cykl" ma jedną krawędź start→start (długość 0),
        // więc delta wstawienia = nnDelta (brak krawędzi do zastąpienia).
        int delta = nnDelta(start, j, tsp, useProfit);
        if (delta > bestDelta) { bestDelta = delta; best = j; }
    }
    tour.push_back(best);
    inTour[best] = true;
}


std::vector<int> greedyGC(int start, const TSPInstance& tsp, bool useProfit) {
    int n = tsp.size();
    std::vector<bool> inTour(n, false);
    std::vector<int> tour;
    tour.reserve(n);

    tour.push_back(start);
    inTour[start] = true;
    addSecondNode(tour, inTour, tsp, useProfit);

    while ((int)tour.size() < n) {
        int bestV     = -1;
        int bestPos   = -1;
        int bestDelta = INT_MIN;
        int k = static_cast<int>(tour.size());

        for (int v = 0; v < n; ++v) {
            if (inTour[v]) continue;
            for (int i = 0; i < k; ++i) {
                int delta = insertionDelta(tour[i], v, tour[(i + 1) % k], tsp, useProfit);
                if (delta > bestDelta) {
                    bestDelta = delta;
                    bestV     = v;
                    bestPos   = i + 1;
                }
            }
        }
        tour.insert(tour.begin() + bestPos, bestV);
        inTour[bestV] = true;
    }
    return tour;
}


std::vector<int> regretGC(int start, const TSPInstance& tsp, bool useProfit) {
    int n = tsp.size();
    std::vector<bool> inTour(n, false);
    std::vector<int> tour;
    tour.reserve(n);

    tour.push_back(start);
    inTour[start] = true;
    addSecondNode(tour, inTour, tsp, useProfit);

    while ((int)tour.size() < n) {
        int bestV      = -1;
        int bestPos    = -1;
        int bestRegret = INT_MIN;
        int bestDelta1 = INT_MIN;  // preferuj wyższą delta1
        int k = static_cast<int>(tour.size());

        for (int v = 0; v < n; ++v) {
            if (inTour[v]) continue;

            int delta1 = INT_MIN, delta2 = INT_MIN;
            int pos1   = -1;

            for (int i = 0; i < k; ++i) {
                int delta = insertionDelta(tour[i], v, tour[(i + 1) % k], tsp, useProfit);
                if (delta > delta1) {
                    delta2 = delta1;
                    delta1 = delta;
                    pos1   = i + 1;
                } else if (delta > delta2) {
                    delta2 = delta;
                }
            }

            // 2-żal: ile tracimy nie wybierając v teraz (delta1 - delta2)
            int regret = delta1 - delta2;
            if (regret > bestRegret || (regret == bestRegret && delta1 > bestDelta1)) {
                bestRegret = regret;
                bestV      = v;
                bestPos    = pos1;
                bestDelta1 = delta1;
            }
        }

        tour.insert(tour.begin() + bestPos, bestV);
        inTour[bestV] = true;
    }
    return tour;
}


std::vector<int> weightedRegretGC(int start, const TSPInstance& tsp, bool useProfit,
                                  double wRegret, double wCost) {
    int n = tsp.size();
    std::vector<bool> inTour(n, false);
    std::vector<int> tour;
    tour.reserve(n);

    tour.push_back(start);
    inTour[start] = true;
    addSecondNode(tour, inTour, tsp, useProfit);

    while ((int)tour.size() < n) {
        int    bestV     = -1;
        int    bestPos   = -1;
        double bestScore = -1e18;
        int k = static_cast<int>(tour.size());

        for (int v = 0; v < n; ++v) {
            if (inTour[v]) continue;

            int delta1 = INT_MIN, delta2 = INT_MIN;
            int pos1   = -1;

            for (int i = 0; i < k; ++i) {
                int delta = insertionDelta(tour[i], v, tour[(i + 1) % k], tsp, useProfit);
                if (delta > delta1) {
                    delta2 = delta1;
                    delta1 = delta;
                    pos1   = i + 1;
                } else if (delta > delta2) {
                    delta2 = delta;
                }
            }

            // score = wRegret * (delta1 - delta2) + wCost * delta1
            // domyślnie wRegret=1, wCost=1 → 2*delta1 - delta2
            double score = wRegret * (delta1 - delta2) + wCost * delta1;
            if (score > bestScore) {
                bestScore = score;
                bestV     = v;
                bestPos   = pos1;
            }
        }

        tour.insert(tour.begin() + bestPos, bestV);
        inTour[bestV] = true;
    }
    return tour;
}

/**
 * rekurencyjnie usuwa wierzchołki dla ktorych skrocenie trasy jest lepsze niz ich profit
 */
std::vector<int> phaseII(std::vector<int> tour, const TSPInstance& tsp) {
    bool improved = true;
    while (improved && tour.size() > 2) {
        improved = false;
        int bestDelta = 0;  // szukamy delty > 0 (poprawa)
        int bestIdx   = -1;
        int k = static_cast<int>(tour.size());

        for (int i = 0; i < k; ++i) {
            int prev = tour[(i - 1 + k) % k];
            int curr = tour[i];
            int next = tour[(i + 1) % k];
            // Delta funkcji celu po usunięciu curr:
            //   tracimy profit[curr], zyskujemy skrócenie trasy
            int delta = tsp.dist[prev][curr] + tsp.dist[curr][next]
                       - tsp.dist[prev][next] - tsp.nodes[curr].profit;
            if (delta > bestDelta) {
                bestDelta = delta;
                bestIdx   = i;
            }
        }

        if (bestIdx != -1) {
            tour.erase(tour.begin() + bestIdx);
            improved = true;
        }
    }
    return tour;
}

std::pair<AlgoStats,std::vector<std::vector<int>>> getBaseSolutions(AlgoFunc algo, TSPInstance& tsp, BaseSolutionType type, int n) {
    if (n == -1) n = tsp.size();

    Stats s1{ 0.0, INT_MAX, INT_MIN };
    Stats s2{ 0.0, INT_MAX, INT_MIN };
    std::vector<int> bestTour;
    std::vector<std::vector<int>> solutions;
    int bestScore = INT_MIN;
    int bestScore1 = INT_MIN;
    std::vector<int> bestTour1;
    for (int s = 0; s < n; ++s) {
        auto tour    = algo(s, tsp, true);
        int  score1  = tsp.evaluate(tour);
        //dodanie statystyk długości trasy po fazie I
        // int length1 = tsp.evaluate_distance(tour);
        // s1_len.avg += length1;
        // s1_len.min  = std::min(s1_len.min, length1);
        // s1_len.max  = std::max(s1_len.max, length1);
        if(score1 > bestScore1) {
            bestScore1 = score1;
            bestTour1 = tour;
        }
        // if (tour.size() < 2){
        //     std::cout<< "Warning: tour size = " << tour.size() << " for start = " << s << "\n";
        // }  // nie ma fazy II dla 0/1 wierzchołka
        //koniec dodatku
        s1.avg += score1;
        s1.min  = std::min(s1.min, score1);
        s1.max  = std::max(s1.max, score1);

        tour         = phaseII(tour, tsp);
        solutions.push_back(tour);
        int  score2  = tsp.evaluate(tour);
        s2.avg += score2;
        s2.min  = std::min(s2.min, score2);
        s2.max  = std::max(s2.max, score2);

        if (score2 > bestScore) {
            bestScore = score2;
            bestTour  = tour;
        }
    }

    s1.avg /= n;
    s2.avg /= n;
    // spushowanie statystyk długości trasy po fazie I oraz najlepszego rozwiązania po fazie I(bo nie ma ich w oryginale)
    // s1_len.avg /= n;
    // if(useA) {
    //     phaseIA_length.push_back({s1_len, s1_len, bestTour1});
    // } else {
    //     phaseIB_length.push_back({s1_len, s1_len, bestTour1});
    // }
    return std::make_pair(AlgoStats{ s1, s2, bestTour }, solutions);

}
AlgoStatsTimed collectStatsWalk(AlgoFuncWalk algo, std::vector<int> base_solution,const TSPInstance& tsp, InTourMoveType move_type, int n) {
    if (n == -1) n = tsp.size();
    
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    std::vector<int> bestTour;
    std::mt19937 rng(69);
    int bestScore = INT_MIN;
    
    for (int s = 0; s < n; ++s) {
        // printf(" %d/%d\n", s+1, n);
        auto start_time = std::chrono::high_resolution_clock::now();
        auto tour    = algo( tsp, base_solution, move_type, rng);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

        int  score1  = tsp.evaluate(tour);
        score_stats.avg += score1;
        score_stats.min  = std::min(score_stats.min, score1);
        score_stats.max  = std::max(score_stats.max, score1);
        time_stats.avg += duration;
        time_stats.min  = std::min(time_stats.min, duration);
        time_stats.max  = std::max(time_stats.max, duration);

        if (score1 > bestScore) {
            bestScore = score1;
            bestTour  = tour;
        }
    }

    score_stats.avg /= n;
    time_stats.avg /= n;
    AlgoStatsTimed result;
    result.time_stats = time_stats;
    result.score_stats = score_stats;
    result.bestTour = bestTour;
    return result;
}

std::pair<AlgoStats,std::vector<std::vector<int>>> getBaseSolutionsRandom(const TSPInstance& tsp, int runs, std::mt19937& rng) {
    Stats s1{ 0.0, INT_MAX, INT_MIN };
    Stats s2{ 0.0, INT_MAX, INT_MIN };
    // Stats s1_len{ 0.0, INT_MAX, INT_MIN };
    int bestScore1 = INT_MIN;
    std::vector<std::vector<int>> solutions;
    std::vector<int> bestTour;
    int bestScore = INT_MIN;
    std::vector<int> bestTour1;

    for (int i = 0; i < runs; ++i) {
        auto tour   = randomSolution(tsp.size(), rng);
        solutions.push_back(tour);
        int  score1 = tsp.evaluate(tour);
        // int length1 = tsp.evaluate_distance(tour);
        // s1_len.avg += length1;
        // s1_len.min  = std::min(s1_len.min, length1);
        // s1_len.max  = std::max(s1_len.max, length1);
        if(score1 > bestScore1) {
            bestScore1 = score1;
            bestTour1 = tour;
        }
        s1.avg += score1;
        s1.min  = std::min(s1.min, score1);
        s1.max  = std::max(s1.max, score1);

        tour    = phaseII(tour, tsp);
        int  score2 = tsp.evaluate(tour);
        s2.avg += score2;
        s2.min  = std::min(s2.min, score2);
        s2.max  = std::max(s2.max, score2);

        if (score2 > bestScore) {
            bestScore = score2;
            bestTour  = tour;
        }
    }
    // s1_len.avg/= runs;
    //     if(useA) {
    //     phaseIA_length.push_back({s1_len, s1_len, bestTour1});
    // } else {
    //     phaseIB_length.push_back({s1_len, s1_len, bestTour1});
    // }
    s1.avg /= runs;
    s2.avg /= runs;
    return std::make_pair(AlgoStats{ s1, s2, bestTour }, solutions);
}

AlgoStats collectStats(AlgoFunc algo, const TSPInstance& tsp, bool useProfit, int n) {
    if (n == -1) n = tsp.size();

    Stats s1{ 0.0, INT_MAX, INT_MIN };
    Stats s1_len{ 0.0, INT_MAX, INT_MIN };
    Stats s2{ 0.0, INT_MAX, INT_MIN };
    std::vector<int> bestTour;
    int bestScore = INT_MIN;
    int bestScore1 = INT_MIN;
    std::vector<int> bestTour1;
    for (int s = 0; s < n; ++s) {
        auto tour    = algo(s, tsp, useProfit);
        int  score1  = tsp.evaluate(tour);
        //dodanie statystyk długości trasy po fazie I
        int length1 = tsp.evaluate_distance(tour);
        s1_len.avg += length1;
        s1_len.min  = std::min(s1_len.min, length1);
        s1_len.max  = std::max(s1_len.max, length1);
        if(score1 > bestScore1) {
            bestScore1 = score1;
            bestTour1 = tour;
        }
        // if (tour.size() < 2){
        //     std::cout<< "Warning: tour size = " << tour.size() << " for start = " << s << "\n";
        // }  // nie ma fazy II dla 0/1 wierzchołka
        //koniec dodatku
        s1.avg += score1;
        s1.min  = std::min(s1.min, score1);
        s1.max  = std::max(s1.max, score1);

        tour         = phaseII(tour, tsp);
        int  score2  = tsp.evaluate(tour);
        s2.avg += score2;
        s2.min  = std::min(s2.min, score2);
        s2.max  = std::max(s2.max, score2);

        if (score2 > bestScore) {
            bestScore = score2;
            bestTour  = tour;
        }
    }

    s1.avg /= n;
    s2.avg /= n;
    // spushowanie statystyk długości trasy po fazie I oraz najlepszego rozwiązania po fazie I(bo nie ma ich w oryginale)
    s1_len.avg /= n;
    if(useA) {
        phaseIA_length.push_back({s1_len, s1_len, bestTour1});
    } else {
        phaseIB_length.push_back({s1_len, s1_len, bestTour1});
    }
    return { s1, s2, bestTour };
}

AlgoStats collectRandomStats(const TSPInstance& tsp, int runs, std::mt19937& rng) {
    Stats s1{ 0.0, INT_MAX, INT_MIN };
    Stats s2{ 0.0, INT_MAX, INT_MIN };
    Stats s1_len{ 0.0, INT_MAX, INT_MIN };
    int bestScore1 = INT_MIN;

    std::vector<int> bestTour;
    int bestScore = INT_MIN;
    std::vector<int> bestTour1;

    for (int i = 0; i < runs; ++i) {
        auto tour   = randomSolution(tsp.size(), rng);
        int  score1 = tsp.evaluate(tour);
        int length1 = tsp.evaluate_distance(tour);
        s1_len.avg += length1;
        s1_len.min  = std::min(s1_len.min, length1);
        s1_len.max  = std::max(s1_len.max, length1);
        if(score1 > bestScore1) {
            bestScore1 = score1;
            bestTour1 = tour;
        }
        s1.avg += score1;
        s1.min  = std::min(s1.min, score1);
        s1.max  = std::max(s1.max, score1);

        tour    = phaseII(tour, tsp);
        int  score2 = tsp.evaluate(tour);
        s2.avg += score2;
        s2.min  = std::min(s2.min, score2);
        s2.max  = std::max(s2.max, score2);

        if (score2 > bestScore) {
            bestScore = score2;
            bestTour  = tour;
        }
    }
    s1_len.avg/= runs;
        if(useA) {
        phaseIA_length.push_back({s1_len, s1_len, bestTour1});
    } else {
        phaseIB_length.push_back({s1_len, s1_len, bestTour1});
    }
    s1.avg /= runs;
    s2.avg /= runs;
    return { s1, s2, bestTour };
}
AlgoStatsTimed collectRandomWalkStats(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, int runs, std::mt19937& rng,double one_run_time_limit) {
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    std::vector<int> bestTour;
    int bestScore = INT_MIN;
    for (int i = 0; i < runs; ++i) {
        // std::cout<<"new run";
        auto start_time = std::chrono::high_resolution_clock::now();
        auto tour   = randomWalk(tsp, base_solution, move_type, one_run_time_limit, rng);
        auto end_time = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

        int  score1 = tsp.evaluate(tour);
        score_stats.avg += score1;
        score_stats.min  = std::min(score_stats.min, score1);
        score_stats.max  = std::max(score_stats.max, score1);
        time_stats.avg += duration;
        time_stats.min  = std::min(time_stats.min, duration);
        time_stats.max  = std::max(time_stats.max, duration);

        if (score1 > bestScore) {
            bestScore = score1;
            bestTour  = tour;
        }
    }
    score_stats.avg /= runs;
    time_stats.avg /= runs;
    AlgoStatsTimed result;
    result.time_stats = time_stats;
    result.score_stats = score_stats;
    result.bestTour = bestTour;
    return result;

}
int transformation_delta(const Neighbour& neighbor,std::vector<int>& tour, const TSPInstance& tsp) {
    int delta = 0;
    // if( neighbor.type == MoveType::empty) {
    //     puts("problem");
    // }
    int k= static_cast<int>(tour.size());
    switch(neighbor.type) {
            case MoveType::Add:
                // if(neighbor.first<0||neighbor.second<0 || neighbor.first >70000 || neighbor.second >70000 ||tour[(neighbor.second+k)%k] >70000 || tour[(neighbor.second-1+k)% k] >70000)
                //     puts("problem");
                if (k == 0)
                    delta = tsp.nodes[neighbor.first].profit;
                else
                    delta = insertionDelta(tour[(neighbor.second-1+k)% k],neighbor.first,tour[(neighbor.second+k)%k], tsp, true);
                break;
            case MoveType::remove:
                delta = removalDelta(tour[(neighbor.first-1+k)% k],tour[neighbor.first],tour[(neighbor.first + 1+k)% k], tsp, true);
                break;
            case MoveType::swap_vertices:
                // if ( tour[(neighbor.first-1+k)%k]<0|| tour[neighbor.first]<0 || tour[(neighbor.first + 1+k)%k]<0 || tour[(neighbor.second-1+k)%k]<0 || tour[neighbor.second]<0 || tour[(neighbor.second + 1+k)%k]<0) {
                //     puts("stop");
                // }
                if(tour.size()<=2)
                    delta = 0;
                else
                    delta = swapVerticesDelta(tour[(neighbor.first-1+k)% k], tour[neighbor.first], tour[(neighbor.first + 1+k)% k],tour[(neighbor.second-1+k)% k],tour[neighbor.second],tour[(neighbor.second + 1+k)% k], tsp);
                break;
            case MoveType::swap_edges:
                delta = swapEdgesDelta(tour[neighbor.first], tour[(neighbor.first + 1+k)% k], tour[neighbor.second], tour[(neighbor.second + 1+k)% k], tsp);
                break;
            }
    return delta;
}
std::vector<int> randomWalk( const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, double time_limit,std::mt19937& rng) {
    std::vector<int> tour = base_solution;
    std::vector<int> bestTour = tour;
    int bestScore= tsp.evaluate(tour);
    int score = bestScore;
    int delta = 0;
    double elapsed_time = 0.0;
    auto walk_start_time = std::chrono::high_resolution_clock::now();

    while(elapsed_time < time_limit) {

        std::vector<Neighbour> neighbors = generateNeighbourhood(tour, tsp, move_type);
        if (neighbors.empty()) throw std::runtime_error("No neighbor generated in randomWalk");

        std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size()) - 1);
        int choosen_idx = dist(rng);
        Neighbour neighbor = neighbors[choosen_idx];
        delta = transformation_delta(neighbor, tour, tsp);
        neighbor.transform(tour, tsp);
        score += delta;
        if( score > bestScore) {
            bestTour = tour;
            bestScore = score;
        }
        auto now = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(now - walk_start_time).count();
    }
    return bestTour;
}
std::string moveTypeToString(MoveType type) {
    switch (type) {
        case MoveType::Add:           return "Add";
        case MoveType::remove:        return "Remove";
        case MoveType::swap_vertices: return "SwapVertices";
        case MoveType::swap_edges:    return "SwapEdges";
        case MoveType::empty:         return "None";
        default:                      return "Unknown";
    }
}
struct StoredMove {
    MoveType type;
    int delta;
    int v;
    int a, b, c, d;
};

struct StoredMoveKey {
    int type;
    int v;
    int a, b, c, d;

    bool operator==(const StoredMoveKey& other) const {
        return type == other.type && v == other.v && a == other.a && b == other.b &&
               c == other.c && d == other.d;
    }
};

struct StoredMoveKeyHash {
    std::size_t operator()(const StoredMoveKey& key) const {
        std::size_t h = 1469598103934665603ull;
        auto mix = [&](int x) {
            h ^= static_cast<std::size_t>(x + 1000003);
            h *= 1099511628211ull;
        };
        mix(key.type);
        mix(key.v);
        mix(key.a);
        mix(key.b);
        mix(key.c);
        mix(key.d);
        return h;
    }
};

struct StoredMoveCompare {
    bool operator()(const StoredMove& lhs, const StoredMove& rhs) const {
        return lhs.delta < rhs.delta;
    }
};

using StoredMoveQueue = std::priority_queue<StoredMove, std::vector<StoredMove>, StoredMoveCompare>;
using KnownMoveSet = std::unordered_set<StoredMoveKey, StoredMoveKeyHash>;

static std::vector<int> buildPositions(const std::vector<int>& tour, int n) {
    std::vector<int> pos(n, -1);
    for (int i = 0; i < static_cast<int>(tour.size()); ++i) pos[tour[i]] = i;
    return pos;
}

static bool hasUndirectedEdge(const std::vector<int>& tour, const std::vector<int>& pos, int a, int b) {
    if (a < 0 || b < 0 || a >= static_cast<int>(pos.size()) || b >= static_cast<int>(pos.size())) return false;
    if (pos[a] == -1 || pos[b] == -1 || tour.empty()) return false;
    int k = static_cast<int>(tour.size());
    return tour[(pos[a] + 1) % k] == b || tour[(pos[b] + 1) % k] == a;
}

static int directedEdgeState(const std::vector<int>& tour, const std::vector<int>& pos,
                             int a, int b, int& edgePos) {
    edgePos = -1;
    if (!hasUndirectedEdge(tour, pos, a, b)) return 0;
    int k = static_cast<int>(tour.size());
    if (tour[(pos[a] + 1) % k] == b) {
        edgePos = pos[a];
        return 1;
    }
    edgePos = pos[b];
    return -1;
}

static StoredMoveKey storedMoveKey(const StoredMove& move) {
    StoredMoveKey key{static_cast<int>(move.type), move.v, move.a, move.b, move.c, move.d};
    if ((move.type == MoveType::Add || move.type == MoveType::remove) && key.a > key.b) {
        std::swap(key.a, key.b);
    }
    if (move.type == MoveType::swap_edges) {
        if (key.c < key.a || (key.c == key.a && key.d < key.b)) {
            std::swap(key.a, key.c);
            std::swap(key.b, key.d);
        }
    }
    return key;
}

static void addStoredMove(StoredMoveQueue& moves,
                          KnownMoveSet& known,
                          const StoredMove& move) {
    if (move.delta <= 0) return;
    StoredMoveKey key = storedMoveKey(move);
    if (known.insert(key).second) moves.push(move);
}

static std::vector<bool> buildInTour(const std::vector<int>& tour, int n) {
    std::vector<bool> inTour(n, false);
    for (int v : tour) inTour[v] = true;
    return inTour;
}

static bool areAdjacentEdges(int i, int j, int k) {
    if (i == j) return true;
    if (i > j) std::swap(i, j);
    return j == i + 1 || (i == 0 && j == k - 1);
}

static void addInsertionMovesForVertex(const std::vector<int>& tour, const TSPInstance& tsp,
                                       int inserted, StoredMoveQueue& moves,
                                       KnownMoveSet& known) {
    int n = tsp.size();
    int k = static_cast<int>(tour.size());
    if (inserted < 0 || inserted >= n || k == 0) return;
    for (int i = 0; i < k; ++i) {
        int a = tour[i];
        int b = tour[(i + 1) % k];
        int delta = insertionDelta(a, inserted, b, tsp, true);
        addStoredMove(moves, known, {MoveType::Add, delta, inserted, a, b, -1, -1});
    }
}

static void addInsertionMovesForEdgePosition(const std::vector<int>& tour, const TSPInstance& tsp,
                                             int edgePos, StoredMoveQueue& moves,
                                             KnownMoveSet& known) {
    int n = tsp.size();
    int k = static_cast<int>(tour.size());
    if (k == 0 || edgePos < 0 || edgePos >= k) return;
    std::vector<bool> inTour = buildInTour(tour, n);
    int a = tour[edgePos];
    int b = tour[(edgePos + 1) % k];

    for (int v = 0; v < n; ++v) {
        if (inTour[v]) continue;
        int delta = insertionDelta(a, v, b, tsp, true);
        addStoredMove(moves, known, {MoveType::Add, delta, v, a, b, -1, -1});
    }
}

static void addRemovalMoveForPosition(const std::vector<int>& tour, const TSPInstance& tsp,
                                      int pos, StoredMoveQueue& moves,
                                      KnownMoveSet& known) {
    int k = static_cast<int>(tour.size());
    if (k <= 2 || pos < 0 || pos >= k) return;
    int prev = tour[(pos - 1 + k) % k];
    int v = tour[pos];
    int next = tour[(pos + 1) % k];
    int delta = removalDelta(prev, v, next, tsp, true);
    addStoredMove(moves, known, {MoveType::remove, delta, v, prev, next, -1, -1});
}

static void addSwapMovesForEdgePosition(const std::vector<int>& tour, const TSPInstance& tsp,
                                        int edgePos, StoredMoveQueue& moves,
                                        KnownMoveSet& known) {
    int k = static_cast<int>(tour.size());
    if (k <= 3 || edgePos < 0 || edgePos >= k) return;
    int a = tour[edgePos];
    int b = tour[(edgePos + 1) % k];

    for (int j = 0; j < k; ++j) {
        if (areAdjacentEdges(edgePos, j, k)) continue;
        int c = tour[j];
        int d = tour[(j + 1) % k];
        int sameDirectionDelta = tsp.dist[a][b] + tsp.dist[c][d] - tsp.dist[a][c] - tsp.dist[b][d];
        int reversedDirectionDelta = tsp.dist[a][b] + tsp.dist[c][d] - tsp.dist[a][d] - tsp.dist[b][c];
        addStoredMove(moves, known, {MoveType::swap_edges, sameDirectionDelta, -1, a, b, c, d});
        addStoredMove(moves, known, {MoveType::swap_edges, reversedDirectionDelta, -1, a, b, d, c});
    }
}

static void addAllCurrentImprovingMoves(const std::vector<int>& tour, const TSPInstance& tsp,
                                        StoredMoveQueue& moves,
                                        KnownMoveSet& known) {
    int n = tsp.size();
    int k = static_cast<int>(tour.size());
    std::vector<bool> inTour = buildInTour(tour, n);

    for (int v = 0; v < n; ++v) {
        if (inTour[v]) continue;
        addInsertionMovesForVertex(tour, tsp, v, moves, known);
    }
    for (int i = 0; i < k; ++i) {
        addRemovalMoveForPosition(tour, tsp, i, moves, known);
    }
    for (int i = 0; i < k; ++i) {
        for (int j = i + 2; j < k; ++j) {
            if (i == 0 && j == k - 1) continue;
            int a = tour[i];
            int b = tour[(i + 1) % k];
            int c = tour[j];
            int d = tour[(j + 1) % k];
            int sameDirectionDelta = tsp.dist[a][b] + tsp.dist[c][d] - tsp.dist[a][c] - tsp.dist[b][d];
            int reversedDirectionDelta = tsp.dist[a][b] + tsp.dist[c][d] - tsp.dist[a][d] - tsp.dist[b][c];
            addStoredMove(moves, known, {MoveType::swap_edges, sameDirectionDelta, -1, a, b, c, d});
            addStoredMove(moves, known, {MoveType::swap_edges, reversedDirectionDelta, -1, a, b, d, c});
        }
    }
}

static void pushUniqueIndex(std::vector<int>& values, int value) {
    if (std::find(values.begin(), values.end(), value) == values.end()) values.push_back(value);
}

static void addLocalImprovingMoves(const std::vector<int>& tour, const TSPInstance& tsp,
                                   const std::vector<int>& affectedVertices,
                                   StoredMoveQueue& moves,
                                   KnownMoveSet& known) {
    int n = tsp.size();
    int k = static_cast<int>(tour.size());
    if (k == 0) return;

    std::vector<int> pos = buildPositions(tour, n);
    std::vector<int> edgePositions;
    std::vector<int> removePositions;
    std::vector<int> outsideVertices;

    for (int v : affectedVertices) {
        if (v < 0 || v >= n) continue;
        int p = pos[v];
        if (p == -1) {
            pushUniqueIndex(outsideVertices, v);
            continue;
        }
        pushUniqueIndex(edgePositions, p);
        pushUniqueIndex(edgePositions, (p - 1 + k) % k);
        pushUniqueIndex(removePositions, p);
        pushUniqueIndex(removePositions, (p - 1 + k) % k);
        pushUniqueIndex(removePositions, (p + 1) % k);
    }

    for (int edgePos : edgePositions) {
        addInsertionMovesForEdgePosition(tour, tsp, edgePos, moves, known);
        addSwapMovesForEdgePosition(tour, tsp, edgePos, moves, known);
    }
    for (int posToRemove : removePositions) {
        addRemovalMoveForPosition(tour, tsp, posToRemove, moves, known);
    }
    for (int v : outsideVertices) {
        addInsertionMovesForVertex(tour, tsp, v, moves, known);
    }
}

static bool applyAddMove(const StoredMove& move, std::vector<int>& tour, const TSPInstance& tsp) {
    std::vector<int> pos = buildPositions(tour, tsp.size());
    if (move.v < 0 || move.v >= tsp.size() || pos[move.v] != -1) return false;
    if (!hasUndirectedEdge(tour, pos, move.a, move.b)) return false;
    int k = static_cast<int>(tour.size());
    int pa = pos[move.a];
    int pb = pos[move.b];
    int insertPos = -1;
    if (tour[(pa + 1) % k] == move.b) insertPos = pb;
    else if (tour[(pb + 1) % k] == move.a) insertPos = pa;
    if (insertPos == -1) return false;
    tour.insert(tour.begin() + insertPos, move.v);
    return true;
}

static bool applyRemoveMove(const StoredMove& move, std::vector<int>& tour, const TSPInstance& tsp) {
    if (tour.size() <= 2) return false;
    std::vector<int> pos = buildPositions(tour, tsp.size());
    if (move.v < 0 || move.v >= tsp.size() || pos[move.v] == -1) return false;
    int k = static_cast<int>(tour.size());
    int p = pos[move.v];
    int prev = tour[(p - 1 + k) % k];
    int next = tour[(p + 1) % k];
    if (!((prev == move.a && next == move.b) || (prev == move.b && next == move.a))) return false;
    tour.erase(tour.begin() + p);
    return true;
}

static int checkSwapMove(const StoredMove& move, const std::vector<int>& tour,
                         const TSPInstance& tsp, int& pos1, int& pos2) {
    std::vector<int> pos = buildPositions(tour, tsp.size());
    int state1 = directedEdgeState(tour, pos, move.a, move.b, pos1);
    int state2 = directedEdgeState(tour, pos, move.c, move.d, pos2);
    if (state1 == 0 || state2 == 0) return -1; // krawedz zniknela
    if (state1 != state2) return 0;             // moze byc aplikowalny pozniej
    if (pos1 == pos2) return -1;
    int k = static_cast<int>(tour.size());
    if ((pos1 + 1) % k == pos2 || (pos2 + 1) % k == pos1) return -1;
    return 1;
}

static void applySwapEdgesStored(std::vector<int>& tour, int pos1, int pos2) {
    if (pos1 > pos2) std::swap(pos1, pos2);
    auto start_iter = tour.begin() + pos1 + 1;
    auto end_iter = tour.begin() + pos2 + 1;
    std::reverse(start_iter, end_iter);
}

static bool applyStoredMove(const StoredMove& move, std::vector<int>& tour, const TSPInstance& tsp,
                            bool& removeFromList, std::vector<int>& affectedVertices) {
    removeFromList = true;
    affectedVertices.clear();
    switch (move.type) {
        case MoveType::Add:
            if (applyAddMove(move, tour, tsp)) {
                affectedVertices = {move.v, move.a, move.b};
                return true;
            }
            return false;
        case MoveType::remove:
            if (applyRemoveMove(move, tour, tsp)) {
                affectedVertices = {move.v, move.a, move.b};
                return true;
            }
            return false;
        case MoveType::swap_edges: {
            int pos1 = -1, pos2 = -1;
            int state = checkSwapMove(move, tour, tsp, pos1, pos2);
            if (state == -1) return false;
            if (state == 0) {
                removeFromList = false;
                return false;
            }
            applySwapEdgesStored(tour, pos1, pos2);
            affectedVertices = {move.a, move.b, move.c, move.d};
            return true;
        }
        default:
            return false;
    }
}

std::vector<int> steepestWalk(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng) {
    std::vector<int> tour = base_solution;
    bool improved = true;
    // std::vector<Neighbour> used_neibhbors;
    while(improved){
        improved = false;
        std::vector<Neighbour> neighbors = generateNeighbourhood(tour, tsp, move_type);
        // std::cout<<"Generated " << neighbors.size() << " neighbors\n";
        int bestDelta = 0;
        Neighbour bestNeighbor = Neighbour(MoveType::empty, -1, -1);
        for(Neighbour& neighbor : neighbors) {
            int delta = transformation_delta(neighbor, tour, tsp);
            // std::vector<int> tour_dbg = tour; // tworzymy kopię trasy, aby przetestować transformację
            // neighbor.transform(tour_dbg, tsp); // testujemy transformację na kopii trasy

            // if (neighbor.second ==tour.size() -1){
            //     puts("stop");
            // }
            // int delta_dbg = tsp.evaluate(tour_dbg) - tsp.evaluate(tour); // obliczamy delta na podstawie oceny trasy po transformacji
            // if(delta!= delta_dbg) {
                
            //     std::cout << "Delta mismatch! Calculated: " << delta << ", Evaluated: " << delta_dbg << "\n";
            //     std::cout<< "Neighbor type: " << moveTypeToString(neighbor.type) << ", first: " << neighbor.first << ", second: " << neighbor.second << "\n";
            //     std::cout<< "delta: " <<delta<< "real delta: " << delta_dbg << "\n";
            // }
            if(delta > bestDelta) {
            //     std::vector<int> tour_dbg = tour; // tworzymy kopię trasy, aby przetestować transformację
            //     neighbor.transform(tour_dbg, tsp); // testujemy transformację na kopii trasy
            //     int delta_dbg = tsp.evaluate(tour_dbg) - tsp.evaluate(tour); // obliczamy delta na podstawie oceny trasy po transformacji

            //     if(delta!= delta_dbg) {
                
            //     std::cout << "Delta mismatch! Calculated: " << delta << ", Evaluated: " << delta_dbg << "\n";
            //     std::cout<< "Neighbor type: " << moveTypeToString(neighbor.type) << ", first: " << neighbor.first << ", second: " << neighbor.second << "\n";
            //     std::cout<< "delta: " <<delta<< "real delta: " << delta_dbg << "\n";
            //     std::cout<<"tour0: " << tour[neighbor.first] << ", tour1: " << tour[neighbor.second] << "\n";
            //     std::cout<<"tour_dbg0: " << tour_dbg[neighbor.first] << ", tour_dbg1: " << tour_dbg[neighbor.second] << "\n";
            //     std::cout<<"tsp.dist[last][0]: " << tsp.dist[tour[(neighbor.first-1+tour.size())% tour.size()]][tour[neighbor.first]] << ", tsp.dist[1][2]: " << tsp.dist[tour[neighbor.second]][tour[(neighbor.second + 1+tour.size())% tour.size()]] << "\n";
            //     std::cout<<"tsp.dist[last][1]: " << tsp.dist[tour[(neighbor.first-1+tour.size())% tour.size()]][tour[neighbor.second]] << ", tsp.dist[0][2]: " << tsp.dist[tour[neighbor.first]][tour[(neighbor.second + 1+tour.size())% tour.size()]] << "\n";
            //     std::cout<<"toursize:" << tour.size() << "\n";
            // }
                // if (neighbor.type == MoveType::empty) {
                //     puts("null zwraca delta" );
                // }
                bestDelta = delta;
                bestNeighbor = neighbor;
            }
        }
        if (bestNeighbor.type != MoveType::empty && bestDelta > 0) {
            bestNeighbor.transform(tour, tsp);
            // used_neibhbors.push_back(bestNeighbor);
            improved = true;

        }
        // if(tsp.evaluate(tour) == -5240){
        //     std::cout<<"best neighbor: first: " << bestNeighbor.first << ", second: " << bestNeighbor.second << ", type: " << moveTypeToString(bestNeighbor.type) << "\n";
        //     std::cout<<"tour[first]: " << tour[bestNeighbor.first] << ", tour[second]: " << tour[bestNeighbor.second] << "\n";
        // }
        // std::cout << (improved ? "Yes" : "No") << " " << moveTypeToString(bestNeighbor.type) << " score: " << tsp.evaluate(tour) << "\n";
        // std::cout<<(improved?"Yes":"No") << " " << moveTypeToString(bestNeighbor.type)<< " score:" << tsp.evaluate(tour) << "\n";
    }
    return tour;
}

std::vector<int> steepestWalkLM(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng) {
    (void)rng;
    if (move_type != InTourMoveType::SwapEdges) {
        return steepestWalk(tsp, base_solution, move_type, rng);
    }

    std::vector<int> tour = base_solution;
    StoredMoveQueue improvingMoves;
    KnownMoveSet knownMoves;
    addAllCurrentImprovingMoves(tour, tsp, improvingMoves, knownMoves);

    while (true) {
        bool accepted = false;
        std::vector<StoredMove> deferredMoves;

        while (!improvingMoves.empty()) {
            StoredMove move = improvingMoves.top();
            improvingMoves.pop();
            StoredMoveKey key = storedMoveKey(move);
            if (knownMoves.find(key) == knownMoves.end()) continue;

            bool removeFromList = true;
            std::vector<int> affectedVertices;
            bool applied = applyStoredMove(move, tour, tsp, removeFromList, affectedVertices);
            if (applied) {
                knownMoves.erase(key);
                accepted = true;
                for (const StoredMove& deferred : deferredMoves) improvingMoves.push(deferred);
                addLocalImprovingMoves(tour, tsp, affectedVertices, improvingMoves, knownMoves);
                break;
            }
            if (removeFromList) {
                knownMoves.erase(key);
            } else {
                deferredMoves.push_back(move);
            }
        }

        if (!accepted) {
            for (const StoredMove& deferred : deferredMoves) improvingMoves.push(deferred);
        }
        if (!accepted) break;
    }
    return tour;
}

std::vector<int> steepestWalkCandidate(const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng) {
    (void)rng;
    std::vector<int> tour = base_solution;
    static std::unordered_map<const TSPInstance*, CandidateLists> candidateCache;
    auto cacheIt = candidateCache.find(&tsp);
    if (cacheIt == candidateCache.end()) {
        cacheIt = candidateCache.emplace(&tsp, buildCandidateLists(tsp, 10)).first;
    }
    const CandidateLists& candidates = cacheIt->second;
    bool improved = true;
    while(improved){
        improved = false;
        std::vector<Neighbour> neighbors = generateCandidateNeighbourhood(tour, tsp, move_type, candidates);
        int bestDelta = 0;
        Neighbour bestNeighbor = Neighbour(MoveType::empty, -1, -1);
        for(Neighbour& neighbor : neighbors) {
            int delta = transformation_delta(neighbor, tour, tsp);
            if(delta > bestDelta) {
                bestDelta = delta;
                bestNeighbor = neighbor;
            }
        }
        if (bestNeighbor.type != MoveType::empty && bestDelta > 0) {
            bestNeighbor.transform(tour, tsp);
            improved = true;
        }
    }
    return tour;
}

std::vector<int> greedyWalk( const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, std::mt19937& rng) {
    std::vector<int> tour = base_solution;
    bool improved = true;
    int score = tsp.evaluate(tour);
    int counter = 0;
    // MoveType lastMoveType_dbg = MoveType::empty;
    while(improved) {
        // if (counter++%1000 == 0) {
        //     std::cout << counter<<"Current score: " << score << ", tour size: " << tour.size() << "\n";
        // }
        improved = false;
        std::vector<Neighbour> neighbors = generateNeighbourhood(tour, tsp, move_type);
        
        std::shuffle(neighbors.begin(), neighbors.end(), rng); // losowa kolejność sąsiadów
        
        for(Neighbour& neighbor : neighbors) {
            int delta = transformation_delta(neighbor, tour, tsp);
            // lastMoveType_dbg = neighbor.type;
            if(delta > 0 ) {
                // if (neighbor.type == MoveType::Add || neighbor.type == MoveType::remove)
                //     puts("add/remove zwraca delta");
                neighbor.transform(tour, tsp);
                
                score += delta;
                improved = true;
                
                break; // wybieramy pierwszego sąsiada przynoszącego poprawę
            }
        }
        // std::cout << (improved ? "Yes" : "No") << " " << moveTypeToString(lastMoveType_dbg) << " score: " << score << "\n";
    }
    return tour;
}

AlgoStatsTimed MLSL(const TSPInstance& tsp, std::mt19937& rng, int runs, int iterations_per_run, AlgoFuncWalk local_search) {

    InTourMoveType move_type = InTourMoveType::SwapEdges; // Możesz zmienić na inny typ ruchu, jeśli chcesz
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    PerturbationStats perturbation_stats{ 0, INT_MAX,INT_MIN, 0 };
    std::vector<int> bestTour;
    int delta = 0;
    int best_score = INT_MIN;
    for(int i = 0 ; i < runs; i++){
        std::cout<<"Run " << i+1 << "/" << runs << "\n";
        auto [solution, score, runs_time, n] = collectMLSLOneRun(local_search, tsp, move_type, iterations_per_run, rng);
        

        time_stats.avg += runs_time; 
        time_stats.min  = std::min(time_stats.min, runs_time);
        time_stats.max  = std::max(time_stats.max, runs_time);
        score_stats.avg += score;
        score_stats.min  = std::min(score_stats.min, score);
        score_stats.max  = std::max(score_stats.max, score);
        delta = score - best_score;

        if (delta>0) {
            best_score = score;
            bestTour = solution;
        }
    }
    score_stats.avg /= runs;
    time_stats.avg /= runs;
    AlgoStatsTimed result;
    result.time_stats = time_stats;
    result.score_stats = score_stats;
    result.bestTour = bestTour;
    perturbation_stats.avg = iterations_per_run;
    perturbation_stats.min = iterations_per_run;
    perturbation_stats.max = iterations_per_run;
    perturbation_stats.best= iterations_per_run;
    result.perturbation_stats = perturbation_stats;
    return result;
}
std::tuple<std::vector<int>, int, long long, int> collectMLSLOneRun(AlgoFuncWalk algo,const TSPInstance& tsp, InTourMoveType move_type, int n, std::mt19937& rng) {
    if (n == -1) n = tsp.size();
    std::vector<int> bestTour;
    auto start_time = std::chrono::high_resolution_clock::now();

    // std::mt19937 rng(69);
    int bestScore = INT_MIN;
    for (int s = 0; s < n; ++s) {
        if (s % 30 == 0)
        std::cout<<"Iteration " << s+1 << "/" << n << "\n";
        std::vector<int> base_solution = randomSolution(tsp.size(), rng);

        auto tour    = algo( tsp, base_solution, move_type, rng);
        
       
        int  score1  = tsp.evaluate(tour);

        if (score1 > bestScore) {
            bestScore = score1;
            bestTour  = tour;
        }
    }
     auto end_time = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();


    return {bestTour, bestScore, duration, n};
}

AlgoStatsTimed ILS(const TSPInstance& tsp,std::mt19937& rng ,int runs,long long time_limit, int moves_in_perturbation, AlgoFuncWalk local_search ){
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    PerturbationStats perturbation_stats{ 0, INT_MAX,INT_MIN, 0 };
    std::vector<int> bestTour;
    int best_score = INT_MIN;
    for (int i = 0; i < runs; i++){
        std::cerr<<"Run " << i+1 << "/" << runs << "\n";
        auto [tour, score, duration, perturbation_count] = ILSOneRun(tsp, rng, time_limit, moves_in_perturbation, local_search);
        score_stats.avg += score;
        score_stats.min  = std::min(score_stats.min, score);
        score_stats.max  = std::max(score_stats.max, score);
        time_stats.avg += duration;
        time_stats.min  = std::min(time_stats.min, duration);
        time_stats.max  = std::max(time_stats.max, duration);
        perturbation_stats.avg += perturbation_count;
        perturbation_stats.min  = std::min(perturbation_stats.min, perturbation_count);
        perturbation_stats.max  = std::max(perturbation_stats.max, perturbation_count);
        if (score > best_score) {
            best_score = score;
            bestTour = tour;
            perturbation_stats.best = perturbation_count;
        }
    }
    score_stats.avg /= runs;
    time_stats.avg /= runs;
    perturbation_stats.avg /= runs;
    AlgoStatsTimed result;
    result.time_stats = time_stats;
    result.score_stats = score_stats;
    result.perturbation_stats = perturbation_stats;
    result.bestTour = bestTour;
    return result;
}
/**
 * returns tour, score, duration, perturbation_count
 */
std::tuple<std::vector<int>,int,long long,int> ILSOneRun(const TSPInstance& tsp, std::mt19937& rng, long long time_limit, int moves_in_perturbation, AlgoFuncWalk local_search) {
     auto start_time = std::chrono::high_resolution_clock::now();

    int score;
    long long duration;
    
    int perturbation_count = 0;
    int best_score = INT_MIN;
    std::vector<int> bestTour;

    // std::cerr<<"Generating initial solution...\n";
    std::vector<int> tour = randomSolution(tsp.size(), rng);
  
    // std::cerr<<"Initial solution generated. Starting local search...\n";
    tour = local_search(tsp, tour, InTourMoveType::SwapEdges, rng);
    int current_score = tsp.evaluate(tour);
    bestTour = tour;
    best_score = current_score;
    int debug_counter=0;
    while (true) {
        // std::cerr<<"Perturbation " << perturbation_count+1 <<"time: "<< std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count() <<'/'<< time_limit <<"\n";
        std::vector<int> perturbated_tour  = perturbation(bestTour, tsp, moves_in_perturbation, rng, MoveType::empty);
        // std::cerr<<"Perturbation done. Starting local search...\n";
        perturbated_tour = local_search(tsp, perturbated_tour, InTourMoveType::SwapEdges, rng);
        // std::cerr<<"Local search done. Evaluating solution...\n";
        int perturbated_score = tsp.evaluate(perturbated_tour);
        // std::cerr<<"evaluation done. checking time";
        long long elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
        if (elapsed_time >= time_limit) {
        return {bestTour, best_score, elapsed_time, perturbation_count};
        }
        perturbation_count++;

        if (perturbated_score > best_score) {
            best_score = perturbated_score;
            bestTour = perturbated_tour;
        }
    }
}
/**
 * returns a std::pair<tour_with_perturbation,inTour,changed_indexes(inTour)>
 */
std::vector<int> perturbation(std::vector<int> tour, const TSPInstance& tsp, int moves_in_perturbation, std::mt19937& rng, MoveType move_type) {
    std::vector<bool> inTour(tsp.size(),false);
    for(auto v : tour) inTour[v]=true;
    if (move_type == MoveType::empty)
    {
        std::uniform_int_distribution<int> perturbation_dist(0,2);
        int number = perturbation_dist(rng);
        switch(number){
            case 0:
             move_type = MoveType::Add;
            break;
            case 1:
             move_type = MoveType::remove;
            break;
          
            case 2:
            move_type= MoveType::swap_edges;
            break;
        }
    }
    
    // Validate move type is possible for current tour size
    int currentSize = static_cast<int>(tour.size());
    if (move_type == MoveType::Add && currentSize +moves_in_perturbation>= tsp.size()) {
        move_type = MoveType::remove;
    }
    if (move_type == MoveType::remove && currentSize - moves_in_perturbation <= 2) {
        move_type = MoveType::Add;
    }
    if (move_type == MoveType::swap_edges && currentSize < 4) {
        if(currentSize - moves_in_perturbation <= 2) {
            move_type = MoveType::Add;
        } else {
            move_type = MoveType::remove;
        }
    }
    
    for (int i = 0; i < moves_in_perturbation; ++i) {
        Neighbour neighbor = generateRandomMove(tour, inTour,  tsp, rng, move_type);
        neighbor.transform(tour, tsp);
    }

    return tour;
}   
Neighbour generateRandomMove(const std::vector<int>& tour,std::vector<bool>&inTour, const TSPInstance& tsp,std::mt19937& rng,MoveType moveType) {
    if(moveType == MoveType::empty){
        std::uniform_int_distribution<int> moveTypeDist(0, 2);
        int number = moveTypeDist(rng);
        switch(number){
            case 0:
            moveType = MoveType::Add;
            break;
            case 1:
            moveType = MoveType::remove;
            break;
            case 2:
            moveType= MoveType::swap_edges;
            break;
        }
    }
    
    int n = tsp.size();
    int k = tour.size();
    int idx;
    switch(moveType){
        case MoveType::Add: {
            if (k >= n) {
                throw std::runtime_error("Cannot add: tour is full");
            }
            while(true){
                std::uniform_int_distribution<int> n_dist(0,n-1);
                std::uniform_int_distribution<int> pos_dist(0,k);
                idx = n_dist(rng);
                if (!inTour[idx]) {
                    inTour[idx] = true;
                    return Neighbour(MoveType::Add, idx, pos_dist(rng));
                }
            }
            break;
        }

        case MoveType::remove: {
            if (k <= 2) {
                throw std::runtime_error("Cannot remove: tour too small");
            }
            std::uniform_int_distribution<int> remove_pos_dist(0,k-1);
            idx = remove_pos_dist(rng);
            inTour[tour[idx]] = false;
            return Neighbour(MoveType::remove, idx, -1);
        }

        case MoveType::swap_edges: {
            if (k < 4) {
                throw std::runtime_error("Cannot swap edges: tour too small");
            }
            std::uniform_int_distribution<int> swap_e_dist(0,k-1);
            int first_edge = swap_e_dist(rng);
            int second_edge = swap_e_dist(rng);
            while(second_edge == first_edge || second_edge == (first_edge + 1) % k || first_edge == (second_edge + 1) % k)
                second_edge = swap_e_dist(rng);
            if(first_edge > second_edge) std::swap(first_edge, second_edge);
            return Neighbour(MoveType::swap_edges, first_edge, second_edge);
        }

        case MoveType::empty: {
            throw std::runtime_error("Generated empty move type in generateRandomNeighbour");
        }
    }
    throw std::runtime_error("Unknown move type in generateRandomNeighbour");
}
static std::string removalModeToString(RemovalMode mode) {
    switch (mode) {
        case RemovalMode::WorstVertex: return "WorstVertex";
        case RemovalMode::WorstEdge: return "WorstEdge";
        case RemovalMode::Random: return "Random";
        case RemovalMode::RandomSubpath: return "RandomSubpath";
        case RemovalMode::AllRandom: return "AllRandom";
        default: return "Unknown";
    }
}
AlgoStatsTimed LNS(const TSPInstance& tsp,std::mt19937& rng,int runs ,long long time_limit, float destruction_rate, RemovalMode mode, bool use_local_search, AlgoFuncWalk local_search ){
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    PerturbationStats perturbation_stats{ 0, INT_MAX,INT_MIN, 0 };
    std::vector<int> bestTour;
    int best_score = INT_MIN;
    for (int i = 0; i < runs; i++){
        std::cerr<<"Run " << i+1 << "/" << runs << " with mode " << removalModeToString(mode) << " "<< destruction_rate<<"\n";
        auto [tour, score, duration, perturbation_count] = LNSOneRun(tsp, rng, time_limit, destruction_rate, mode, use_local_search, local_search);
        score_stats.avg += score;
        score_stats.min  = std::min(score_stats.min, score);
        score_stats.max  = std::max(score_stats.max, score);
        time_stats.avg += duration;
        time_stats.min  = std::min(time_stats.min, duration);
        time_stats.max  = std::max(time_stats.max, duration);
        perturbation_stats.avg += perturbation_count;
        perturbation_stats.min  = std::min(perturbation_stats.min, perturbation_count);
        perturbation_stats.max  = std::max(perturbation_stats.max, perturbation_count);
        if (score > best_score) {
            best_score = score;
            bestTour = tour;
            perturbation_stats.best = perturbation_count;
        }
    }
    score_stats.avg /= runs;
    time_stats.avg /= runs;
    perturbation_stats.avg /= runs;
    AlgoStatsTimed result;
    result.time_stats = time_stats;
    result.score_stats = score_stats;
    result.perturbation_stats = perturbation_stats;
    result.bestTour = bestTour;
    return result;
}

std::tuple<std::vector<int>,int,long long,int> LNSOneRun(const TSPInstance& tsp, std::mt19937& rng, long long time_limit, float destruction_rate, RemovalMode mode, bool use_local_search, AlgoFuncWalk local_search) {
    auto start_time = std::chrono::high_resolution_clock::now();

    int score;
    long long duration;
    
    int perturbation_count = 0;
    int best_score = INT_MIN;
    std::vector<int> bestTour;

    std::vector<int> tour = randomSolution(tsp.size(), rng);
  
    tour = local_search(tsp, tour, InTourMoveType::SwapEdges, rng);
    int current_score = tsp.evaluate(tour);
    bestTour = tour;
    best_score = current_score;
    int debug_counter=0;
    std::uniform_int_distribution<int> mode_dist(0, 2);
    while (true) {
        RemovalMode activeMode = mode;
        if (mode == RemovalMode::AllRandom) {
            activeMode = static_cast<RemovalMode>(mode_dist(rng));
        }

        std::vector<int> tmp_solution = destroy(tour, tsp, destruction_rate, activeMode, rng);
        tmp_solution = repair(tmp_solution,tsp,rng);
        if(use_local_search) {
            tmp_solution = local_search(tsp, tmp_solution, InTourMoveType::SwapEdges, rng);
        }
        int tmp_score = tsp.evaluate(tmp_solution);
        long long elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
        if (elapsed_time >= time_limit) {
            return {bestTour, best_score, elapsed_time, perturbation_count};
        }
        perturbation_count++; 
        if (tmp_score > best_score) {
            best_score = tmp_score;
            bestTour = tmp_solution;
            tour = tmp_solution;
        }
        
    }
}

std::vector<int> destroy(const std::vector<int>& tour, const TSPInstance& tsp, 
                         float rate, RemovalMode mode, std::mt19937& rng) {
    int n = tour.size();
    int to_remove = static_cast<int>(std::round(static_cast<float>(n) * rate));
    if (n <= 4 || to_remove <= 0) return tour;
    if (to_remove >= n - 2) to_remove = n - 3;

    std::vector<bool> is_removed(n, false);
    
    if (mode == RemovalMode::WorstVertex) {
        // ... logika z std::nth_element dla kosztu wierzchołków ...
        struct Item { int idx; int cost; };
        std::vector<Item> items(n);
        for (int i = 0; i < n; ++i) {
            int prev = tour[(i - 1 + n) % n];
            int next = tour[(i + 1) % n];
            items[i] = {i, tsp.dist[prev][tour[i]] + tsp.dist[tour[i]][next]};
        }
        std::nth_element(items.begin(), items.begin() + n - to_remove, items.end(),
                         [](const Item& a, const Item& b) { return a.cost < b.cost; });
        for (int i = n - to_remove; i < n; ++i) is_removed[items[i].idx] = true;
    } 
    else if (mode == RemovalMode::WorstEdge) {
        // ... logika usuwania końców najdłuższych krawędzi ...
        struct Edge { int u_idx, v_idx, len; };
        std::vector<Edge> edges(n);
        for (int i = 0; i < n; ++i) {
            edges[i] = {i, (i + 1) % n, tsp.dist[tour[i]][tour[(i + 1) % n]]};
        }
        std::nth_element(edges.begin(), std::max(edges.begin(), edges.end() - to_remove), edges.end(),
                         [](const Edge& a, const Edge& b) { return a.len < b.len; });
        int removed = 0;
        for (int i = n - 1; i >= 0 && removed < to_remove; --i) {
            if (!is_removed[edges[i].u_idx]) { is_removed[edges[i].u_idx] = true; removed++; }
            if (removed >= to_remove) break;
            if (!is_removed[edges[i].v_idx]) { is_removed[edges[i].v_idx] = true; removed++; }
        }
    } 
    else if (mode == RemovalMode::Random) {
        // Proste losowanie indeksów
        std::vector<int> indices(n);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);
        for(int i = 0; i < to_remove; ++i) is_removed[indices[i]] = true;
    }
    else if (mode == RemovalMode::RandomSubpath) {
        // Losujemy jeden spójny podciąg na cyklu i usuwamy go w całości.
        std::uniform_int_distribution<int> start_dist(0, n - 1);
        int start = start_dist(rng);
        for (int i = 0; i < to_remove; ++i) {
            is_removed[(start + i) % n] = true;
        }
    }

    // Budowanie nowej trasy
    std::vector<int> partial_tour;
    for (int i = 0; i < n; ++i) {
        if (!is_removed[i]) partial_tour.push_back(tour[i]);
    }
    return partial_tour;
}

std::vector<int> repair(const std::vector<int>& partial_tour, const TSPInstance& tsp, std::mt19937& rng) {
    (void)rng;

    std::vector<int> tour = partial_tour;
    std::vector<bool> inTour(tsp.size(), false);
    for (int v : tour) {
        inTour[v] = true;
    }

    while (static_cast<int>(tour.size()) < tsp.size()) {
        int bestVertex = -1;
        int bestPosition = -1;
        int bestRegret = INT_MIN;
        int bestDelta1 = INT_MIN;

        for (int v = 0; v < tsp.size(); ++v) {
            if (inTour[v]) continue;

            int delta1 = INT_MIN;
            int delta2 = INT_MIN;
            int position1 = -1;

            if (tour.empty()) {
                delta1 = tsp.nodes[v].profit;
                position1 = 0;
            } else {
                int k = static_cast<int>(tour.size());
                for (int i = 0; i < k; ++i) {
                    int delta = insertionDelta(tour[i], v, tour[(i + 1) % k], tsp, true);
                    if (delta > delta1) {
                        delta2 = delta1;
                        delta1 = delta;
                        position1 = i + 1;
                    } else if (delta > delta2) {
                        delta2 = delta;
                    }
                }
            }

            int regret = delta1 - delta2;
            if (regret > bestRegret || (regret == bestRegret && delta1 > bestDelta1)) {
                bestRegret = regret;
                bestDelta1 = delta1;
                bestVertex = v;
                bestPosition = position1;
            }
        }

        if (bestVertex == -1) break;

        tour.insert(tour.begin() + bestPosition, bestVertex);
        inTour[bestVertex] = true;
    }

    return phaseII(tour, tsp);
}
