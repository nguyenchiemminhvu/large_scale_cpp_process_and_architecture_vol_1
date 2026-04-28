// node.h                                                             -*-C++-*-
//
// Demonstrates the DUMB DATA levelization technique.
// Book reference: Lakos §3.5.5 "Dumb Data"
//
// CONCEPT:
//   A "dumb" data structure holds raw data without semantic knowledge
//   about how that data will be used. Higher-level components interpret
//   the data.
//
// PROBLEM WITHOUT DUMB DATA:
//   Node needs to know about Graph to check connectivity.
//   Graph needs to know about Node's type to process it.
//   This creates a cycle: Node ←→ Graph
//
// SOLUTION WITH DUMB DATA:
//   Node is "dumb" — it just holds data (type tag, value).
//   Graph knows about Node and interprets its data.
//   No cycle: Graph → Node (one direction only)

#ifndef INCLUDED_NODE
#define INCLUDED_NODE

#include <string>

/// A "dumb" node: just holds data, makes no semantic decisions.
///
/// The 'type' field is an integer tag. Node has no knowledge of
/// what the types mean or how they will be processed.
struct Node {
    // Type constants (known to both Node and its users)
    enum class Type {
        k_NORMAL  = 0,
        k_SOURCE  = 1,  // entry point
        k_SINK    = 2,  // exit point
        k_BRIDGE  = 3,  // connects disjoint subgraphs
    };

    Type        d_type;
    int         d_id;
    std::string d_label;
    int         d_weight;

    // Constructors
    Node();
    Node(int id, const std::string& label, Type type = Type::k_NORMAL, int weight = 0);
};

#endif  // INCLUDED_NODE
