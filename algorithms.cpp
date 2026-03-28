#include "algorithms.h"
#include <algorithm>
#include <numeric>
#include <climits>
#include <iostream>
#include <chrono>
#include "neighbourhood.h"

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
    int d = tsp.dist[previous][latter] - tsp.dist[previous][removed] - tsp.dist[removed][latter];
    if (useProfit) d -= tsp.nodes[removed].profit;
    return d;
}
static inline int swapVerticesDelta(int prev_1, int to_swap_1, int latter_1,
                                   int prev_2, int to_swap_2, int latter_2,
                                   const TSPInstance& tsp) {
    int d = 0;
    // Usuwamy to_swap_1 i wstawiamy to_swap_2
    d += tsp.dist[prev_1][to_swap_1] + tsp.dist[to_swap_1][latter_1] - tsp.dist[prev_1][to_swap_2] - tsp.dist[to_swap_2][latter_1];
    // Usuwamy to_swap_2 i wstawiamy to_swap_1
    d += tsp.dist[prev_2][to_swap_2] + tsp.dist[to_swap_2][latter_2] - tsp.dist[prev_2][to_swap_1] - tsp.dist[to_swap_1][latter_2];
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

std::vector<int> getBaseSolution(const TSPInstance& tsp, BaseSolutionType type, int runs = 1) {
    AlgoStats stats;
    switch(type) {
        case BaseSolutionType::best:
            stats = collectStats(regretGC, tsp, false, runs);
            break;
        case BaseSolutionType::random: {
            std::mt19937 rng(69);
            stats = collectRandomStats(tsp, runs, rng);
        }
    }
    return stats.bestTour;
}
AlgoStatsTimed collectStatsWalk(AlgoFuncWalk algo, std::vector<int> base_solution,const TSPInstance& tsp, InTourMoveType move_type, int n = -1) {
    if (n == -1) n = tsp.size();
    
    Stats score_stats{ 0.0, INT_MAX, INT_MIN };
    TimeStats time_stats{ 0.0, INT_MAX, INT_MIN };
    std::vector<int> bestTour;
    int bestScore = INT_MIN;
    for (int s = 0; s < n; ++s) {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto tour    = algo(s, tsp, base_solution, move_type);
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
    return { time_stats, score_stats, bestTour };
}

AlgoStats collectStats(AlgoFunc algo, const TSPInstance& tsp, bool useProfit, int n = -1) {
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
        auto start_time = std::chrono::high_resolution_clock::now();
        auto tour   = randomWalk(i, tsp, base_solution, move_type, one_run_time_limit);
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
    return { time_stats, score_stats, bestTour };

}
int transformation_delta(const Neighbour& neighbor,std::vector<int>& tour, const TSPInstance& tsp) {
    int delta = 0;
    int k= static_cast<int>(tour.size());
    switch(neighbor.type) {
            case MoveType::Add:
                delta = insertionDelta(tour[(neighbor.second-1+k)% k],tour[neighbor.first],tour[neighbor.second], tsp, true);
                break;
            case MoveType::remove:
                delta = removalDelta(tour[(neighbor.first-1+k)% k],tour[neighbor.first],tour[(neighbor.first + 1+k)% k], tsp, true);
                break;
            case MoveType::swap_vertices:
                delta = swapVerticesDelta(tour[(neighbor.first-1+k)% k], tour[neighbor.first], tour[(neighbor.first + 1+k)% k],tour[(neighbor.second-1+k)% k],tour[neighbor.second],tour[(neighbor.second + 1+k)% k], tsp);
                break;
            case MoveType::swap_edges:
                delta = swapEdgesDelta(tour[neighbor.first], tour[(neighbor.first + 1+k)% k], tour[neighbor.second], tour[(neighbor.second + 1+k)% k], tsp);
                break;
            }
    return delta;
}
std::vector<int> randomWalk(int start, const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type, double time_limit) {
    std::vector<int> tour = base_solution;
    std::vector<int> bestTour = tour;
    int bestScore= tsp.evaluate(tour);
    int score = bestScore;
    std::mt19937 rng(69);
    int delta = 0;
    double elapsed_time = 0.0;
    auto walk_start_time = std::chrono::high_resolution_clock::now();

    while(elapsed_time < time_limit) {

        std::vector<Neighbour> neighbors = generateNeighbourhood(tour, tsp, move_type);
        std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size()) - 1);

        int choosen_idx = dist(rng);

        if (neighbors.empty()) throw std::runtime_error("No neighbor generated in randomWalk at idx " + std::to_string(choosen_idx));

        Neighbour neighbor = neighbors[choosen_idx];
        delta = transformation_delta(neighbor, tour, tsp);
        neighbor.transform(tour, tsp);
        score += delta;
        if( score > bestScore) {
            bestTour = tour;
            bestScore = score;
        }
        auto now = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(now - walk_start_time).count();
    }
    return bestTour;
}
std::vector<int> steepestWalk(int start, const TSPInstance& tsp, std::vector<int> base_solution, InTourMoveType move_type){
    std::vector<int> tour = base_solution;
    int bestScore = tsp.evaluate(tour);
    bool improved = true;
    int score = bestScore;
    Neighbour bestNeighbor = Neighbour(MoveType::Add, -1, -1);
    while(improved) {

        improved = false;
        std::vector<Neighbour> neighbors = generateNeighbourhood(tour, tsp, move_type);
        for(const Neighbour& neighbor : neighbors) {
            int delta = transformation_delta(neighbor, tour, tsp);
            if(delta > 0 && delta + score > bestScore) {
                bestNeighbor = neighbor;
                bestScore = delta + score;
                improved = true;
            }
        }
        if(improved) {
            bestNeighbor.transform(tour, tsp);
            score = bestScore;
            bestNeighbor = Neighbour(MoveType::Add, -1, -1);
        }
    }
    return tour;
}

