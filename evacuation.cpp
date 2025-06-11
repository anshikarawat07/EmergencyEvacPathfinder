#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <limits>
#include <algorithm>
#include <cstdlib>
using namespace std;

// Utility: Split string by delimiter
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Build graph from CSV
map<string, map<string, pair<float, float>>> buildGraph(const string& filename) {
    map<string, map<string, pair<float, float>>> graph;
    ifstream file(filename);
    string line;
    getline(file, line); // skip header
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

// Load hospital/safezone names
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


void dfsPaths(const string& current, const string& start,
              const map<string, vector<string>>& prev,
              vector<string>& path, vector<vector<string>>& allPaths) {
    path.push_back(current);
    if (current == start) {
        vector<string> validPath = path;
        reverse(validPath.begin(), validPath.end());
        allPaths.push_back(validPath);
    } else {
        map<string, vector<string>>::const_iterator it = prev.find(current);
        if (it != prev.end()) {
            for (size_t i = 0; i < it->second.size(); ++i) {
                dfsPaths(it->second[i], start, prev, path, allPaths);
            }
        }
    }
    path.pop_back();
}
vector<vector<string>> dijkstraAllPaths(
    const map<string, map<string, pair<float, float>>>& graph,
    const string& start, const string& end,
    float& totalTime, float& totalDist
) {
    map<string, float> minTime;
    map<string, float> dist;
    map<string, vector<string>> prev;
    priority_queue<pair<float, string>, vector<pair<float, string>>, greater<pair<float, string>>> pq;

    for (map<string, map<string, pair<float, float>>>::const_iterator it = graph.begin(); it != graph.end(); ++it) {
        minTime[it->first] = numeric_limits<float>::infinity();
        dist[it->first] = numeric_limits<float>::infinity();
    }

    minTime[start] = 0.0;
    dist[start] = 0.0;
    pq.push(make_pair(0.0, start));

    while (!pq.empty()) {
        pair<float, string> top = pq.top();
        pq.pop();

        float curTime = top.first;
        string node = top.second;

        if (node == end) break;

        const map<string, pair<float, float>>& neighbors = graph.at(node);
        for (map<string, pair<float, float>>::const_iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
            string neigh = it->first;
            float timeCost = it->second.first;
            float newTime = curTime + timeCost;
            float newDist = dist[node] + it->second.second;

            if (newTime < minTime[neigh]) {
                minTime[neigh] = newTime;
                dist[neigh] = newDist;
                prev[neigh] = vector<string>(1, node);
                pq.push(make_pair(newTime, neigh));
            } else if (newTime == minTime[neigh] && newDist == dist[neigh]) {
                prev[neigh].push_back(node);
            }
        }
    }

    totalTime = minTime[end];
    totalDist = dist[end];

    vector<vector<string>> allPaths;
    if (prev.find(end) != prev.end()) {
        vector<string> temp;
        dfsPaths(end, start, prev, temp, allPaths);
    }
    return allPaths;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage:\n";
        cerr << argv[0] << " 1 start end\n";
        cerr << argv[0] << " 2 start\n";
        return 1;
    }

    int mode = atoi(argv[1]);
    map<string, map<string, pair<float, float>>> graph = buildGraph("roads.csv");

    if (mode == 1 && argc == 4) {
        string start = argv[2], end = argv[3];
        float t = 0, d = 0;
        vector<vector<string>> allPaths = dijkstraAllPaths(graph, start, end, t, d);

        if (allPaths.empty()) {
            cout << "Path: \nNo valid path found.\n";
        } else {
            cout << "Fastest evacuation time: " << t << " minutes\n";
            cout << "Shortest distance: " << d << " km\n";
            cout << "All optimal paths:\n";
            for (size_t i = 0; i < allPaths.size(); ++i) {
                for (size_t j = 0; j < allPaths[i].size(); ++j) {
                    cout << allPaths[i][j];
                    if (j < allPaths[i].size() - 1) cout << " -> ";
                }
                cout << '\n';
            }
        }
    } else if (mode == 2 && argc == 3) {
        string start = argv[2];
        vector<string> hospitals = loadHospitals("safezone.csv");

        float bestTime = numeric_limits<float>::infinity(), bestDist = 0;
        vector<string> bestPath;
        string nearestHospital;

        for (size_t i = 0; i < hospitals.size(); ++i) {
            float t = 0, d = 0;
            vector<vector<string>> paths = dijkstraAllPaths(graph, start, hospitals[i], t, d);
            if (!paths.empty() && t < bestTime) {
                bestTime = t;
                bestDist = d;
                bestPath = paths[0];
                nearestHospital = hospitals[i];
            }
        }

        if (bestPath.empty()) {
            cout << "Path: \nNo hospital path found.\n";
        } else {
            cout << "Nearest Hospital: " << nearestHospital << endl;
            cout << "Path: ";
            for (size_t i = 0; i < bestPath.size(); ++i) {
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
