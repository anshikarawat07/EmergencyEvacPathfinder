#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <limits>
#include <algorithm>
using namespace std;

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Graph loader
map<string, map<string, pair<float, float>>> buildGraph(const string& filename) {
    map<string, map<string, pair<float, float>>> graph;
    ifstream file(filename);
    string line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        auto fields = split(line, ',');
        if (fields.size() < 5) continue;
        string n1 = fields[0], n2 = fields[1];
        float dist = stof(fields[2]);
        float time = stof(fields[3]);
        float traffic = stof(fields[4]);
        float multiplier = 1.0;
        if (traffic == 2) multiplier = 2.0;
        else if (traffic == 3) multiplier = 2.5;
        float adjustedTime = time * multiplier;

        graph[n1][n2] = make_pair(adjustedTime, dist);
        graph[n2][n1] = make_pair(adjustedTime, dist);
    }
    return graph;
}

// Hospital list loader
vector<string> loadHospitals(const string& filename) {
    vector<string> hospitals;
    ifstream file(filename);
    string line;
    getline(file, line); // skip header
    while (getline(file, line)) {
        auto fields = split(line, ',');
        if (!fields.empty()) {
            hospitals.push_back(fields[0]);
        }
    }
    return hospitals;
}

// Dijkstra algorithm
vector<string> dijkstra(
    const map<string, map<string, pair<float, float>>>& graph,
    const string& start, const string& end,
    float& totalTime, float& totalDist
) {
    map<string, float> minTime;
    map<string, float> dist;
    map<string, string> prev;
    priority_queue<pair<float, string>, vector<pair<float, string>>, greater<pair<float, string>>> pq;

    for (map<string, map<string, pair<float, float>>> it = graph.begin(); it != graph.end(); ++it) {
        minTime[it->first] = numeric_limits<float>::infinity();
    }
    minTime[start] = 0.0;
    distSoFar[start] = 0.0;
    pq.push(make_pair(0.0, start));

    while (!pq.empty()) {
        float curTime = pq.top().first;
        string node = pq.top().second;
        pq.pop();

        if (node == end) break;
        const map<string, pair<float, float>>& neighbors = graph.at(node);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
            string neigh = it->first;
            float timeCost = it->second.first;
            float newTime = curTime + timeCost;
            if (newTime < minTime[neigh]|| (newTime == minTime[neighbor] && totalDistance[node] + edgeDist < totalDistance[neighbor])) {
                minTime[neigh] = newTime;
                dist[neigh] = distSoFar[node] + it->second.second;
                prev[neigh] = node;
                pq.push(make_pair(newTime, neigh));
            }
        }
    }

    vector<string> path;
    string at = end;
    if (start != end && prev.find(at) == prev.end()) return {};
    while (at != start) {
        path.push_back(at);
        at = prev[at];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    totalTime = minTime[end];
    totalDist = distSoFar[end];
    return path;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage:\n";
        cerr << argv[0] << " 1 start end\n";
        cerr << argv[0] << " 2 start\n";
        return 1;
    }

    int mode = atoi(argv[1]);
    auto graph = buildGraph("roads.csv");

    if (mode == 1 && argc == 4) {
        string start = argv[2], end = argv[3];
        float t = 0, d = 0;
        auto path = dijkstra(graph, start, end, t, d);
        if (path.empty()) {
            cout << "Path: \nNo valid path found.\n";
        } else {
            cout << "Path: ";
            for (size_t i = 0; i < path.size(); ++i) {
                cout << path[i];
                if (i < path.size() - 1) cout << " -> ";
            }
            cout << "\nDistance: " << d << "\nTotal Time: " << t << " minutes\n";
        }
    } else if (mode == 2 && argc == 3) {
        string start = argv[2];
        string  hospitals = loadHospitals("safezone.csv");

        float bestTime = numeric_limits<float>::infinity(), bestDist = 0;
        vector<string> bestPath;
        string nearestHospital;

        for (int i = 0; i < hospitals.size(); ++i) {
            float t = 0, d = 0;
            auto path = dijkstra(graph, start, hospitals[i], t, d);
            if (!path.empty() && t < bestTime) {
                bestTime = t;
                bestDist = d;
                bestPath = path;
                nearestHospital = hospitals[i];
            }
        }

        if (bestPath.empty()) {
            cout << "Path: \nNo hospital path found.\n";
        } else {
            cout << "Nearest Hospital: " << nearestHospital << endl;
            cout << "Path: ";
            for (int i = 0; i < bestPath.size(); ++i) {
                cout << bestPath[i];
                if (i < bestPath.size() - 1) cout << " -> ";
            }
            cout << "\nDistance: " << bestDist << "\nTotal Time: " << bestTime << " minutes\n";
        }
    } else {
        cerr << "Invalid input format.\n";
        return 1;
    }

    return 0;
}
