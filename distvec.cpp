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


void sendMessage(std::ofstream& outputFile, int src, int dest, const std::vector<std::vector<int>>& dist, const std::vector<std::vector<int>>& nextHop, const std::string& message, const int INF) {
    if (dist[src][dest] == INF) {
        // Destination is not reachable
        outputFile << "from " << src << " to " << dest << " cost infinite hops unreachable message " << message << std::endl;
    } else {
        // Trace the path using the nextHop table
        std::vector<int> path;
        int currentNode = src;
        while (currentNode != dest) {
            path.push_back(currentNode);
            currentNode = nextHop[currentNode][dest];
            if (currentNode == -1) { // Break loop if path is incomplete or broken
                break;
            }
        }

        // Output the message routing information
        outputFile << "from " << src << " to " << dest << " cost " << dist[src][dest] << " hops ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) outputFile << " ";
            outputFile << path[i];
        }
        outputFile << " message " << message << std::endl;
    }
}

void handleMessageForwarding(const std::string& messageFilePath, const std::vector<std::vector<int>>& dist, const std::vector<std::vector<int>>& nextHop, std::ofstream& outputFile, const int INF) {
    std::ifstream messageFile(messageFilePath);
    if (!messageFile.is_open()) {
        std::cerr << "Could not open message file\n";
        return;
    }

    int src, dest;
    std::string messageContent;
    while (messageFile >> src >> dest) {
        std::getline(messageFile, messageContent); // To correctly handle the message content after reading src and dest
        if (messageContent.size() > 0 && messageContent[0] == ' ') { // Remove leading space from message content
            messageContent.erase(0, 1);
        }
        sendMessage(outputFile, src, dest, dist, nextHop, messageContent, INF);
    }

    messageFile.close();
}

void outputForwardingTables(const std::vector<std::vector<int>>& dist, const std::vector<std::vector<int>>& nextHop, std::ofstream& outputFile, int maxNodes, const int INF) {
    for (int node = 1; node <= maxNodes; ++node) {
        for (int dest = 1; dest <= maxNodes; ++dest) {
            if (node == dest) {
                // Output the node itself with a next hop of itself and a cost of 0, ensuring it's in order
                outputFile << dest << " " << dest << " " << 0 << std::endl;
            } else if (dist[node][dest] != INF) {
                int nexthop = nextHop[node][dest]; // Use the actual next hop for the destination
                outputFile << dest << " " << nexthop << " " << dist[node][dest] << std::endl;
            }
        }
        // outputFile << std::endl; // Separate tables for readability when debugging
    }
}

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

void recomputeDistanceVectors(const Graph& graph, std::vector<std::vector<int>>& dist, std::vector<std::vector<int>>& nextHop, int maxNodes) {
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
    // Reset nextHop table
    for (int i = 1; i <= maxNodes; ++i) {
        for (int j = 1; j <= maxNodes; ++j) {
            if (i == j)
                nextHop[i][j] = i; // Next hop to itself is itself
            else if (dist[i][j] != INF)
                nextHop[i][j] = j; // Directly connected nodes
            else
                nextHop[i][j] = -1; // Undefined next hop
        }
    }

    // Apply the DV update logic
    bool updated;
    do {
        updated = false;

        for (int i = 1; i <= maxNodes; ++i) {
            for (const auto& edge : graph[i]) {
                int neighbor = edge.first;
                for (int j = 1; j <= maxNodes; ++j) {
                    // Check for a better path or a tie with a preferable next hop
                    if (dist[i][j] > dist[i][neighbor] + dist[neighbor][j] ||
                        (dist[i][j] == dist[i][neighbor] + dist[neighbor][j] && nextHop[i][j] > nextHop[i][neighbor])) {
                        dist[i][j] = dist[i][neighbor] + dist[neighbor][j];
                        // Update next hop, considering direct connections and ensuring the next hop isn't set to -1 incorrectly
                        nextHop[i][j] = (dist[i][neighbor] == INF) ? -1 : nextHop[i][neighbor];
                        updated = true;
                    }
                }
            }
        }

    } while (updated);
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

    std::ofstream outputFile("output.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Could not open output.txt for writing\n";
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


    std::vector<std::vector<int>> dist(maxNodes+1, std::vector<int>(maxNodes+1, INF));
    std::vector<std::vector<int>> nextHop(maxNodes+1, std::vector<int>(maxNodes+1, -1));
    // do the Distance Vector computing for the first time, before any changes applied
    recomputeDistanceVectors(graph, dist, nextHop, maxNodes);
    // output the forwarding table after computing the distance table
    outputForwardingTables(dist, nextHop, outputFile, maxNodes, INF);
    // handle message forwarding
    handleMessageForwarding(argv[2], dist, nextHop, outputFile, INF);

    // debug the distance table after iteratively update distances until no changes
    if (debug) {
        printDistanceTable(dist, maxNodes, INF);
    }

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
        recomputeDistanceVectors(graph, dist, nextHop, maxNodes);
        // output the forwarding table after computing the distance table
        outputForwardingTables(dist, nextHop, outputFile, maxNodes, INF);
        // handle message forwarding
        handleMessageForwarding(argv[2], dist, nextHop, outputFile, INF);
        std::cout << "Distance vectors recomputed." << std::endl;
        if (debug) {
            printDistanceTable(dist, maxNodes, INF);
        }
    }

    changesFile.close();

    return 0;
}
