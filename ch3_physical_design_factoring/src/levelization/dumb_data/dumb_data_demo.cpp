// dumb_data_demo.cpp
#ifndef INCLUDED_GRAPH
#include <graph.h>
#endif

#include <cassert>
#include <iostream>

int main()
{
    std::cout << "=== Chapter 3: Dumb Data Levelization Demo ===\n\n";

    std::cout << "CONCEPT:\n";
    std::cout << "  Node is a 'dumb' data structure — it holds type tags (integers)\n";
    std::cout << "  without any semantic knowledge of Graph's algorithms.\n";
    std::cout << "  Graph interprets these tags to implement graph logic.\n";
    std::cout << "  Result: Graph → Node (acyclic), no Node → Graph dependency.\n\n";

    std::cout << "DEPENDENCY GRAPH:\n";
    std::cout << "  Graph  (Level 2) — knows about Node, interprets type tags\n";
    std::cout << "    ↓ uses\n";
    std::cout << "  Node   (Level 1) — just holds data, no knowledge of Graph\n\n";

    Graph g;
    int src  = g.addNode("Start",    Node::Type::k_SOURCE);
    int a    = g.addNode("Process A", Node::Type::k_NORMAL, 5);
    int b    = g.addNode("Process B", Node::Type::k_NORMAL, 3);
    int brdg = g.addNode("Merge",    Node::Type::k_BRIDGE, 1);
    int sink = g.addNode("End",      Node::Type::k_SINK);

    g.addEdge(src, a, 2);
    g.addEdge(src, b, 1);
    g.addEdge(a, brdg, 2);
    g.addEdge(b, brdg, 3);
    g.addEdge(brdg, sink, 1);

    std::cout << "--- Graph Contents ---\n";
    g.print();

    std::cout << "\n--- Graph Analysis (Graph interprets 'dumb' node types) ---\n";

    auto srcs = g.sources();
    std::cout << "Sources (" << srcs.size() << "):\n";
    for (const auto* n : srcs) {
        std::cout << "  [" << n->d_id << "] " << n->d_label << "\n";
    }

    auto snks = g.sinks();
    std::cout << "Sinks (" << snks.size() << "):\n";
    for (const auto* n : snks) {
        std::cout << "  [" << n->d_id << "] " << n->d_label << "\n";
    }

    auto brgs = g.bridges();
    std::cout << "Bridges (" << brgs.size() << "):\n";
    for (const auto* n : brgs) {
        std::cout << "  [" << n->d_id << "] " << n->d_label << "\n";
    }

    std::cout << "\n--- Key Takeaways ---\n";
    std::cout << "1. 'Dumb' data holds type tags without interpreting them\n";
    std::cout << "2. Higher-level components interpret the data (switch/if on type)\n";
    std::cout << "3. Node can be tested independently of Graph\n";
    std::cout << "4. Node can be used in other contexts that interpret types differently\n";
    std::cout << "5. This is similar to the 'data callback' pattern (§3.5.6)\n";

    assert(g.nodeCount() == 5);
    assert(g.edgeCount() == 5);
    assert(srcs.size() == 1);
    assert(snks.size() == 1);

    return 0;
}
