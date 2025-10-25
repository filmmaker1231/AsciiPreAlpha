#include "Pathfinding.h"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <limits>

struct Node {
    int x, y;
    float g, f;
    Node* parent;
    Node(int x, int y, float g, float f, Node* parent)
        : x(x), y(y), g(g), f(f), parent(parent) {}
};

struct NodeCmp {
    bool operator()(const Node* a, const Node* b) const {
        return a->f > b->f;
    }
};

std::vector<std::pair<int, int>> aStarFindPath(
    int startX, int startY,
    int goalX, int goalY,
    const CellGrid& grid
) {
    int w = grid.getWidthInCells();
    int h = grid.getHeightInCells();

    auto heuristic = [](int x1, int y1, int x2, int y2) {
        return static_cast<float>(std::abs(x1 - x2) + std::abs(y1 - y2));
    };

    std::priority_queue<Node*, std::vector<Node*>, NodeCmp> open;
    std::unordered_map<int, Node*> allNodes;
    auto hash = [w](int x, int y) { return y * w + x; };

    Node* start = new Node(startX, startY, 0.0f, heuristic(startX, startY, goalX, goalY), nullptr);
    open.push(start);
    allNodes[hash(startX, startY)] = start;

    std::vector<std::pair<int, int>> path;
    bool found = false;

    while (!open.empty()) {
        Node* current = open.top();
        open.pop();

        if (current->x == goalX && current->y == goalY) {
            // Reconstruct path
            while (current) {
                path.emplace_back(current->x, current->y);
                current = current->parent;
            }
            std::reverse(path.begin(), path.end());
            found = true;
            break;
        }

        // 4 directions
        const int dx[4] = {1, -1, 0, 0};
        const int dy[4] = {0, 0, 1, -1};
        for (int dir = 0; dir < 4; ++dir) {
            int nx = current->x + dx[dir];
            int ny = current->y + dy[dir];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
            if (!grid.isCellWalkable(nx, ny)) continue;

            float ng = current->g + 1.0f;
            int nhash = hash(nx, ny);
            if (!allNodes.count(nhash) || ng < allNodes[nhash]->g) {
                Node* neighbor = new Node(nx, ny, ng, ng + heuristic(nx, ny, goalX, goalY), current);
                open.push(neighbor);
                allNodes[nhash] = neighbor;
            }
        }
    }

    // Clean up
    for (auto& kv : allNodes) delete kv.second;
    if (!found) path.clear();
    return path;
}
