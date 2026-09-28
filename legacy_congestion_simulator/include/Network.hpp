#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "Node.hpp"
#include "Link.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

class Network {
private:
    std::vector<std::unique_ptr<Node>> nodes;
    std::vector<std::unique_ptr<Link>> links;

    // Fast lookup maps
    std::unordered_map<int, Node*> nodeMap;
    std::unordered_map<int, Link*> linkMap;

public:
    Network() = default;

    int addNode(std::unique_ptr<Node> node);
    int addLink(std::unique_ptr<Link> link);

    Node* getNode(int nodeId);
    const Node* getNode(int nodeId) const;

    Link* getLink(int linkId);
    const Link* getLink(int linkId) const;

    Link* findLinkBetween(int srcNodeId, int dstNodeId);
    std::vector<Link*> getOutgoingLinks(int nodeId);

    // Compute shortest path routing using Dijkstra's algorithm and populate Router routing tables
    void updateRoutingTables();

    const std::vector<std::unique_ptr<Node>>& getNodes() const { return nodes; }
    const std::vector<std::unique_ptr<Link>>& getLinks() const { return links; }

    void clear();
};

#endif // NETWORK_HPP
