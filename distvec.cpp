#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility> // for std::pair
#include <algorithm> // for max function
#include <tuple>

bool debug = true;
const int INF = 1e9; // Representing infinity

// nodes are numbered from 1 to N and the graph is undirected
// Using vector of vectors to represent an adjacency list, where each pair is (neighbor, cost)
typedef std::vector<std::vector<std::pair<int, int>>> Graph;

void applyChangeToGraph(Graph& graph, int node1, int node2, int newCost, int maxNodes) {
    bool edgeFound = false;
    // Update the cost if the edge exists
    for (auto& edge : graph[node1]) {
        if (edge.first == node2) {
            if (newCost == -999)
                edge.second = INF; // Or remove the edge entirely
            else
                edge.second = newCost;
            edgeFound = true;
            break;
        }
    }
    if (!edgeFound && newCost != INF) {
        // If the edge does not exist and we're not removing, add it
        graph[node1].push_back(std::make_pair(node2, newCost));
    }
    // Since the graph is undirected, repeat for node2 to node1
    edgeFound = false;
    for (auto& edge : graph[node2]) {
        if (edge.first == node1) {
            if (newCost == -999)
                edge.second = INF; // Or remove the edge entirely
            else
                edge.second = newCost;
            edgeFound = true;
            break;
        }
    }
    if (!edgeFound && newCost != INF) {
        graph[node2].push_back(std::make_pair(node1, newCost));
    }
}

void recomputeDistanceVectors(const Graph& graph, std::vector<std::vector<int>>& dist, int maxNodes) {
    // Reset the distance vectors
    for(int i = 1; i <= maxNodes; ++i) {
        std::fill(dist[i].begin(), dist[i].end(), INF);
        dist[i][i] = 0; // Distance to itself is 0
        for(const auto& edge : graph[i]) {
            int neighbor = edge.first;
            int cost = edge.second;
            dist[i][neighbor] = cost; // Direct neighbor cost
        }
    }

    // Apply the DV update logic
    bool updated;
    do {
        updated = false;
        for (int i = 1; i <= maxNodes; ++i) { // For each node
            for (const auto &edge : graph[i]) { // For each neighbor
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
}

void printDistanceTable(const std::vector<std::vector<int>>& dist, int maxNodes, const int INF) {
    std::cout << "Distance table:" << std::endl;
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
    if (debug) {
        printDistanceTable(dist, maxNodes, INF);
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

    // handle the topology changes
    std::ifstream changesFile(argv[3]);
    if (!changesFile.is_open()) {
        std::cerr << "Could not open changes file\n";
        return -1;
    }

    int newCost;
    while (changesFile >> node1 >> node2 >> newCost) {
        // Apply the change to the graph
        // Note: You'll need to adjust your graph structure to allow for easy updates

        std::cout << "Applying change: Node1=" << node1 << ", Node2=" << node2 << ", NewCost=" << newCost << std::endl;
        applyChangeToGraph(graph, node1, node2, newCost, maxNodes);
        std::cout << "Change applied. Recomputing distance vectors..." << std::endl;
        // Recompute the Distance Vectors
        recomputeDistanceVectors(graph, dist, maxNodes);
        std::cout << "Distance vectors recomputed." << std::endl;
        if (debug) {
            printDistanceTable(dist, maxNodes, INF);
        }
    }

    changesFile.close();

    return 0;
}
