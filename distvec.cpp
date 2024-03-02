#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // for std::pair

// Assuming nodes are numbered from 1 to N and the graph is undirected
// Using vector of vectors to represent an adjacency list, where each pair is (neighbor, cost)
typedef std::vector<std::vector<std::pair<int, int>>> Graph;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: ./distvec topofile messagefile changesfile\n";
        return -1;
    }

    std::ifstream topoFile(argv[1]);
    if (!topoFile.is_open()) {
        std::cerr << "Could not open topology file\n";
        return -1;
    }

    Graph graph;
    int node1, node2, cost;

    // Update this part to correctly initialize your graph based on the maximum node ID
    // For now, let's assume a fixed size graph
    graph.resize(100);

    while (topoFile >> node1 >> node2 >> cost) {
        graph[node1].push_back(std::make_pair(node2, cost));
        graph[node2].push_back(std::make_pair(node1, cost)); // because the graph is undirected
    }

    topoFile.close();

    // debug the topology
    // for (int i = 0; i < graph.size(); ++i) {
    //     if (!graph[i].empty()) {
    //         std::cout << "Node " << i << " -> ";
    //         for (const auto& neighbor : graph[i]) {
    //             std::cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
    //         }
    //         std::cout << "\n";
    //     }
    // }

    // You can now proceed with implementing the Distance Vector algorithm using the populated graph

    return 0;
}
