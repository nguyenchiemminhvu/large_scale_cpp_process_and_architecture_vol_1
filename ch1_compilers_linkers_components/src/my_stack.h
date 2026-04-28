// my_stack.h                                                          -*-C++-*-
//
// Component: my_stack
//
// PURPOSE:   Provides a simple stack data structure.
//
// CLASSES:
//   my::Stack<T>  - a LIFO stack container
//
// DESCRIPTION:
//   This component demonstrates the four Component Properties from Lakos Ch1:
//
//   Property 1: This header is SELF-SUFFICIENT — it includes everything needed
//               to compile it standalone (only standard headers here).
//
//   Property 2: The implementation (.cpp) does not expose symbols not declared
//               here.
//
//   Property 3: Clients include ONLY this .h file, never my_stack.cpp.
//
//   Property 4: The Depends-On relation is determined by the #include
//               directives in THIS file (only <vector> and <stdexcept>).
//
// INCLUDE GUARD: INCLUDED_MY_STACK (follows Lakos naming convention)

#ifndef INCLUDED_MY_STACK
#define INCLUDED_MY_STACK

#include <stdexcept>  // std::out_of_range
#include <vector>     // std::vector (used in interface)

namespace my {

/// A simple LIFO stack container.
///
/// This component demonstrates Lakos-style component structure:
/// - A single .h/.cpp pair
/// - Self-sufficient header
/// - No leaking symbols
/// - Clean dependency graph (only depends on std library)
template <typename T>
class Stack {
    std::vector<T> d_data;  // d_ prefix = data member (Lakos convention)

  public:
    // CREATORS

    /// Creates an empty stack.
    Stack() = default;

    /// Creates a stack with initial capacity hint (does not add elements).
    explicit Stack(std::size_t initialCapacity);

    // MANIPULATORS

    /// Pushes a copy of 'value' onto the top of this stack.
    void push(const T& value);

    /// Pushes 'value' (by move) onto the top of this stack.
    void push(T&& value);

    /// Removes the top element from this stack.
    /// Throws std::out_of_range if the stack is empty.
    void pop();

    // ACCESSORS

    /// Returns a const reference to the top element.
    /// Throws std::out_of_range if the stack is empty.
    const T& top() const;

    /// Returns true if this stack has no elements.
    bool empty() const;

    /// Returns the number of elements in this stack.
    std::size_t size() const;
};

// ============================================================
// INLINE DEFINITIONS
// (Template definitions must appear in the header)
// ============================================================

template <typename T>
Stack<T>::Stack(std::size_t initialCapacity)
{
    d_data.reserve(initialCapacity);
}

template <typename T>
void Stack<T>::push(const T& value)
{
    d_data.push_back(value);
}

template <typename T>
void Stack<T>::push(T&& value)
{
    d_data.push_back(std::move(value));
}

template <typename T>
void Stack<T>::pop()
{
    if (d_data.empty()) {
        throw std::out_of_range("my::Stack::pop(): stack is empty");
    }
    d_data.pop_back();
}

template <typename T>
const T& Stack<T>::top() const
{
    if (d_data.empty()) {
        throw std::out_of_range("my::Stack::top(): stack is empty");
    }
    return d_data.back();
}

template <typename T>
bool Stack<T>::empty() const
{
    return d_data.empty();
}

template <typename T>
std::size_t Stack<T>::size() const
{
    return d_data.size();
}

}  // close namespace my

#endif  // INCLUDED_MY_STACK
