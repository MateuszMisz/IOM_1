#pragma once
#include<vector>
#include "tsp.h"
using CandidateLists = std::vector<std::vector<int>>;
enum class MoveType {
    Add,
    remove,
    swap_vertices,
    swap_edges,
    empty
};
/**
 * klasa reprezentująca pojedynczego sąsiada.
 * fields:
 *  - type: typ ruchu (Add, Remove, SwapVertices, SwapEdges)
 *  - first: indeks wierzchołka, krawędzi lub pozycji w rozwiązaniu (w zależności od typu ruchu)
 *  - second: indeks wierzchołka, krawędzi, lub pozycji w rozwiązaniu (w zależności od typu ruchu)
 * interpretacja pól first i second zależy od typu ruchu:
 * Dla type = Add:
 *  - first = indeks wierzchołka (z instancji) do dodania()
 *  - second = indeks pozycji w rozwiązaniu za którym bedzie wstawienie (0 jesli dodajemy przed pierwszym, tour.size() jesli dodajemy na koniec)
 * Dla type = Remove:
 *  - first = indeks w rozwiązaniu do usunięcia
 *  - second (nieużywane) = -1 (dla czytelności)
 * Dla type = SwapVertices:
 *  - first = indeks(pozycja) w rozwiązaniu do zamiany
 *  - second = indeks (pozycja) w rozwiązaniu do zamiany
 * Dla type = SwapEdges:
 *  - first = indeks pierwszej krawędzi do zamiany (oznaczana przez indeks jej wcześniejszego wierzchołka w rozwiązaniu)
 *  - second = indeks drugiej krawędzi do zamiany (oznaczana przez indeks jej wcześniejszego wierzchołka w rozwiązaniu)
 */
struct Neighbour {
    MoveType type;
    int first;
    int second;
    Neighbour(MoveType type, int first, int second= -1) : type(type), first(first), second(second) {}
    void transform(std::vector<int>& tour, const TSPInstance& tsp);
private:
    void add(std::vector<int>& tour, const TSPInstance& tsp);
    void remove(std::vector<int>& tour, const TSPInstance& tsp);
    void swapVertices(std::vector<int>& tour, const TSPInstance& tsp);
    void swapEdges(std::vector<int>& tour, const TSPInstance& tsp);
};
enum class InTourMoveType {
    SwapVertices,
    SwapEdges
};
std::vector<Neighbour> generateNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, InTourMoveType moveType);
CandidateLists buildCandidateLists(const TSPInstance& tsp, int candidate_count = 10);
std::vector<Neighbour> generateCandidateNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, InTourMoveType moveType, const CandidateLists& candidates);
void generateAddNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours);
void generateRemoveNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours);
void generateSwapVerticesNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours);
void generateSwapEdgesNeighbourhood(const std::vector<int>& tour, const TSPInstance& tsp, std::vector<Neighbour>& out_neighbours);
