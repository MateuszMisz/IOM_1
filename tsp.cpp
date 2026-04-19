#include "tsp.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <iostream>
TSPInstance::TSPInstance(const std::string& filename) : name(filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Nie mozna otworzyc pliku: " + filename);

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;
        Node node;
        std::getline(ss, token, ';'); node.x      = std::stoi(token);
        std::getline(ss, token, ';'); node.y      = std::stoi(token);
        std::getline(ss, token, ';'); node.profit = std::stoi(token);
        nodes.push_back(node);
    }

    int n = static_cast<int>(nodes.size());
    dist.assign(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            int d = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
            dist[i][j] = dist[j][i] = d;
        }
}
/**
 * @brief zwraca rożnicę zysk - dystans w sciezce
 */
int TSPInstance::evaluate(const std::vector<int>& tour) const {
    int profit = 0, distance = 0;
    int k = static_cast<int>(tour.size());
    for (int i = 0; i < k; ++i) {
        profit   += nodes[tour[i]].profit;
        distance += dist[tour[i]][tour[(i + 1) % k]];
    }
    return profit - distance;
}
int TSPInstance::evaluate_distance(const std::vector<int>& tour) const {
    int profit = 0, distance = 0;
    int k = static_cast<int>(tour.size());
    for (int i = 0; i < k; ++i) {
        profit   += nodes[tour[i]].profit;
        distance += dist[tour[i]][tour[(i + 1) % k]];
    }
    return distance;
}
int TSPInstance::evaluate_profit(const std::vector<int>& tour) const {
    int profit = 0, distance = 0;
    int k = static_cast<int>(tour.size());
    for (int i = 0; i < k; ++i) {
        profit   += nodes[tour[i]].profit;
        distance += dist[tour[i]][tour[(i + 1) % k]];
    }
    return profit;
}
// int TSPInstance::evaluate_score(const std::vector<int>& tour) const {
//     int profit = 0, distance = 0;
//     int k = static_cast<int>(tour.size());
//     for (int i = 0; i < k; ++i) {
//         profit   += nodes[tour[i]].profit;
//         distance += dist[tour[i]][tour[(i + 1) % k]];
//     }
//     return profit;
// }
