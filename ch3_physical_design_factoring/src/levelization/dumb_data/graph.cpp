// graph.cpp
#ifndef INCLUDED_GRAPH
#include <graph.h>
#endif

#include <iostream>
#include <algorithm>

int Graph::addNode(const std::string& label, Node::Type type, int weight)
{
    int id = static_cast<int>(d_nodes.size());
    d_nodes.emplace_back(id, label, type, weight);
    return id;
}

void Graph::addEdge(int fromId, int toId, int weight)
{
    d_edges.push_back({fromId, toId, weight});
}

std::vector<const Node*> Graph::sources() const
{
    std::vector<const Node*> result;
    for (const auto& n : d_nodes) {
        if (n.d_type == Node::Type::k_SOURCE) {
            result.push_back(&n);
        }
    }
    return result;
}

std::vector<const Node*> Graph::sinks() const
{
    std::vector<const Node*> result;
    for (const auto& n : d_nodes) {
        if (n.d_type == Node::Type::k_SINK) {
            result.push_back(&n);
        }
    }
    return result;
}

std::vector<const Node*> Graph::bridges() const
{
    std::vector<const Node*> result;
    for (const auto& n : d_nodes) {
        if (n.d_type == Node::Type::k_BRIDGE) {
            result.push_back(&n);
        }
    }
    return result;
}

const Node* Graph::findNode(int id) const
{
    for (const auto& n : d_nodes) {
        if (n.d_id == id) return &n;
    }
    return nullptr;
}

int Graph::nodeCount() const { return static_cast<int>(d_nodes.size()); }
int Graph::edgeCount() const { return static_cast<int>(d_edges.size()); }

void Graph::print() const
{
    std::cout << "  Graph: " << d_nodes.size() << " nodes, "
              << d_edges.size() << " edges\n";
    for (const auto& n : d_nodes) {
        std::string typeStr;
        switch (n.d_type) {
            case Node::Type::k_NORMAL: typeStr = "NORMAL"; break;
            case Node::Type::k_SOURCE: typeStr = "SOURCE"; break;
            case Node::Type::k_SINK:   typeStr = "SINK";   break;
            case Node::Type::k_BRIDGE: typeStr = "BRIDGE"; break;
        }
        std::cout << "    Node[" << n.d_id << "] \"" << n.d_label
                  << "\" type=" << typeStr
                  << " weight=" << n.d_weight << "\n";
    }
    for (const auto& e : d_edges) {
        std::cout << "    Edge: " << e.d_fromId << " → " << e.d_toId
                  << " (w=" << e.d_weight << ")\n";
    }
}
