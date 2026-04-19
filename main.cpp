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
    std::cout << std::setw(8) << static_cast<int>(s.avg)
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

    AlgoStatsTimed greedy_walk_a_best_edges ;
    AlgoStatsTimed greedy_walk_b_best_edges ;
    AlgoStatsTimed greedy_walk_a_random_edges ;
    AlgoStatsTimed greedy_walk_b_random_edges ;

    AlgoStatsTimed greedy_walk_a_best_vertices ;
    AlgoStatsTimed greedy_walk_b_best_vertices ;
    AlgoStatsTimed greedy_walk_a_random_vertices ;
    AlgoStatsTimed greedy_walk_b_random_vertices ;

    AlgoStatsTimed steep_walk_a_best_edges ;
    AlgoStatsTimed steep_walk_b_best_edges ;
    AlgoStatsTimed steep_walk_a_random_edges ;
    AlgoStatsTimed steep_walk_b_random_edges ;

    AlgoStatsTimed steep_walk_a_best_vertices ;
    AlgoStatsTimed steep_walk_b_best_vertices ;
    AlgoStatsTimed steep_walk_a_random_vertices ;
    AlgoStatsTimed steep_walk_b_random_vertices ;

    AlgoStatsTimed random_walk_a_best_edges ;
    AlgoStatsTimed random_walk_b_best_edges ;
    AlgoStatsTimed random_walk_a_random_edges ;
    AlgoStatsTimed random_walk_b_random_edges ;

    AlgoStatsTimed random_walk_a_best_vertices ;
    AlgoStatsTimed random_walk_b_best_vertices ;
    AlgoStatsTimed random_walk_a_random_vertices ;
    AlgoStatsTimed random_walk_b_random_vertices ;
    for (int i = 0 ; i < runs; i++){
        std::cerr<<"Local search run "<<i+1<<"/"<<runs<<"\n";
        updateTimeStats(greedy_walk_a_best_edges, collectStatsWalk(greedyWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(greedy_walk_b_best_edges, collectStatsWalk(greedyWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(greedy_walk_a_random_edges, collectStatsWalk(greedyWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(greedy_walk_b_random_edges, collectStatsWalk(greedyWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1),runs);

        updateTimeStats(greedy_walk_a_best_vertices, collectStatsWalk(greedyWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(greedy_walk_b_best_vertices, collectStatsWalk(greedyWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(greedy_walk_a_random_vertices, collectStatsWalk(greedyWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(greedy_walk_b_random_vertices, collectStatsWalk(greedyWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1),runs);
        std::cerr<<"greedy walks done\n";
        updateTimeStats(steep_walk_a_best_edges, collectStatsWalk(steepestWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(steep_walk_b_best_edges, collectStatsWalk(steepestWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(steep_walk_a_random_edges, collectStatsWalk(steepestWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1),runs);
        updateTimeStats(steep_walk_b_random_edges, collectStatsWalk(steepestWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1),runs);

        updateTimeStats(steep_walk_a_best_vertices, collectStatsWalk(steepestWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(steep_walk_b_best_vertices, collectStatsWalk(steepestWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(steep_walk_a_random_vertices, collectStatsWalk(steepestWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1),runs);
        updateTimeStats(steep_walk_b_random_vertices, collectStatsWalk(steepestWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1),runs);
        std::cerr<<"steepest walks done\n";
    }

    auto maxLocalSearchTime = [](const std::vector<double>& times) {
        return *std::max_element(times.begin(), times.end());
    };
    double randomWalkLimitA = maxLocalSearchTime({
        greedy_walk_a_best_edges.time_stats.avg,
        greedy_walk_a_random_edges.time_stats.avg,
        greedy_walk_a_best_vertices.time_stats.avg,
        greedy_walk_a_random_vertices.time_stats.avg,
        steep_walk_a_best_edges.time_stats.avg,
        steep_walk_a_random_edges.time_stats.avg,
        steep_walk_a_best_vertices.time_stats.avg,
        steep_walk_a_random_vertices.time_stats.avg
    });
    double randomWalkLimitB = maxLocalSearchTime({
        greedy_walk_b_best_edges.time_stats.avg,
        greedy_walk_b_random_edges.time_stats.avg,
        greedy_walk_b_best_vertices.time_stats.avg,
        greedy_walk_b_random_vertices.time_stats.avg,
        steep_walk_b_best_edges.time_stats.avg,
        steep_walk_b_random_edges.time_stats.avg,
        steep_walk_b_best_vertices.time_stats.avg,
        steep_walk_b_random_vertices.time_stats.avg
    });

    for (int i = 0 ; i < runs; i++){
        std::cerr<<"Random walk run "<<i+1<<"/"<<runs<<"\n";
        updateTimeStats(random_walk_a_best_edges, collectRandomWalkStats(tspA,bestBaseSolutionsA[i], InTourMoveType::SwapEdges, 1,rng,randomWalkLimitA),runs);
        updateTimeStats(random_walk_b_best_edges, collectRandomWalkStats(tspB,bestBaseSolutionsB[i], InTourMoveType::SwapEdges, 1,rng,randomWalkLimitB),runs);
        updateTimeStats(random_walk_a_random_edges, collectRandomWalkStats(tspA,randomBaseSolutionsA[i], InTourMoveType::SwapEdges, 1,rng,randomWalkLimitA),runs);
        updateTimeStats(random_walk_b_random_edges, collectRandomWalkStats(tspB,randomBaseSolutionsB[i], InTourMoveType::SwapEdges, 1,rng,randomWalkLimitB),runs);

        updateTimeStats(random_walk_a_best_vertices, collectRandomWalkStats(tspA,bestBaseSolutionsA[i], InTourMoveType::SwapVertices, 1,rng,randomWalkLimitA),runs);
        updateTimeStats(random_walk_b_best_vertices, collectRandomWalkStats(tspB,bestBaseSolutionsB[i], InTourMoveType::SwapVertices, 1,rng,randomWalkLimitB),runs);
        updateTimeStats(random_walk_a_random_vertices, collectRandomWalkStats(tspA,randomBaseSolutionsA[i], InTourMoveType::SwapVertices, 1,rng,randomWalkLimitA),runs);
        updateTimeStats(random_walk_b_random_vertices, collectRandomWalkStats(tspB,randomBaseSolutionsB[i], InTourMoveType::SwapVertices, 1,rng,randomWalkLimitB),runs);
    }
    
    //print walk times 
    if(true){
    std::cout<<"algorytm;instancja;rozwiazanie bazowe;typ ruchu wewnatrztrasowego;sredni czas;max czas;min czas\n";
    std::cout<<"greedy_walk;A;regretGC;swap_edges;"<<greedy_walk_a_best_edges.time_stats.avg<<";"<<greedy_walk_a_best_edges.time_stats.max<<";"<<greedy_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_edges;"<<greedy_walk_b_best_edges.time_stats.avg<<";"<<greedy_walk_b_best_edges.time_stats.max<<";"<<greedy_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_edges;"<<greedy_walk_a_random_edges.time_stats.avg<<";"<<greedy_walk_a_random_edges.time_stats.max<<";"<<greedy_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_edges;"<<greedy_walk_b_random_edges.time_stats.avg<<";"<<greedy_walk_b_random_edges.time_stats.max<<";"<<greedy_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"greedy_walk;A;regretGC;swap_vertices;"<<greedy_walk_a_best_vertices.time_stats.avg<<";"<<greedy_walk_a_best_vertices.time_stats.max<<";"<<greedy_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_vertices;"<<greedy_walk_b_best_vertices.time_stats.avg<<";"<<greedy_walk_b_best_vertices.time_stats.max<<";"<<greedy_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_vertices;"<<greedy_walk_a_random_vertices.time_stats.avg<<";"<<greedy_walk_a_random_vertices.time_stats.max<<";"<<greedy_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_vertices;"<<greedy_walk_b_random_vertices.time_stats.avg<<";"<<greedy_walk_b_random_vertices.time_stats.max<<";"<<greedy_walk_b_random_vertices.time_stats.min<<"\n";
    
    std::cout<<"steep_walk;A;regretGC;swap_edges;"<<steep_walk_a_best_edges.time_stats.avg<<";"<<steep_walk_a_best_edges.time_stats.max<<";"<<steep_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_edges;"<<steep_walk_b_best_edges.time_stats.avg<<";"<<steep_walk_b_best_edges.time_stats.max<<";"<<steep_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_edges;"<<steep_walk_a_random_edges.time_stats.avg<<";"<<steep_walk_a_random_edges.time_stats.max<<";"<<steep_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_edges;"<<steep_walk_b_random_edges.time_stats.avg<<";"<<steep_walk_b_random_edges.time_stats.max<<";"<<steep_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"steep_walk;A;regretGC;swap_vertices;"<<steep_walk_a_best_vertices.time_stats.avg<<";"<<steep_walk_a_best_vertices.time_stats.max<<";"<<steep_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_vertices;"<<steep_walk_b_best_vertices.time_stats.avg<<";"<<steep_walk_b_best_vertices.time_stats.max<<";"<<steep_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_vertices;"<<steep_walk_a_random_vertices.time_stats.avg<<";"<<steep_walk_a_random_vertices.time_stats.max<<";"<<steep_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_vertices;"<<steep_walk_b_random_vertices.time_stats.avg<<";"<<steep_walk_b_random_vertices.time_stats.max<<";"<<steep_walk_b_random_vertices.time_stats.min<<"\n";  

    std::cout<<"random_walk;A;regretGC;swap_edges;"<<random_walk_a_best_edges.time_stats.avg<<";"<<random_walk_a_best_edges.time_stats.max<<";"<<random_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_edges;"<<random_walk_b_best_edges.time_stats.avg<<";"<<random_walk_b_best_edges.time_stats.max<<";"<<random_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_edges;"<<random_walk_a_random_edges.time_stats.avg<<";"<<random_walk_a_random_edges.time_stats.max<<";"<<random_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_edges;"<<random_walk_b_random_edges.time_stats.avg<<";"<<random_walk_b_random_edges.time_stats.max<<";"<<random_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"random_walk;A;regretGC;swap_vertices;"<<random_walk_a_best_vertices.time_stats.avg<<";"<<random_walk_a_best_vertices.time_stats.max<<";"<<random_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_vertices;"<<random_walk_b_best_vertices.time_stats.avg<<";"<<random_walk_b_best_vertices.time_stats.max<<";"<<random_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_vertices;"<<random_walk_a_random_vertices.time_stats.avg<<";"<<random_walk_a_random_vertices.time_stats.max<<";"<<random_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_vertices;"<<random_walk_b_random_vertices.time_stats.avg<<";"<<random_walk_b_random_vertices.time_stats.max<<";"<<random_walk_b_random_vertices.time_stats.min<<"\n";
    }
    //print walk scores
    if(true){
    std::cout<<"\n\n\nalgorytm;instancja;rozwiazanie bazowe;typ ruchu wewnatrztrasowego;sredni wynik;max wynik;min wynik\n";
    std::cout<<"greedy_walk;A;regretGC;swap_edges;"<<greedy_walk_a_best_edges.score_stats.avg<<";"<<greedy_walk_a_best_edges.score_stats.max<<";"<<greedy_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_edges;"<<greedy_walk_b_best_edges.score_stats.avg<<";"<<greedy_walk_b_best_edges.score_stats.max<<";"<<greedy_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_edges;"<<greedy_walk_a_random_edges.score_stats.avg<<";"<<greedy_walk_a_random_edges.score_stats.max<<";"<<greedy_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_edges;"<<greedy_walk_b_random_edges.score_stats.avg<<";"<<greedy_walk_b_random_edges.score_stats.max<<";"<<greedy_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"greedy_walk;A;regretGC;swap_vertices;"<<greedy_walk_a_best_vertices.score_stats.avg<<";"<<greedy_walk_a_best_vertices.score_stats.max<<";"<<greedy_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_vertices;"<<greedy_walk_b_best_vertices.score_stats.avg<<";"<<greedy_walk_b_best_vertices.score_stats.max<<";"<<greedy_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_vertices;"<<greedy_walk_a_random_vertices.score_stats.avg<<";"<<greedy_walk_a_random_vertices.score_stats.max<<";"<<greedy_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_vertices;"<<greedy_walk_b_random_vertices.score_stats.avg<<";"<<greedy_walk_b_random_vertices.score_stats.max<<";"<<greedy_walk_b_random_vertices.score_stats.min<<"\n";
    
    std::cout<<"steep_walk;A;regretGC;swap_edges;"<<steep_walk_a_best_edges.score_stats.avg<<";"<<steep_walk_a_best_edges.score_stats.max<<";"<<steep_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_edges;"<<steep_walk_b_best_edges.score_stats.avg<<";"<<steep_walk_b_best_edges.score_stats.max<<";"<<steep_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_edges;"<<steep_walk_a_random_edges.score_stats.avg<<";"<<steep_walk_a_random_edges.score_stats.max<<";"<<steep_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_edges;"<<steep_walk_b_random_edges.score_stats.avg<<";"<<steep_walk_b_random_edges.score_stats.max<<";"<<steep_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"steep_walk;A;regretGC;swap_vertices;"<<steep_walk_a_best_vertices.score_stats.avg<<";"<<steep_walk_a_best_vertices.score_stats.max<<";"<<steep_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_vertices;"<<steep_walk_b_best_vertices.score_stats.avg<<";"<<steep_walk_b_best_vertices.score_stats.max<<";"<<steep_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_vertices;"<<steep_walk_a_random_vertices.score_stats.avg<<";"<<steep_walk_a_random_vertices.score_stats.max<<";"<<steep_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_vertices;"<<steep_walk_b_random_vertices.score_stats.avg<<";"<<steep_walk_b_random_vertices.score_stats.max<<";"<<steep_walk_b_random_vertices.score_stats.min<<"\n";  

    std::cout<<"random_walk;A;regretGC;swap_edges;"<<random_walk_a_best_edges.score_stats.avg<<";"<<random_walk_a_best_edges.score_stats.max<<";"<<random_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_edges;"<<random_walk_b_best_edges.score_stats.avg<<";"<<random_walk_b_best_edges.score_stats.max<<";"<<random_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_edges;"<<random_walk_a_random_edges.score_stats.avg<<";"<<random_walk_a_random_edges.score_stats.max<<";"<<random_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_edges;"<<random_walk_b_random_edges.score_stats.avg<<";"<<random_walk_b_random_edges.score_stats.max<<";"<<random_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"random_walk;A;regretGC;swap_vertices;"<<random_walk_a_best_vertices.score_stats.avg<<";"<<random_walk_a_best_vertices.score_stats.max<<";"<<random_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_vertices;"<<random_walk_b_best_vertices.score_stats.avg<<";"<<random_walk_b_best_vertices.score_stats.max<<";"<<random_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_vertices;"<<random_walk_a_random_vertices.score_stats.avg<<";"<<random_walk_a_random_vertices.score_stats.max<<";"<<random_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_vertices;"<<random_walk_b_random_vertices.score_stats.avg<<";"<<random_walk_b_random_vertices.score_stats.max<<";"<<random_walk_b_random_vertices.score_stats.min<<"\n";
    }
    std::cout<<"\n\n\nalgorytm;instancja;sredni wynik;max wynik;min wynik\n";
    std::cout<<"regretGC;A;"<<baseStatsA.phaseII.avg<<";"<<baseStatsA.phaseII.max<<";"<<baseStatsA.phaseII.min<<"\n";
    std::cout<<"regretGC;B;"<<baseStatsB.phaseII.avg<<";"<<baseStatsB.phaseII.max<<";"<<baseStatsB.phaseII.min<<"\n";
    std::cout<<"random;A;"<<randomBaseStatsA.phaseII.avg<<";"<<randomBaseStatsA.phaseII.max<<";"<<randomBaseStatsA.phaseII.min<<"\n";
    std::cout<<"random;B;"<<randomBaseStatsB.phaseII.avg<<";"<<randomBaseStatsB.phaseII.max<<";"<<randomBaseStatsB.phaseII.min<<"\n";

    saveTourToFile("best_greedy_walk_a_best_edges.txt", greedy_walk_a_best_edges.bestTour);
    saveTourToFile("best_greedy_walk_b_best_edges.txt", greedy_walk_b_best_edges.bestTour);
    saveTourToFile("greedy_walk_a_random_edges.txt", greedy_walk_a_random_edges.bestTour);
    saveTourToFile("greedy_walk_b_random_edges.txt", greedy_walk_b_random_edges.bestTour);

    saveTourToFile("greedy_walk_a_best_vertices.txt", greedy_walk_a_best_vertices.bestTour);
    saveTourToFile("greedy_walk_b_best_vertices.txt", greedy_walk_b_best_vertices.bestTour);
    saveTourToFile("greedy_walk_a_random_vertices.txt", greedy_walk_a_random_vertices.bestTour);
    saveTourToFile("greedy_walk_b_random_vertices.txt", greedy_walk_b_random_vertices.bestTour);

    saveTourToFile("steep_walk_a_best_edges.txt", steep_walk_a_best_edges.bestTour);
    saveTourToFile("steep_walk_b_best_edges.txt", steep_walk_b_best_edges.bestTour);
    saveTourToFile("steep_walk_a_random_edges.txt", steep_walk_a_random_edges.bestTour);
    saveTourToFile("steep_walk_b_random_edges.txt", steep_walk_b_random_edges.bestTour);

    saveTourToFile("steep_walk_a_best_vertices.txt", steep_walk_a_best_vertices.bestTour);
    saveTourToFile("steep_walk_b_best_vertices.txt", steep_walk_b_best_vertices.bestTour);
    saveTourToFile("steep_walk_a_random_vertices.txt", steep_walk_a_random_vertices.bestTour);
    saveTourToFile("steep_walk_b_random_vertices.txt", steep_walk_b_random_vertices.bestTour);

    saveTourToFile("random_walk_a_best_edges.txt", random_walk_a_best_edges.bestTour);
    saveTourToFile("random_walk_b_best_edges.txt", random_walk_b_best_edges.bestTour);
    saveTourToFile("random_walk_a_random_edges.txt", random_walk_a_random_edges.bestTour);
    saveTourToFile("random_walk_b_random_edges.txt", random_walk_b_random_edges.bestTour);

    saveTourToFile("random_walk_a_best_vertices.txt", random_walk_a_best_vertices.bestTour);
    saveTourToFile("random_walk_b_best_vertices.txt", random_walk_b_best_vertices.bestTour);
    saveTourToFile("random_walk_a_random_vertices.txt", random_walk_a_random_vertices.bestTour);
    saveTourToFile("random_walk_b_random_vertices.txt", random_walk_b_random_vertices.bestTour);

    AlgoStatsTimed lm_walk_a_random_edges;
    AlgoStatsTimed lm_walk_b_random_edges;
    AlgoStatsTimed candidate_walk_a_random_edges;
    AlgoStatsTimed candidate_walk_b_random_edges;

    for (int i = 0; i < runs; ++i) {
        std::cerr << "Task 3 run " << i + 1 << "/" << runs << "\n";
        updateTimeStats(lm_walk_a_random_edges, collectStatsWalk(steepestWalkLM, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(lm_walk_b_random_edges, collectStatsWalk(steepestWalkLM, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(candidate_walk_a_random_edges, collectStatsWalk(steepestWalkCandidate, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1), runs);
        updateTimeStats(candidate_walk_b_random_edges, collectStatsWalk(steepestWalkCandidate, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1), runs);
    }

    // Referencja: regretGC (najlepszy algorytm z zadania 1) – 100 uruchomień z pomiarem czasu
    AlgoStatsTimed regret_gc_a_timed;
    AlgoStatsTimed regret_gc_b_timed;
    std::vector<int> regret_gc_a_best_tour, regret_gc_b_best_tour;
    for (int s = 0; s < runs; ++s) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto tourA = phaseII(regretGC(s, tspA, true), tspA);
        auto t1 = std::chrono::high_resolution_clock::now();
        long long durA = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        int scoreA = tspA.evaluate(tourA);
        AlgoStatsTimed tmpA; tmpA.time_stats = {(double)durA, durA, durA}; tmpA.score_stats = {(double)scoreA, scoreA, scoreA}; tmpA.bestTour = tourA;
        updateTimeStats(regret_gc_a_timed, tmpA, runs);

        auto t2 = std::chrono::high_resolution_clock::now();
        auto tourB = phaseII(regretGC(s, tspB, true), tspB);
        auto t3 = std::chrono::high_resolution_clock::now();
        long long durB = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
        int scoreB = tspB.evaluate(tourB);
        AlgoStatsTimed tmpB; tmpB.time_stats = {(double)durB, durB, durB}; tmpB.score_stats = {(double)scoreB, scoreB, scoreB}; tmpB.bestTour = tourB;
        updateTimeStats(regret_gc_b_timed, tmpB, runs);
    }
    saveTourToFile("regret_gc_a_best.txt", regret_gc_a_timed.bestTour);
    saveTourToFile("regret_gc_b_best.txt", regret_gc_b_timed.bestTour);

    std::ofstream task3("wyniki3.csv");
    if (task3.is_open()) {
        task3 << "algorytm;instancja;rozwiazanie bazowe;typ ruchu wewnatrztrasowego;sredni czas;max czas;min czas;sredni wynik;max wynik;min wynik\n";
        auto writeTask3Row = [&](const std::string& name, const std::string& instance,
                                 const std::string& base, const std::string& move_type,
                                 const AlgoStatsTimed& stats) {
            task3 << name << ";" << instance << ";" << base << ";" << move_type << ";"
                  << stats.time_stats.avg << ";" << stats.time_stats.max << ";" << stats.time_stats.min << ";"
                  << stats.score_stats.avg << ";" << stats.score_stats.max << ";" << stats.score_stats.min << "\n";
        };
        writeTask3Row("regret_gc",     "A", "deterministic", "none", regret_gc_a_timed);
        writeTask3Row("regret_gc",     "B", "deterministic", "none", regret_gc_b_timed);
        writeTask3Row("steep_walk",    "A", "random", "swap_edges", steep_walk_a_random_edges);
        writeTask3Row("steep_walk",    "B", "random", "swap_edges", steep_walk_b_random_edges);
        writeTask3Row("lm_walk",       "A", "random", "swap_edges", lm_walk_a_random_edges);
        writeTask3Row("lm_walk",       "B", "random", "swap_edges", lm_walk_b_random_edges);
        writeTask3Row("candidate_walk","A", "random", "swap_edges", candidate_walk_a_random_edges);
        writeTask3Row("candidate_walk","B", "random", "swap_edges", candidate_walk_b_random_edges);
    } else {
        std::cerr << "Blad zapisu do: wyniki3.csv\n";
    }

    saveTourToFile("lm_walk_a_random_edges.txt", lm_walk_a_random_edges.bestTour);
    saveTourToFile("lm_walk_b_random_edges.txt", lm_walk_b_random_edges.bestTour);
    saveTourToFile("candidate_walk_a_random_edges.txt", candidate_walk_a_random_edges.bestTour);
    saveTourToFile("candidate_walk_b_random_edges.txt", candidate_walk_b_random_edges.bestTour);

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
    
    
    // try {
    //     TSPInstance tspA("TSPA.csv");
    //     TSPInstance tspB("TSPB.csv");

    //     printInstanceSummary(tspA);
    //     printInstanceSummary(tspB);

    //     auto wRegret = [](int s, const TSPInstance& t, bool u) {
    //         return weightedRegretGC(s, t, u, 1.0, 1.0);
    //     };

    //     std::vector<std::string> labels = {
    //         "NNa", "NNb",
    //         "GCa", "GCb",
    //         "RegretGCa", "RegretGCb",
    //         "WRegretGCa", "WRegretGCb"
    //     };
    //     std::vector<std::pair<AlgoFunc, bool>> algos = {
    //         { greedyNN, false }, { greedyNN, true  },
    //         { greedyGC, false }, { greedyGC, true  },
    //         { regretGC, false }, { regretGC, true  },
    //         { wRegret,  false }, { wRegret, true  }
    //     };

    //     std::cout << "Trwa obliczanie (200 startow x 8 algorytmow x 2 instancje)...\n\n";
    //     std::mt19937 rng(69);
    //     labels.insert(labels.begin(), "Random");
    //     std::vector<AlgoStats> statsA, statsB;
    //     useA=true;
    //     statsA.push_back(collectRandomStats(tspA, 200, rng));
    //     useA=false;
    //     statsB.push_back(collectRandomStats(tspB, 200, rng));
    //     for (auto& [algo, useProfit] : algos) {
    //         useA = true;
    //         statsA.push_back(collectStats(algo, tspA, useProfit));
    //         useA = false;
    //         statsB.push_back(collectStats(algo, tspB, useProfit));
    //     }

    //     //  for (int i = 0; i < (int)phaseIB_length.size(); ++i) {
    //     //     if (phaseIB_length[i].phaseI.min <1 || phaseIB_length[i].phaseI.max <1) {
    //     //         std::cout << "Warning: phase I tour size < 2 for " << labels[i] << " on TSPB\n";
    //     //     }
    //     //     if(i == (int)phaseIB_length.size()-1) {
    //     //         std::cout<< "WRGCbB: min "<<phaseIB_length[i].phaseI.min<<" max "<<phaseIB_length[i].phaseI.max<<" avg "<<phaseIB_length[i].phaseI.avg<<"\n";
    //     //     }
    //     // }
    //     // faza I
    //     std::cout << "Wyniki po I fazie\n";
    //     std::cout << "    srednia (min - max)\n\n";
    //     printStatsTable(labels, statsA, statsB, true);

    //     // faza II
    //     std::cout << "Wyniki po II fazie\n";
    //     std::cout << "    srednia (min - max)\n\n";
       
    //     printStatsTable(labels, statsA, statsB, false);
    //     std::cout << "wyniki po ! fazie (dlugosc trasy)\n";
    //     printStatsTable(labels, phaseIA_length, phaseIB_length, true);
    //     // Najlepsze rozwiazania
    //     std::cout << "Najlepsze rozwiazania\n";
    //     for (int i = 0; i < (int)labels.size(); ++i) {
    //         std::cout << "  " << std::left << std::setw(12) << labels[i] << std::right
    //                   << "  TSPA: wynik=" << std::setw(7) << tspA.evaluate(statsA[i].bestTour)
    //                   << "  n=" << std::setw(3) << statsA[i].bestTour.size()
    //                   << "  profit = " << std::setw(7) << tspA.evaluate_profit(statsA[i].bestTour)
    //                   << "  distance = " << std::setw(7) << tspA.evaluate_distance(statsA[i].bestTour)
    //                   << "  ||  TSPB: wynik=" << std::setw(7) << tspB.evaluate(statsB[i].bestTour)
    //                   << "  n=" << std::setw(3) << statsB[i].bestTour.size() << "\n"
    //                   << "  profit = " << std::setw(7) << tspB.evaluate_profit(statsB[i].bestTour)
    //                   << "  distance = " << std::setw(7) << tspB.evaluate_distance(statsB[i].bestTour) << "\n";
    //     }

    //     // Eksport najlepszych rozwiązań do plików dla Solution Checkera
    //     // Format: jedna liczba na linię = ID wierzchołka (0-indexed), wkleić do kolumny F od wiersza 3
    //     std::cout << "\nEksport rozwiazań do plikow solution_*.txt ...\n";
    //     for (int i = 0; i < (int)labels.size(); ++i) {
    //         auto exportTour = [&](const std::string& instance,
    //                               const std::vector<int>& tour) {
    //             std::string fname = "solution_" + instance + "_" + labels[i] + ".txt";
    //             // zamień spacje na podkreślniki w nazwie pliku
    //             for (char& c : fname) if (c == ' ') c = '_';
    //             std::ofstream f(fname);
    //             for (int node : tour)
    //                 f << node << "\n";
    //         };
    //         exportTour("TSPA", statsA[i].bestTour);
    //         exportTour("TSPB", statsB[i].bestTour);
    //     }
    //     std::cout << "Gotowe. Wklej zawartosc pliku do kolumny F (od wiersza 3) w Solution checker2.xlsx\n";

    // } catch (const std::exception& e) {
    //     std::cerr << "Blad: " << e.what() << "\n";
    //     return 1;
    // }
    return 0;
}
