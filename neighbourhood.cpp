#include "neighbourhood.h"
#include <algorithm>
#include <random>
#include <iostream>
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
            out_neighbours.emplace_back(MoveType::swap_edges, i, j);
        }
    }
}
Neighbour generateRandomNeigbour(const std::vector<int>& tour, const TSPInstance& tsp,std::mt19937& rng){
    std::uniform_int_distribution<int> moveTypeDist(0, 3);
    MoveType moveType = static_cast<MoveType>(moveTypeDist(rng));
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
            while(second_edge == first_edge || second_edge == (first_edge + 1) %k)
                second_edge = swap_e_dist(rng);
            if(first_edge > second_edge) std::swap(first_edge, second_edge);
            return Neighbour(MoveType::swap_edges, first_edge, second_edge);}
        case MoveType::empty:
            throw std::runtime_error("Generated empty move type in generateRandomNeighbour");
    }
}