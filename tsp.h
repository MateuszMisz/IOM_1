#pragma once
#include <vector>
#include <string>

struct Node {
    int x, y, profit;
};
struct TSPInstance {
    std::string name;
    std::vector<Node> nodes;
    std::vector<std::vector<int>> dist;

    explicit TSPInstance(const std::string& filename);
    int size() const { return static_cast<int>(nodes.size()); }
    int evaluate(const std::vector<int>& tour) const;
    int evaluate_distance(const std::vector<int>& tour) const;
    int evaluate_profit(const std::vector<int>& tour) const;
};
