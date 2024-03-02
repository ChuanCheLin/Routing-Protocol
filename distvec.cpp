#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // for std::pair
#include <algorithm> // for max function
#include <tuple>

bool debug = true;

// nodes are numbered from 1 to N and the graph is undirected
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

    int node1, node2, cost;
    int maxNodes = 0;

    // Temporary storage to hold edge information before resizing the graph
    std::vector<std::tuple<int, int, int>> edges;

    // determine the maximum node ID
    while (topoFile >> node1 >> node2 >> cost) {
        edges.push_back(std::make_tuple(node1, node2, cost));
        maxNodes = std::max(maxNodes, std::max(node1, node2));
    }

    // we know the maximum node ID, we can resize the graph and populate it
    Graph graph(maxNodes + 1); // +1 because nodes are 1-indexed instead of 0-indexed

    // construct the graph
    for (const auto& edge : edges) {
        node1 = std::get<0>(edge);
        node2 = std::get<1>(edge);
        cost = std::get<2>(edge);
        graph[node1].push_back(std::make_pair(node2, cost));
        graph[node2].push_back(std::make_pair(node1, cost)); // because the graph is undirected
    }

    topoFile.close();

    // debug the topology
    if (debug == true){
        for (int i = 0; i < graph.size(); ++i) {
            if (!graph[i].empty()) {
                std::cout << "Node " << i << " -> ";
                for (const auto& neighbor : graph[i]) {
                    std::cout << "(" << neighbor.first << ", " << neighbor.second << ") ";
                }
                std::cout << "\n";
            }
        }
    }

    // the Distance Vector Algorithm

    const int INF = 1e9; // Representing infinity
    std::vector<std::vector<int>> dist(maxNodes+1, std::vector<int>(maxNodes+1, INF));

    // Initialize distance tables
    for(int i = 1; i <= maxNodes; ++i) {
        dist[i][i] = 0; // Distance to itself is 0
        for(auto& edge : graph[i]) {
            int neighbor = edge.first;
            int cost = edge.second;
            dist[i][neighbor] = cost; // Distance to direct neighbor
        }
    }

    // debug after we Initialize distance tables
    if (debug == true){
        std::cout << "Initial distance table:" << std::endl;
        for(int i = 1; i <= maxNodes; ++i) {
            for(int j = 1; j <= maxNodes; ++j) {
                if(dist[i][j] == INF)
                    std::cout << "INF ";
                else
                    std::cout << dist[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    // Iteratively Update Distances
    bool updated;
    do {
        updated = false;
        for (int i = 1; i <= maxNodes; ++i) { // For each node
            for (auto &edge : graph[i]) { // For each neighbor
                int neighbor = edge.first;
                int edgeCost = edge.second;
                for (int j = 1; j <= maxNodes; ++j) { // For each possible destination
                    if (dist[i][j] > dist[i][neighbor] + dist[neighbor][j]) {
                        dist[i][j] = dist[i][neighbor] + dist[neighbor][j];
                        updated = true; // Mark that we made an update
                    }
                }
            }
        }
    } while (updated); // Continue until no updates are made

    // debug the distance table after iteratively update distances until no changes
    if (debug == true){
        std::cout << "Final distance table:" << std::endl;
        for(int i = 1; i <= maxNodes; ++i) {
            for(int j = 1; j <= maxNodes; ++j) {
                if(dist[i][j] == INF)
                    std::cout << "INF ";
                else
                    std::cout << dist[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    // handle message forwarding

    std::ifstream messageFile(argv[2]);
    if (!messageFile.is_open()) {
        std::cerr << "Could not open message file\n";
        return -1;
    }

    int src, dest;
    std::string messageContent;
    while (messageFile >> src >> dest) {
        std::getline(messageFile, messageContent); // To read the message content
        if (dist[src][dest] == INF) {
            std::cout << "From " << src << " to " << dest << " is unreachable." << std::endl;
        } else {
            std::cout << "From " << src << " to " << dest << " costs " << dist[src][dest] << ", message: " << messageContent << std::endl;
        }
    }

    messageFile.close();

    // // handle the topology changes
    // std::ifstream changesFile(argv[3]);
    // if (!changesFile.is_open()) {
    //     std::cerr << "Could not open changes file\n";
    //     return -1;
    // }

    // int node1, node2, newCost;
    // while (changesFile >> node1 >> node2 >> newCost) {
    //     // Apply the change to the graph
    //     // Note: You'll need to adjust your graph structure to allow for easy updates
    //     applyChangeToGraph(graph, node1, node2, newCost, maxNodes);

    //     // Recompute the Distance Vectors
    //     recomputeDistanceVectors(graph, dist, maxNodes);

    //     // Optionally, resimulate message forwarding if necessary
    // }

    // changesFile.close();

    return 0;
}
