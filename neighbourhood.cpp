#include "neighbourhood.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <stdexcept>
#include <set>
#include <tuple>
void Neighbour::transform(std::vector<int>& tour, const TSPInstance& tsp) {
    switch (type) {
        case MoveType::Add:           add(tour, tsp); break;
        case MoveType::remove:        remove(tour, tsp); break;
        case MoveType::swap_vertices: swapVertices(tour, tsp); break;
        case MoveType::swap_edges:    swapEdges(tour, tsp); break;
    }
}
void Neighbour::add(std::vector<int>& tour, const TSPInstance& tsp) {
    tour.insert(tour.begin() + second, first);
}
void Neighbour::remove(std::vector<int>& tour, const TSPInstance& tsp) {
    tour.erase(tour.begin() + first);
}
void Neighbour::swapVertices(std::vector<int>& tour, const TSPInstance& tsp) {
    if(tour[first]<0 || tour[second]<0){
        puts("problem");
    }
    std::swap(tour[first], tour[second]);
}
/**
 * zamiana krawędzi to wg obrazka z polecenia odwrócenie fragmentu trasy (first, second] - przy zalozeniu oznaczen krawedzi przez indeks ich wcześniejszego wierzchołka w rozwiązaniu
 */
void Neighbour::swapEdges(std::vector<int>& tour, const TSPInstance& tsp) {

    auto start_iter = tour.begin() + first + 1;
    auto end_iter = tour.begin() + second + 1; 
    if(start_iter > end_iter) // jesli second < first, to zamieniami pierwszy w reverse z drugim. daje to geometrycznie ten sam efekt
        std::swap(start_iter, end_iter);
    std::reverse(start_iter, end_iter);
}
std::vector<Neighbour> generateNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, InTourMoveType moveType){
    int k = tour.size();
    int n = tsp.size();
    int add_remove_neigbhours = (n-k)*(k+1)+k; // wszystkie dodania + wszystkie usunięcia
    int total_neighbours=add_remove_neigbhours+3;//+3 dla sweitego spokoju
    switch (moveType) {
        case InTourMoveType::SwapVertices: total_neighbours +=(k*(k-1)/2); break;
        case InTourMoveType::SwapEdges: total_neighbours +=(k*(k-3)/2); break;
    }
    std::vector<Neighbour> neighbours;
    neighbours.reserve(total_neighbours);
    generateAddNeighbourhood(tour, tsp, neighbours);
    generateRemoveNeighbourhood(tour, tsp, neighbours);
    switch (moveType) {
        case InTourMoveType::SwapVertices: generateSwapVerticesNeighbourhood(tour, tsp, neighbours); break;
        case InTourMoveType::SwapEdges: generateSwapEdgesNeighbourhood(tour, tsp, neighbours); break;
    }
    return neighbours;
}
CandidateLists buildCandidateLists(const TSPInstance& tsp, int candidate_count) {
    int n = tsp.size();
    CandidateLists candidates(n);
    for (int v = 0; v < n; ++v) {
        std::vector<int> others;
        others.reserve(n - 1);
        for (int u = 0; u < n; ++u) {
            if (u != v) others.push_back(u);
        }
        std::sort(others.begin(), others.end(), [&](int a, int b) {
            if (tsp.dist[v][a] != tsp.dist[v][b]) return tsp.dist[v][a] < tsp.dist[v][b];
            return a < b;
        });
        int take = std::min(candidate_count, static_cast<int>(others.size()));
        candidates[v].assign(others.begin(), others.begin() + take);
    }
    return candidates;
}
std::vector<Neighbour> generateCandidateNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, InTourMoveType moveType, const CandidateLists& candidates) {
    int k = static_cast<int>(tour.size());
    int n = tsp.size();
    std::vector<Neighbour> neighbours;

    std::vector<bool> inTour(n, false);
    std::vector<int> pos(n, -1);
    for (int i = 0; i < k; ++i) {
        inTour[tour[i]] = true;
        pos[tour[i]] = i;
    }

    std::vector<std::vector<unsigned char>> candidateEdge(n, std::vector<unsigned char>(n, 0));
    std::vector<std::vector<int>> incidentCandidates(n);
    for (int v = 0; v < n; ++v) {
        for (int u : candidates[v]) {
            if (u < 0 || u >= n || u == v) continue;
            if (!candidateEdge[v][u]) {
                candidateEdge[v][u] = 1;
                candidateEdge[u][v] = 1;
                incidentCandidates[v].push_back(u);
                incidentCandidates[u].push_back(v);
            }
        }
    }

    std::vector<std::vector<unsigned char>> seenAdd(n, std::vector<unsigned char>(k + 1, 0));
    auto addUniqueAdd = [&](int v, int insertPos) {
        if (v < 0 || v >= n || insertPos < 0 || insertPos > k) return;
        if (!seenAdd[v][insertPos]) {
            seenAdd[v][insertPos] = 1;
            neighbours.emplace_back(MoveType::Add, v, insertPos);
        }
    };

    // for (int v = 0; v < n; ++v) {
    //     if (inTour[v]) continue;
    //     for (int near : incidentCandidates[v]) {
    //         if (!inTour[near]) continue;
    //         int p = pos[near];
    //         addUniqueAdd(v, p);
    //         addUniqueAdd(v, p + 1);
    //     }
    // }
    for (int v = 0; v < tour.size();v++){
        int v_global_idx = tour[v];
        for(int near : incidentCandidates[v_global_idx]){
            if(!inTour[near]){

            addUniqueAdd(near, v);
            addUniqueAdd(near, v+1);
            }
        }
    }

    if (k > 2) {
        for (int i = 0; i < k; ++i) {
            int prev = tour[(i - 1 + k) % k];
            int next = tour[(i + 1) % k];
            if (candidateEdge[prev][next]) {
                neighbours.emplace_back(MoveType::remove, i, -1);
            }
        }
    }

    switch (moveType) {
        case InTourMoveType::SwapVertices:
        {
            for (int i = 0; i < k; ++i) {
                for (int j = i + 1; j < k; ++j) {
                    int b = tour[i];
                    int c = tour[j];
                    int prevI = tour[(i - 1 + k) % k];
                    int nextI = tour[(i + 1) % k];
                    int prevJ = tour[(j - 1 + k) % k];
                    int nextJ = tour[(j + 1) % k];
                    bool candidate = false;

                    if (i + 1 == j) {
                        candidate = candidateEdge[prevI][c] || candidateEdge[c][b] || candidateEdge[b][nextJ];
                    } else if (i == 0 && j == k - 1) {
                        candidate = candidateEdge[prevJ][b] || candidateEdge[b][c] || candidateEdge[c][nextI];
                    } else {
                        candidate = candidateEdge[prevI][c] || candidateEdge[c][nextI] ||
                                    candidateEdge[prevJ][b] || candidateEdge[b][nextJ];
                    }

                    if (candidate) neighbours.emplace_back(MoveType::swap_vertices, i, j);
                }
            }
            break;
        }
        case InTourMoveType::SwapEdges:
        {
            std::vector<std::vector<unsigned char>> seenSwap(k, std::vector<unsigned char>(k, 0));
            auto addUniqueSwapEdges = [&](int i, int j) {
                if (i == j) return;
                if (i > j) std::swap(i, j);
                if (j == i + 1) return;
                if (i == 0 && j == k - 1) return;
                if (!seenSwap[i][j]) {
                    seenSwap[i][j] = 1;
                    neighbours.emplace_back(MoveType::swap_edges, i, j);
                }
            };

            for (int i = 0; i < k; ++i) {
                int a = tour[i];
                int b = tour[(i + 1) % k];

                for (int c : incidentCandidates[a]) {
                    if (inTour[c]) addUniqueSwapEdges(i, pos[c]);
                }
                for (int d : incidentCandidates[b]) {
                    if (inTour[d]) addUniqueSwapEdges(i, (pos[d] - 1 + k) % k);
                }
            }
            break;
        }
    }
    return neighbours;
}
void generateAddNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp,std::vector<Neighbour>& out_neighbours) {
    std::vector<bool> inTour(tsp.size(), false);
    for (int v : tour) inTour[v] = true;

    int k = static_cast<int>(tour.size());
    for (int v = 0; v < tsp.size(); ++v) {
        if (inTour[v]) continue;
        for (int i = 0; i <= k; ++i) {
            out_neighbours.emplace_back(MoveType::Add, v, i);
        }
    }
}
void generateRemoveNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours) {
    int k = static_cast<int>(tour.size());
    if (k <= 2) return;
    for (int i = 0; i < k; ++i) {
        out_neighbours.emplace_back(MoveType::remove, i,-1);
    }
}
void generateSwapVerticesNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours) {
    int k = static_cast<int>(tour.size());
    for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
            out_neighbours.emplace_back(MoveType::swap_vertices, i, j);
        }
    }
}
void generateSwapEdgesNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours) {
    int k = static_cast<int>(tour.size());
    for (int i = 0; i < k; ++i) {
        for (int j = i + 2; j < k; ++j) { // unikamy sąsiednich krawędzi (i, i+1) oraz (k-1, 0), 
            if (i == 0 && j == k - 1) continue;
            out_neighbours.emplace_back(MoveType::swap_edges, i, j);
        }
    }
}
Neighbour generateRandomNeigbour(const std::vector<int>& tour, const TSPInstance& tsp,std::mt19937& rng,MoveType moveType) {
    if(moveType == MoveType::empty){
        std::uniform_int_distribution<int> moveTypeDist(0, 3);
        moveType = static_cast<MoveType>(moveTypeDist(rng));
    }
    
    int n = tsp.size();
    int k = tour.size();

    switch(moveType){
        case MoveType::Add:{
            std::uniform_int_distribution<int> n_dist(0,n-1);
            std::uniform_int_distribution<int> pos_dist(0,k);
            return Neighbour(MoveType::Add, n_dist(rng), pos_dist(rng));}

        case MoveType::remove:{
            std::uniform_int_distribution<int> remove_pos_dist(0,k-1);
            return Neighbour(MoveType::remove, remove_pos_dist(rng), -1);}
        case MoveType::swap_vertices:{
            std::uniform_int_distribution<int> swap_v_dist(0,k-1);
            int first = swap_v_dist(rng);
            int second = swap_v_dist(rng); 
            while(second == first){
                second = swap_v_dist(rng);
            }
            if(first > second) std::swap(first, second);
            return Neighbour(MoveType::swap_vertices, first, second);}
        case MoveType::swap_edges:{
            std::uniform_int_distribution<int> swap_e_dist(0,k-1);
            int first_edge = swap_e_dist(rng);
            int second_edge = swap_e_dist(rng);
            while(second_edge == first_edge || second_edge == (first_edge + 1) % k || first_edge == (second_edge + 1) % k)
                second_edge = swap_e_dist(rng);
            if(first_edge > second_edge) std::swap(first_edge, second_edge);
            return Neighbour(MoveType::swap_edges, first_edge, second_edge);}
        case MoveType::empty:
            throw std::runtime_error("Generated empty move type in generateRandomNeighbour");
    }
    throw std::runtime_error("Unknown move type in generateRandomNeighbour");
}
