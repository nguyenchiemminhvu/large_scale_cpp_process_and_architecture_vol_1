// graph.h                                                            -*-C++-*-
//
// Graph that uses "dumb" Nodes. Graph interprets node data;
// Node knows nothing about Graph. No cycle!

#ifndef INCLUDED_GRAPH
#define INCLUDED_GRAPH

#ifndef INCLUDED_NODE
#include <node.h>
#endif

#include <string>
#include <vector>

/// A directed graph that stores dumb Node objects.
///
/// DEPENDENCY: Graph → Node (acyclic — Node knows nothing about Graph)
///
/// Graph interprets the 'type' tag in Node to implement graph-specific logic
/// (find sources, find sinks, etc.) without Node needing any knowledge of Graph.
class Graph {
    struct Edge {
        int d_fromId;
        int d_toId;
        int d_weight;
    };

    std::vector<Node> d_nodes;
    std::vector<Edge> d_edges;

  public:
    // MANIPULATORS

    /// Adds a node to the graph. Returns the node's ID.
    int addNode(const std::string& label,
                Node::Type type = Node::Type::k_NORMAL,
                int weight = 0);

    /// Adds a directed edge from 'fromId' to 'toId'.
    void addEdge(int fromId, int toId, int weight = 1);

    // ACCESSORS

    /// Returns all source nodes (type == k_SOURCE).
    /// Graph interprets the 'dumb' type tag — Node doesn't know about this.
    std::vector<const Node*> sources() const;

    /// Returns all sink nodes (type == k_SINK).
    std::vector<const Node*> sinks() const;

    /// Returns all bridge nodes.
    std::vector<const Node*> bridges() const;

    /// Returns node by ID, or nullptr if not found.
    const Node* findNode(int id) const;

    /// Returns the number of nodes.
    int nodeCount() const;

    /// Returns the number of edges.
    int edgeCount() const;

    /// Prints a human-readable representation.
    void print() const;
};

#endif  // INCLUDED_GRAPH
