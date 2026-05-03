#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include "tsp.h"
#include "algorithms.h"
#include <filesystem>
#include <cmath>
bool useA = true;
std::vector<AlgoStats> phaseIA_length;
std::vector<AlgoStats> phaseIB_length;
std::vector<std::string> labels = {
            "NNa_len", "NNb_len",
            "GCa_len", "GCb_len",
            "RegretGCa_len", "RegretGCb_len",
            "WRegretGCa_len", "WRegretGCb_len"
        };
static void printStats(const Stats& s) {
    std::cout << std::setw(8) << static_cast<long long>(std::llround(s.avg))
              << " (" << std::setw(7) << s.min
              << " - " << std::setw(7) << s.max << ")";
}

static void printStatsTable(
    const std::vector<std::string>& labels,
    const std::vector<AlgoStats>&   statsA,
    const std::vector<AlgoStats>&   statsB,
    bool usePhaseI)
{
    const int W = 12;
    std::cout << std::left << std::setw(W) << "Metoda"
              << "  " << std::setw(28) << "TSPA"
              << "  " << std::setw(28) << "TSPB" << "\n";
    std::cout << std::string(74, '-') << "\n";

    for (int i = 0; i < (int)labels.size(); ++i) {
        const Stats& sA = usePhaseI ? statsA[i].phaseI  : statsA[i].phaseII;
        const Stats& sB = usePhaseI ? statsB[i].phaseI  : statsB[i].phaseII;
        std::cout << std::left << std::setw(W) << labels[i] << std::right << "  ";
        // if(i==(int)labels.size()-1) {
        
        // std::cout<< "WRGCbB: min"<<statsB[i].phaseI.min<<" max "<<statsB[i].phaseI.max<<" avg "<<statsB[i].phaseI.avg<<"\n";
        // }
        printStats(sA);
        std::cout << "  ";
        printStats(sB);
        std::cout << "\n";
    }
    std::cout << "\n";
}


static void printInstanceSummary(const TSPInstance& tsp) {
    std::cout << "=== " << tsp.name << " ===\n";
    std::cout << "Liczba wierzcholkow: " << tsp.size() << "\n";
    std::cout << "Przykladowe wierzcholki (x, y, profit):\n";
    for (int i = 0; i < std::min(3, tsp.size()); ++i)
        std::cout << "  [" << i << "] " << tsp.nodes[i].x << " " << tsp.nodes[i].y
                  << " profit=" << tsp.nodes[i].profit << "\n";
    std::cout << "Przykladowe odleglosci:\n";
    int n = std::min(4, tsp.size());
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            std::cout << "  dist[" << i << "][" << j << "] = " << tsp.dist[i][j] << "\n";
    std::cout << "\n";
}

void updateTimeStats(AlgoStatsTimed& out_stats, AlgoStatsTimed new_stats,int runs) {
    if(out_stats.score_stats.max< new_stats.score_stats.max) {
        out_stats.bestTour = new_stats.bestTour;
    }
    out_stats.time_stats.avg += new_stats.time_stats.avg/runs;
    out_stats.time_stats.min = std::min(out_stats.time_stats.min, new_stats.time_stats.min);
    out_stats.time_stats.max = std::max(out_stats.time_stats.max, new_stats.time_stats.max);

    out_stats.score_stats.avg += new_stats.score_stats.avg/runs;
    out_stats.score_stats.min = std::min(out_stats.score_stats.min, new_stats.score_stats.min);
    out_stats.score_stats.max = std::max(out_stats.score_stats.max, new_stats.score_stats.max);


}
void saveTourToFile(const std::string& filename, const std::vector<int>& tour) {
    // 1. Definiujesz nazwę folderu
    std::string folder = "wyniki_tsp"; 

    // 2. Tworzysz folder (nic się nie stanie, jeśli już istnieje)
    std::filesystem::create_directories(folder);

    // 3. Łączysz folder z nazwą pliku za pomocą "/"
    std::ofstream f(folder + "/" + filename);

    if (f.is_open()) {
        for (int node : tour) {
            f << node << "\n";
        }
        f.close();
    } else {
        std::cerr << "Blad zapisu do: " << folder << "/" << filename << "\n";
    }
}
int main() {
    std::mt19937 rng(69);
    int runs=100;
    TSPInstance tspA("TSPA.csv");
    TSPInstance tspB("TSPB.csv");
    std::pair<AlgoStats, std::vector<std::vector<int>>> baseStatsSolutions = getBaseSolutions(regretGC, tspA, BaseSolutionType::best, runs);
    std::pair<AlgoStats, std::vector<std::vector<int>>> baseStatsSolutionsB = getBaseSolutions(regretGC, tspB, BaseSolutionType::best, runs);
    std::pair<AlgoStats, std::vector<std::vector<int>>> randomBaseStatsSolutionsA = getBaseSolutionsRandom(tspA, runs, rng);
    std::pair<AlgoStats, std::vector<std::vector<int>>> randomBaseStatsSolutionsB = getBaseSolutionsRandom(tspB, runs, rng);
    std::vector<std::vector<int>> bestBaseSolutionsA = baseStatsSolutions.second;
    std::vector<std::vector<int>> bestBaseSolutionsB = baseStatsSolutionsB.second;
    std::vector<std::vector<int>> randomBaseSolutionsA = randomBaseStatsSolutionsA.second;
    std::vector<std::vector<int>> randomBaseSolutionsB = randomBaseStatsSolutionsB.second;
    AlgoStats baseStatsA = baseStatsSolutions.first;
    AlgoStats baseStatsB = baseStatsSolutionsB.first;
    AlgoStats randomBaseStatsA = randomBaseStatsSolutionsA.first;
    AlgoStats randomBaseStatsB = randomBaseStatsSolutionsB.first;

    AlgoStatsTimed lm_walk_a_random_edges;
    AlgoStatsTimed lm_walk_b_random_edges;
    AlgoStatsTimed candidate_walk_a_random_edges;
    AlgoStatsTimed candidate_walk_b_random_edges;


    AlgoStatsTimed MLSL_a = MLSL(
        tspA, rng, 20, 200, steepestWalkLM);
    AlgoStatsTimed MLSL_b = MLSL(
        tspB, rng, 20, 200, steepestWalkLM);
    // std::cerr<<"a: "<< MLSL_a.time_stats.avg<<" b: "<< MLSL_b.time_stats.avg<<"\n";
    AlgoStatsTimed ILS_a = ILS(
        tspA, rng, 20, MLSL_a.time_stats.avg, 5, steepestWalkLM);
    AlgoStatsTimed ILS_b = ILS(
        tspB, rng, 20, MLSL_b.time_stats.avg, 5, steepestWalkLM);

    //worstedges
    float destruction_rate=0.3;
    AlgoStatsTimed LNS_a_edges_local = LNS(
        tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::WorstEdge, true, steepestWalkLM);
    AlgoStatsTimed LNS_b_edges_local = LNS(
        tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::WorstEdge, true, steepestWalkLM);
    // AlgoStatsTimed LNS_a_edges_no_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::WorstEdge, false, steepestWalkLM);
    // AlgoStatsTimed LNS_b_edges_no_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::WorstEdge, false, steepestWalkLM);

    //worst vertices
    AlgoStatsTimed LNS_a_vertices_local = LNS(
        tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::WorstVertex, true, steepestWalkLM);
    AlgoStatsTimed LNS_b_vertices_local = LNS(
        tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::WorstVertex, true, steepestWalkLM);
    // AlgoStatsTimed LNS_a_vertices_no_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::WorstVertex, false, steepestWalkLM);
    // AlgoStatsTimed LNS_b_vertices_no_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::WorstVertex, false, steepestWalkLM);

    // subpaths
    AlgoStatsTimed LNS_a_subpaths_local = LNS(
        tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::RandomSubpath, true, steepestWalkLM);
    AlgoStatsTimed LNS_b_subpaths_local = LNS(
        tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::RandomSubpath, true, steepestWalkLM);

    //random
    // AlgoStatsTimed LNS_a_random_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::Random, true, steepestWalkLM);
    // AlgoStatsTimed LNS_b_random_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::Random, true, steepestWalkLM);
    // AlgoStatsTimed LNS_a_random_no_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::Random, false, steepestWalkLM);
    // AlgoStatsTimed LNS_b_random_no_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::Random, false, steepestWalkLM);

    //all random
    // AlgoStatsTimed LNS_a_all_random_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::AllRandom, true, steepestWalkLM);
    // AlgoStatsTimed LNS_b_all_random_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::AllRandom, true, steepestWalkLM);
    // AlgoStatsTimed LNS_a_all_random_no_local = LNS(
    //     tspA, rng, 20, MLSL_a.time_stats.avg, destruction_rate, RemovalMode::AllRandom, false, steepestWalkLM);
    // AlgoStatsTimed LNS_b_all_random_no_local = LNS(
    //     tspB, rng, 20, MLSL_b.time_stats.avg, destruction_rate, RemovalMode::AllRandom, false, steepestWalkLM);

    for (int i = 0; i < runs; ++i) {
        std::cerr << "Task 3 run " << i + 1 << "/" << runs << "\n";
        updateTimeStats(lm_walk_a_random_edges, collectStatsWalk(steepestWalkLM, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(lm_walk_b_random_edges, collectStatsWalk(steepestWalkLM, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(candidate_walk_a_random_edges, collectStatsWalk(steepestWalkCandidate, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(candidate_walk_b_random_edges, collectStatsWalk(steepestWalkCandidate, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1), runs);
    }

    

    std::ofstream task3("wyniki3.csv");
    if (task3.is_open()) {
        task3 << "algorytm;instancja;rozwiazanie bazowe;typ ruchu wewnatrztrasowego;sredni czas;max czas;min czas;sredni wynik;max wynik;min wynik;srednia iteracji; min iteracji; max iteracji; najlepsze iteracje; \n";
        auto writeTask3Row = [&](const std::string& name, const std::string& instance,
                                 const std::string& base, const std::string& move_type,
                                 const AlgoStatsTimed& stats) {
            auto roundedScoreAvg = static_cast<long long>(std::llround(stats.score_stats.avg));
            task3 << name << ";" << instance << ";" << base << ";" << move_type << ";"
                  << stats.time_stats.avg << ";" << stats.time_stats.max << ";" << stats.time_stats.min << ";"
                << roundedScoreAvg << ";" << stats.score_stats.max << ";" << stats.score_stats.min << ";"
                  << stats.perturbation_stats.avg << ";" << stats.perturbation_stats.min << ";" << stats.perturbation_stats.max << ";" << stats.perturbation_stats.best << "\n";
        };
        writeTask3Row("lm_walk",       "A", "random", "swap_edges", lm_walk_a_random_edges);
        writeTask3Row("lm_walk",       "B", "random", "swap_edges", lm_walk_b_random_edges);
        writeTask3Row("candidate_walk","A", "random", "swap_edges", candidate_walk_a_random_edges);
        writeTask3Row("candidate_walk","B", "random", "swap_edges", candidate_walk_b_random_edges);
        writeTask3Row("MLSL",          "A", "random", "swap_edges", MLSL_a);
        writeTask3Row("MLSL",          "B", "random", "swap_edges", MLSL_b);
        writeTask3Row("ILS",           "A", "random", "swap_edges", ILS_a);
        writeTask3Row("ILS",           "B", "random", "swap_edges", ILS_b);
        writeTask3Row("LNS_worst_edges_local", "A", "random", "swap_edges", LNS_a_edges_local);
        writeTask3Row("LNS_worst_edges_local", "B", "random", "swap_edges", LNS_b_edges_local);
        writeTask3Row("LNS_worst_vertices_local", "A", "random", "swap_edges", LNS_a_vertices_local);
        writeTask3Row("LNS_worst_vertices_local", "B", "random", "swap_edges", LNS_b_vertices_local);
        writeTask3Row("LNS_subpaths_local", "A", "random", "swap_edges", LNS_a_subpaths_local);
        writeTask3Row("LNS_subpaths_local", "B", "random", "swap_edges", LNS_b_subpaths_local);
        // writeTask3Row("LNS_random_local", "A", "random", "swap_edges", LNS_a_random_local);
        // writeTask3Row("LNS_random_local", "B", "random", "swap_edges", LNS_b_random_local);
        // writeTask3Row("LNS_all_random_local", "A", "random", "swap_edges", LNS_a_all_random_local);
        // writeTask3Row("LNS_all_random_local", "B", "random", "swap_edges", LNS_b_all_random_local);
    } else {
        std::cerr << "Blad zapisu do: wyniki3.csv\n";
    }

    saveTourToFile("lm_walk_a_random_edges.txt", lm_walk_a_random_edges.bestTour);
    saveTourToFile("lm_walk_b_random_edges.txt", lm_walk_b_random_edges.bestTour);
    saveTourToFile("candidate_walk_a_random_edges.txt", candidate_walk_a_random_edges.bestTour);
    saveTourToFile("candidate_walk_b_random_edges.txt", candidate_walk_b_random_edges.bestTour);
    saveTourToFile("MLSL_a.txt", MLSL_a.bestTour);
    saveTourToFile("MLSL_b.txt", MLSL_b.bestTour);

    // Demo przed/po: jeden wspólny start losowy dla wizualizacji
    std::mt19937 demo_rng(42);
    std::vector<int> demo_start_a = randomSolution(tspA.size(), demo_rng);
    std::mt19937 demo_rng2(42);
    std::vector<int> demo_start_b = randomSolution(tspB.size(), demo_rng2);
    saveTourToFile("demo_before_a.txt", demo_start_a);
    saveTourToFile("demo_before_b.txt", demo_start_b);
    saveTourToFile("demo_steep_after_a.txt",     steepestWalk(tspA,          demo_start_a, InTourMoveType::SwapEdges, demo_rng));
    saveTourToFile("demo_steep_after_b.txt",     steepestWalk(tspB,          demo_start_b, InTourMoveType::SwapEdges, demo_rng2));
    saveTourToFile("demo_lm_after_a.txt",        steepestWalkLM(tspA,        demo_start_a, InTourMoveType::SwapEdges, demo_rng));
    saveTourToFile("demo_lm_after_b.txt",        steepestWalkLM(tspB,        demo_start_b, InTourMoveType::SwapEdges, demo_rng2));
    saveTourToFile("demo_cand_after_a.txt",      steepestWalkCandidate(tspA, demo_start_a, InTourMoveType::SwapEdges, demo_rng));
    saveTourToFile("demo_cand_after_b.txt",      steepestWalkCandidate(tspB, demo_start_b, InTourMoveType::SwapEdges, demo_rng2));
    std::cerr << "Demo przed/po zapisane.\n";
    
    
    return 0;
}
