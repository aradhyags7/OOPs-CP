#include "Network.hpp"
#include <queue>
#include <limits>

int Network::addNode(std::unique_ptr<Node> node) {
    if (!node) return -1;
    int id = node->getId();
    nodeMap[id] = node.get();
    nodes.push_back(std::move(node));
    return id;
}

int Network::addLink(std::unique_ptr<Link> link) {
    if (!link) return -1;
    int id = link->getId();
    linkMap[id] = link.get();
    links.push_back(std::move(link));
    return id;
}

Node* Network::getNode(int nodeId) {
    auto it = nodeMap.find(nodeId);
    return (it != nodeMap.end()) ? it->second : nullptr;
}

const Node* Network::getNode(int nodeId) const {
    auto it = nodeMap.find(nodeId);
    return (it != nodeMap.end()) ? it->second : nullptr;
}

Link* Network::getLink(int linkId) {
    auto it = linkMap.find(linkId);
    return (it != linkMap.end()) ? it->second : nullptr;
}

const Link* Network::getLink(int linkId) const {
    auto it = linkMap.find(linkId);
    return (it != linkMap.end()) ? it->second : nullptr;
}

Link* Network::findLinkBetween(int srcNodeId, int dstNodeId) {
    for (auto& link : links) {
        if (link->getFromNodeId() == srcNodeId && link->getToNodeId() == dstNodeId) {
            return link.get();
        }
    }
    return nullptr;
}

std::vector<Link*> Network::getOutgoingLinks(int nodeId) {
    std::vector<Link*> outLinks;
    for (auto& link : links) {
        if (link->getFromNodeId() == nodeId) {
            outLinks.push_back(link.get());
        }
    }
    return outLinks;
}

void Network::updateRoutingTables() {
    // Run Dijkstra's shortest path algorithm for each Router node
    for (auto& nodePtr : nodes) {
        if (nodePtr->getType() != NodeType::ROUTER) continue;

        RouterNode* router = static_cast<RouterNode*>(nodePtr.get());
        int srcId = router->getId();

        // Dijkstra algorithm initialization
        std::unordered_map<int, double> dist;
        std::unordered_map<int, Link*> firstHopLink;

        for (auto& n : nodes) {
            dist[n->getId()] = std::numeric_limits<double>::infinity();
        }
        dist[srcId] = 0.0;

        // Min-priority queue storing pairs of (distance, nodeId)
        using DistNodePair = std::pair<double, int>;
        std::priority_queue<DistNodePair, std::vector<DistNodePair>, std::greater<DistNodePair>> pq;
        pq.push({0.0, srcId});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d > dist[u]) continue;

            for (Link* link : getOutgoingLinks(u)) {
                if (!link->isActive()) continue;

                int v = link->getToNodeId();
                double weight = link->getPropagationDelaySec();

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    // Keep track of the very first hop link from source router
                    firstHopLink[v] = (u == srcId) ? link : firstHopLink[u];
                    pq.push({dist[v], v});
                }
            }
        }

        // Populate routing table
        for (auto& n : nodes) {
            int destId = n->getId();
            if (destId != srcId && firstHopLink.count(destId) > 0 && firstHopLink[destId] != nullptr) {
                router->addRoute(destId, firstHopLink[destId]->getId());
            }
        }
    }
}

void Network::clear() {
    nodes.clear();
    links.clear();
    nodeMap.clear();
    linkMap.clear();
}
