#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <limits>
#include <algorithm>
using namespace std;

// Split helper
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) tokens.push_back(token);
    return tokens;
}

// Build the graph from CSV data
map<string, map<string, pair<float, float>>> buildGraph(const string& filename) {
    map<string, map<string, pair<float, float>>> graph;
    ifstream file(filename);
    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        auto fields = split(line, ',');
        if (fields.size() < 5) continue;
        string n1 = fields[0], n2 = fields[1];
        float distance = stof(fields[2]);
        float time = stof(fields[3]);
        float traffic = stof(fields[4]);
        float multiplier = 1.0;

        // Apply traffic multiplier based on the traffic condition
        if (traffic == 2) multiplier = 2.0;
        else if (traffic == 3) multiplier = 2.5;  // Adjust for high traffic

        // Adjust the time based on traffic multiplier
        float adjustedTime = time * multiplier;

        // Add edges for both directions
        graph[n1][n2] = {adjustedTime, distance};
        graph[n2][n1] = {adjustedTime, distance};
    }
    return graph;
}

// Dijkstra algorithm to minimize adjusted time
vector<string> dijkstra(
    const map<string, map<string, pair<float, float>>>& graph,
    const string& start, const string& end,
    float& totalTime, float& totalDist
) {
    map<string, float> minTime;
    map<string, float> totalDistance;
    map<string, string> prev;
    priority_queue<pair<float, string>, vector<pair<float, string>>, greater<>> pq;

    // Initialize all nodes with infinite time
    for (auto& entry : graph) minTime[entry.first] = numeric_limits<float>::infinity();
    minTime[start] = 0.0;
    totalDistance[start] = 0.0;
    pq.push({0.0, start});

    while (!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        float curTime = top.first;
        string node = top.second;

        if (node == end) break;

        for (auto it = graph.at(node).begin(); it != graph.at(node).end(); ++it) {
            string neighbor = it->first;
            float edgeTime = it->second.first;
            float edgeDist = it->second.second;
            float newTime = curTime + edgeTime;

            if (newTime < minTime[neighbor]) {
                minTime[neighbor] = newTime;
                totalDistance[neighbor] = totalDistance[node] + edgeDist;
                prev[neighbor] = node;
                pq.push({newTime, neighbor});
            }
        }
    }

    // Reconstruct the path from start to end
    vector<string> path;
    string at = end;
    if (prev.find(at) == prev.end() && start != end) return {}; // No valid path found
    while (at != start) {
        path.push_back(at);
        at = prev[at];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    totalTime = minTime[end];
    totalDist = totalDistance[end];
    return path;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <start_location> <end_location>" << endl;
        return 1;
    }

    string start = argv[1]; // Get start location from command line argument
    string end = argv[2];   // Get end location from command line argument

    // Build the graph from the CSV file
    string filename = "roads.csv"; // Path to the CSV file
    auto graph = buildGraph(filename);

    // Find the optimal path using the unified cost
    float totalDistance = 0, totalTime = 0;
    vector<string> path = dijkstra(graph, start, end, totalDistance, totalTime);

    // Output the result
    if (path.empty()) {
        cout << "No valid path found." << endl;
    } else {
        cout << "Path: ";
        for (int i = 0; i < (int)path.size(); ++i) {
            cout << path[i];
            if (i < (int)path.size() - 1) 
            cout << " -> ";
        }
        cout << endl<<"Distance: " << totalDistance << endl;
        cout << "Total Time: " << totalTime << endl;
    }

    return 0;
}

