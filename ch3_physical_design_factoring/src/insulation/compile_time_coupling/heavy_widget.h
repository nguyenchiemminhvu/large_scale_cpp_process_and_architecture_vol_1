// heavy_widget.h                                                     -*-C++-*-
//
// Demonstrates EXCESSIVE compile-time coupling.
// Book reference: Lakos §3.6-3.7 Avoiding Excessive Link/Compile-Time Dependencies
//
// PROBLEM: This header includes many heavy headers, causing every client
// to pay the compile-time cost of parsing them.

#ifndef INCLUDED_HEAVY_WIDGET
#define INCLUDED_HEAVY_WIDGET

// These are "heavy" headers that take time to parse and introduce
// many transitive includes. In a real system, these might be:
// - <boost/multiprecision/cpp_int.hpp>  (500KB+ header)
// - <opencv2/core.hpp>                  (pulls in hundreds of headers)
// - <windows.h>                         (huge, pollutes namespace)
// Instead we simulate with standard headers that represent the concept:
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

/// A widget that exposes its full implementation details in the header.
///
/// PROBLEM: Every client that includes this header must also parse
/// and compile ALL of the above headers.
///
/// Even clients that only need 'getName()' pay the cost of parsing
/// <unordered_map>, <set>, <chrono>, etc.
class HeavyWidget {
    std::string                           d_name;
    std::vector<int>                      d_data;
    std::unordered_map<std::string, int>  d_lookup;
    std::set<std::string>                 d_tags;
    std::chrono::system_clock::time_point d_created;
    std::map<int, std::string>            d_history;

  public:
    explicit HeavyWidget(const std::string& name);

    std::string name() const;
    void        addData(int value);
    int         dataSize() const;
    void        addTag(const std::string& tag);
    bool        hasTag(const std::string& tag) const;
};

#endif  // INCLUDED_HEAVY_WIDGET
