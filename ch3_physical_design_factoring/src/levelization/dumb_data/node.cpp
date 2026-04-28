// node.cpp
#ifndef INCLUDED_NODE
#include <node.h>
#endif

Node::Node()
: d_type(Type::k_NORMAL)
, d_id(0)
, d_label("")
, d_weight(0)
{
}

Node::Node(int id, const std::string& label, Type type, int weight)
: d_type(type)
, d_id(id)
, d_label(label)
, d_weight(weight)
{
}
