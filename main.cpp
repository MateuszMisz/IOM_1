#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include "tsp.h"
#include "algorithms.h"
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

void printWalkStatRow(AlgoStatsTimed stats, std::string prefix){


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
        std::cout<<"Run "<<i+1<<"/"<<runs<<"\n";
        AlgoStatsTimed greedy_walk_a_best_edges = collectStatsWalk(greedyWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1);
        AlgoStatsTimed greedy_walk_b_best_edges = collectStatsWalk(greedyWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1);
        AlgoStatsTimed greedy_walk_a_random_edges = collectStatsWalk(greedyWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1);
        AlgoStatsTimed greedy_walk_b_random_edges = collectStatsWalk(greedyWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1);

        AlgoStatsTimed greedy_walk_a_best_vertices = collectStatsWalk(greedyWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1);
        AlgoStatsTimed greedy_walk_b_best_vertices = collectStatsWalk(greedyWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1);
        AlgoStatsTimed greedy_walk_a_random_vertices = collectStatsWalk(greedyWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1);
        AlgoStatsTimed greedy_walk_b_random_vertices = collectStatsWalk(greedyWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1);
        std::cout<<"greedy walks done\n";
        AlgoStatsTimed steep_walk_a_best_edges = collectStatsWalk(steepestWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1);
        std::cout<<"steep1 done\n";
        AlgoStatsTimed steep_walk_b_best_edges = collectStatsWalk(steepestWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1);
        std::cout<<"steep2 done\n";
        AlgoStatsTimed steep_walk_a_random_edges = collectStatsWalk(steepestWalk, randomBaseSolutionsA[i], tspA, InTourMoveType::SwapEdges, 1);
        std::cout<<"steep3 done\n";
        AlgoStatsTimed steep_walk_b_random_edges = collectStatsWalk(steepestWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapEdges, 1);
        std::cout<<"steep4 done\n";

        AlgoStatsTimed steep_walk_a_best_vertices = collectStatsWalk(steepestWalk, bestBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1);
        std::cout<<"steep5 done\n";
        AlgoStatsTimed steep_walk_b_best_vertices = collectStatsWalk(steepestWalk, bestBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1);
        std::cout<<"steep6 done\n";
        AlgoStatsTimed steep_walk_a_random_vertices = collectStatsWalk(steepestWalk,    randomBaseSolutionsA[i], tspA, InTourMoveType::SwapVertices, 1);
        std::cout<<"steep7 done\n";
        AlgoStatsTimed steep_walk_b_random_vertices = collectStatsWalk(steepestWalk, randomBaseSolutionsB[i], tspB, InTourMoveType::SwapVertices, 1);
        std::cout<<"steep walks done\n";
        AlgoStatsTimed random_walk_a_best_edges = collectRandomWalkStats(tspA,bestBaseSolutionsA[i], InTourMoveType::SwapEdges, 1,rng,std::max(greedy_walk_a_best_edges.time_stats.avg,steep_walk_a_best_edges.time_stats.avg));
        AlgoStatsTimed random_walk_b_best_edges = collectRandomWalkStats(tspB,bestBaseSolutionsB[i], InTourMoveType::SwapEdges, 1,rng,std::max(greedy_walk_b_best_edges.time_stats.avg,steep_walk_b_best_edges.time_stats.avg));
        AlgoStatsTimed random_walk_a_random_edges = collectRandomWalkStats(tspA,randomBaseSolutionsA[i], InTourMoveType::SwapEdges, 1,rng,std::max(greedy_walk_a_random_edges.time_stats.avg,steep_walk_a_random_edges.time_stats.avg));
        AlgoStatsTimed random_walk_b_random_edges = collectRandomWalkStats(tspB,randomBaseSolutionsB[i], InTourMoveType::SwapEdges, 1,rng,std::max(greedy_walk_b_random_edges.time_stats.avg,steep_walk_b_random_edges.time_stats.avg));

        AlgoStatsTimed random_walk_a_best_vertices = collectRandomWalkStats(tspA,bestBaseSolutionsA[i], InTourMoveType::SwapVertices, 1,rng,std::max(greedy_walk_a_best_vertices.time_stats.avg,steep_walk_a_best_vertices.time_stats.avg));
        AlgoStatsTimed random_walk_b_best_vertices = collectRandomWalkStats(tspB,bestBaseSolutionsB[i], InTourMoveType::SwapVertices, 1,rng,std::max(greedy_walk_b_best_vertices.time_stats.avg,steep_walk_b_best_vertices.time_stats.avg));
        AlgoStatsTimed random_walk_a_random_vertices = collectRandomWalkStats(tspA,randomBaseSolutionsA[i], InTourMoveType::SwapVertices, 1,rng,std::max(greedy_walk_a_random_vertices.time_stats.avg,steep_walk_a_random_vertices.time_stats.avg));
        AlgoStatsTimed random_walk_b_random_vertices = collectRandomWalkStats(tspB,randomBaseSolutionsB[i], InTourMoveType::SwapVertices, 1,rng,std::max(greedy_walk_b_random_vertices.time_stats.avg,steep_walk_b_random_vertices.time_stats.avg));
    }
    
    //print walk times 
    if(true){
    std::cout<<"algorytm;instancja;rozwiązanie bazowe;  typ ruchu wewnątrztrasowego; średni czas, max czas, min czas";
    std::cout<<"greedy_walk;A;regretGC;swap_edges"<<greedy_walk_a_best_edges.time_stats.avg<<";"<<greedy_walk_a_best_edges.time_stats.max<<";"<<greedy_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_edges"<<greedy_walk_b_best_edges.time_stats.avg<<";"<<greedy_walk_b_best_edges.time_stats.max<<";"<<greedy_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_edges"<<greedy_walk_a_random_edges.time_stats.avg<<";"<<greedy_walk_a_random_edges.time_stats.max<<";"<<greedy_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_edges"<<greedy_walk_b_random_edges.time_stats.avg<<";"<<greedy_walk_b_random_edges.time_stats.max<<";"<<greedy_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"greedy_walk;A;regretGC;swap_vertices"<<greedy_walk_a_best_vertices.time_stats.avg<<";"<<greedy_walk_a_best_vertices.time_stats.max<<";"<<greedy_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_vertices"<<greedy_walk_b_best_vertices.time_stats.avg<<";"<<greedy_walk_b_best_vertices.time_stats.max<<";"<<greedy_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_vertices"<<greedy_walk_a_random_vertices.time_stats.avg<<";"<<greedy_walk_a_random_vertices.time_stats.max<<";"<<greedy_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_vertices"<<greedy_walk_b_random_vertices.time_stats.avg<<";"<<greedy_walk_b_random_vertices.time_stats.max<<";"<<greedy_walk_b_random_vertices.time_stats.min<<"\n";
    
    std::cout<<"steep_walk;A;regretGC;swap_edges"<<steep_walk_a_best_edges.time_stats.avg<<";"<<steep_walk_a_best_edges.time_stats.max<<";"<<steep_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_edges"<<steep_walk_b_best_edges.time_stats.avg<<";"<<steep_walk_b_best_edges.time_stats.max<<";"<<steep_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_edges"<<steep_walk_a_random_edges.time_stats.avg<<";"<<steep_walk_a_random_edges.time_stats.max<<";"<<steep_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_edges"<<steep_walk_b_random_edges.time_stats.avg<<";"<<steep_walk_b_random_edges.time_stats.max<<";"<<steep_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"steep_walk;A;regretGC;swap_vertices"<<steep_walk_a_best_vertices.time_stats.avg<<";"<<steep_walk_a_best_vertices.time_stats.max<<";"<<steep_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_vertices"<<steep_walk_b_best_vertices.time_stats.avg<<";"<<steep_walk_b_best_vertices.time_stats.max<<";"<<steep_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_vertices"<<steep_walk_a_random_vertices.time_stats.avg<<";"<<steep_walk_a_random_vertices.time_stats.max<<";"<<steep_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_vertices"<<steep_walk_b_random_vertices.time_stats.avg<<";"<<steep_walk_b_random_vertices.time_stats.max<<";"<<steep_walk_b_random_vertices.time_stats.min<<"\n";  

    std::cout<<"random_walk;A;regretGC;swap_edges"<<random_walk_a_best_edges.time_stats.avg<<";"<<random_walk_a_best_edges.time_stats.max<<";"<<random_walk_a_best_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_edges"<<random_walk_b_best_edges.time_stats.avg<<";"<<random_walk_b_best_edges.time_stats.max<<";"<<random_walk_b_best_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_edges"<<random_walk_a_random_edges.time_stats.avg<<";"<<random_walk_a_random_edges.time_stats.max<<";"<<random_walk_a_random_edges.time_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_edges"<<random_walk_b_random_edges.time_stats.avg<<";"<<random_walk_b_random_edges.time_stats.max<<";"<<random_walk_b_random_edges.time_stats.min<<"\n";

    std::cout<<"random_walk;A;regretGC;swap_vertices"<<random_walk_a_best_vertices.time_stats.avg<<";"<<random_walk_a_best_vertices.time_stats.max<<";"<<random_walk_a_best_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_vertices"<<random_walk_b_best_vertices.time_stats.avg<<";"<<random_walk_b_best_vertices.time_stats.max<<";"<<random_walk_b_best_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_vertices"<<random_walk_a_random_vertices.time_stats.avg<<";"<<random_walk_a_random_vertices.time_stats.max<<";"<<random_walk_a_random_vertices.time_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_vertices"<<random_walk_b_random_vertices.time_stats.avg<<";"<<random_walk_b_random_vertices.time_stats.max<<";"<<random_walk_b_random_vertices.time_stats.min<<"\n";
    }
    //print walk scores
    if(true){
    std::cout<<"\n\n\nalgorytm;instancja;rozwiązanie bazowe;  typ ruchu wewnątrztrasowego; średni wynik, max wynik, min wynik";
    std::cout<<"greedy_walk;A;regretGC;swap_edges"<<greedy_walk_a_best_edges.score_stats.avg<<";"<<greedy_walk_a_best_edges.score_stats.max<<";"<<greedy_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_edges"<<greedy_walk_b_best_edges.score_stats.avg<<";"<<greedy_walk_b_best_edges.score_stats.max<<";"<<greedy_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_edges"<<greedy_walk_a_random_edges.score_stats.avg<<";"<<greedy_walk_a_random_edges.score_stats.max<<";"<<greedy_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_edges"<<greedy_walk_b_random_edges.score_stats.avg<<";"<<greedy_walk_b_random_edges.score_stats.max<<";"<<greedy_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"greedy_walk;A;regretGC;swap_vertices"<<greedy_walk_a_best_vertices.score_stats.avg<<";"<<greedy_walk_a_best_vertices.score_stats.max<<";"<<greedy_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;regretGC;swap_vertices"<<greedy_walk_b_best_vertices.score_stats.avg<<";"<<greedy_walk_b_best_vertices.score_stats.max<<";"<<greedy_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;A;random;swap_vertices"<<greedy_walk_a_random_vertices.score_stats.avg<<";"<<greedy_walk_a_random_vertices.score_stats.max<<";"<<greedy_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"greedy_walk;B;random;swap_vertices"<<greedy_walk_b_random_vertices.score_stats.avg<<";"<<greedy_walk_b_random_vertices.score_stats.max<<";"<<greedy_walk_b_random_vertices.score_stats.min<<"\n";
    
    std::cout<<"steep_walk;A;regretGC;swap_edges"<<steep_walk_a_best_edges.score_stats.avg<<";"<<steep_walk_a_best_edges.score_stats.max<<";"<<steep_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_edges"<<steep_walk_b_best_edges.score_stats.avg<<";"<<steep_walk_b_best_edges.score_stats.max<<";"<<steep_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_edges"<<steep_walk_a_random_edges.score_stats.avg<<";"<<steep_walk_a_random_edges.score_stats.max<<";"<<steep_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_edges"<<steep_walk_b_random_edges.score_stats.avg<<";"<<steep_walk_b_random_edges.score_stats.max<<";"<<steep_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"steep_walk;A;regretGC;swap_vertices"<<steep_walk_a_best_vertices.score_stats.avg<<";"<<steep_walk_a_best_vertices.score_stats.max<<";"<<steep_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;regretGC;swap_vertices"<<steep_walk_b_best_vertices.score_stats.avg<<";"<<steep_walk_b_best_vertices.score_stats.max<<";"<<steep_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;A;random;swap_vertices"<<steep_walk_a_random_vertices.score_stats.avg<<";"<<steep_walk_a_random_vertices.score_stats.max<<";"<<steep_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"steep_walk;B;random;swap_vertices"<<steep_walk_b_random_vertices.score_stats.avg<<";"<<steep_walk_b_random_vertices.score_stats.max<<";"<<steep_walk_b_random_vertices.score_stats.min<<"\n";  

    std::cout<<"random_walk;A;regretGC;swap_edges"<<random_walk_a_best_edges.score_stats.avg<<";"<<random_walk_a_best_edges.score_stats.max<<";"<<random_walk_a_best_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_edges"<<random_walk_b_best_edges.score_stats.avg<<";"<<random_walk_b_best_edges.score_stats.max<<";"<<random_walk_b_best_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_edges"<<random_walk_a_random_edges.score_stats.avg<<";"<<random_walk_a_random_edges.score_stats.max<<";"<<random_walk_a_random_edges.score_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_edges"<<random_walk_b_random_edges.score_stats.avg<<";"<<random_walk_b_random_edges.score_stats.max<<";"<<random_walk_b_random_edges.score_stats.min<<"\n";

    std::cout<<"random_walk;A;regretGC;swap_vertices"<<random_walk_a_best_vertices.score_stats.avg<<";"<<random_walk_a_best_vertices.score_stats.max<<";"<<random_walk_a_best_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;B;regretGC;swap_vertices"<<random_walk_b_best_vertices.score_stats.avg<<";"<<random_walk_b_best_vertices.score_stats.max<<";"<<random_walk_b_best_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;A;random;swap_vertices"<<random_walk_a_random_vertices.score_stats.avg<<";"<<random_walk_a_random_vertices.score_stats.max<<";"<<random_walk_a_random_vertices.score_stats.min<<"\n";
    std::cout<<"random_walk;B;random;swap_vertices"<<random_walk_b_random_vertices.score_stats.avg<<";"<<random_walk_b_random_vertices.score_stats.max<<";"<<random_walk_b_random_vertices.score_stats.min<<"\n";
    }
    std::cout<<"\n\n\nalgorytm;instancja;średni wynik, max wynik, min wynik";
    std::cout<<"regretGC;A;"<<baseStatsA.phaseII.avg<<";"<<baseStatsA.phaseII.max<<";"<<baseStatsA.phaseII.min<<"\n";
    std::cout<<"regretGC;B;"<<baseStatsB.phaseII.avg<<";"<<baseStatsB.phaseII.max<<";"<<baseStatsB.phaseII.min<<"\n";
    std::cout<<"random;A;"<<randomBaseStatsA.phaseII.avg<<";"<<randomBaseStatsA.phaseII.max<<";"<<randomBaseStatsA.phaseII.min<<"\n";
    std::cout<<"random;B;"<<randomBaseStatsB.phaseII.avg<<";"<<randomBaseStatsB.phaseII.max<<";"<<randomBaseStatsB.phaseII.min<<"\n";

    
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
