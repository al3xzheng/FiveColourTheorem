#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**
 * viz.hpp — Graph Visualizer integration (vector-based)
 *
 * Your C++ flow:
 *   1. viz::loadGraph()    — reads graph from stdin (sent by server on /load)
 *   2. viz::printState()   — call immediately after setup (server reads this on /load)
 *   3. In each loop iter:  viz::waitForStep() then viz::printState()
 *
 * Every function that uses the graph must take:
 *   std::vector<std::vector<int>>& graph   — n rows, n+1 cols (last col = degree)
 *   int n                                  — number of nodes (NOT a global constant)
 */

namespace viz {

// Blocks until server sends a newline (user pressed STEP)
inline void waitForStep() {
    std::string line;
    std::getline(std::cin, line);
}

// Reads graph JSON from stdin, populates graph (n x n+1), colors (size n), sets n
inline bool loadGraph(
    std::vector<std::vector<int>>& graph,
    std::vector<int>&              colors,
    int&                           n
) {
    std::string line;
    if (!std::getline(std::cin, line) || line.empty()) {
        std::cerr << "[viz] loadGraph: no data on stdin." << std::endl;
        return false;
    }

    auto readInt = [&](size_t& p) -> int {
        while (p < line.size() && line[p] != '-' && (line[p] < '0' || line[p] > '9')) p++;
        if (p >= line.size()) return 0;
        bool neg = line[p] == '-';
        if (neg) p++;
        int v = 0;
        while (p < line.size() && line[p] >= '0' && line[p] <= '9')
            v = v * 10 + (line[p++] - '0');
        return neg ? -v : v;
    };

    // Read n
    size_t pos = line.find("\"n\":");
    if (pos == std::string::npos) { std::cerr << "[viz] missing n" << std::endl; return false; }
    pos += 4;
    n = readInt(pos);
    if (n <= 0) { std::cerr << "[viz] invalid n=" << n << std::endl; return false; }

    // Allocate: n rows, n+1 cols (last col = degree)
    graph.assign(n, std::vector<int>(n + 1, 0));
    colors.assign(n, 0);

    // Read n x n matrix
    pos = line.find("\"matrix\"");
    if (pos == std::string::npos) { std::cerr << "[viz] missing matrix" << std::endl; return false; }
    // skip past "matrix" key and find the opening [
    while (pos < line.size() && line[pos] != '[') pos++;
    pos++; // skip outer '['

    for (int i = 0; i < n; i++) {
        while (pos < line.size() && line[pos] != '[') pos++;
        pos++; // skip '['
        for (int j = 0; j < n; j++)
            graph[i][j] = readInt(pos);
    }

    // Compute degree column (index n)
    for (int i = 0; i < n; i++) {
        int deg = 0;
        for (int j = 0; j < n; j++) deg += graph[i][j];
        graph[i][n] = deg;
    }

    std::cerr << "[viz] loadGraph ok: n=" << n << std::endl;
    return true;
}

// Emits current state as one JSON line to stdout
inline void printState(
    const std::vector<std::vector<int>>& graph,
    const std::vector<int>&              colors,
    int                                  iteration,
    int                                  n,
    const std::string&                   status  = "running",
    const std::string&                   message = ""
) {
    std::ostringstream o;
    o << "{\"iteration\":" << iteration
      << ",\"n\":"         << n
      << ",\"status\":\""  << status << "\""
      << ",\"message\":\"" << (message.empty()
            ? "Iteration " + std::to_string(iteration) : message) << "\""
      << ",\"matrix\":[";
    for (int i = 0; i < n; i++) {
        o << "[";
        for (int j = 0; j <= n; j++) { o << graph[i][j]; if (j < n) o << ","; }
        o << "]"; if (i < n - 1) o << ",";
    }
    o << "],\"colors\":[";
    for (int i = 0; i < n; i++) { o << colors[i]; if (i < n - 1) o << ","; }
    o << "]}";
    std::cout << o.str() << "\n";
    std::cout.flush();
}

} // namespace viz
