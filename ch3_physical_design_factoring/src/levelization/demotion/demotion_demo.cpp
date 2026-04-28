// demotion_demo.cpp                                                  -*-C++-*-
//
// Demonstrates the DEMOTION levelization technique.
// Book reference: Lakos §3.5.3 "Demotion" and §3.6
//
// CONCEPT:
//   "Demotion" means moving a piece of functionality from a HIGH-level
//   component DOWN to a lower-level component, so that other components
//   can use it without pulling in unwanted heavy dependencies.
//
// PROBLEM:
//   A utility function like string_hash is logically at Level 1 (no deps),
//   but it's been placed inside a large 'AllUtils' component that also
//   contains Level 5 code (e.g., database connections, report generators).
//   Any component that needs string_hash must link against ALL of AllUtils.
//
// SOLUTION:
//   Demote string_hash to its own Level 1 component.
//   AllUtils can still provide the higher-level things.
//   Components that need only string_hash link only against hash_util.

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// BEFORE DEMOTION (Anti-pattern)
// ============================================================

namespace bad {

// This monolithic utility class mixes Level 1, Level 3, and Level 5 concerns.
// Any user of 'stringHash' must link against the entire thing.
struct AllUtils {
    // Level 1 capability (should be demoted):
    static std::size_t stringHash(const std::string& s) {
        return std::hash<std::string>{}(s);
    }

    // Level 3 capability (depends on Date, etc.):
    static std::string formatDate(int y, int m, int d) {
        return std::to_string(y) + "-" + std::to_string(m) + "-" + std::to_string(d);
    }

    // Level 5 capability (depends on DB, network, etc.):
    // [Not implemented here — represents heavy code that shouldn't be
    //  dragged in just because a client needs stringHash]
    static void saveToDatabase(const std::string& /*key*/, const std::string& /*val*/) {
        // Imagine this pulls in libpqxx, libmysql, etc.
    }
};
// PROBLEM: A Level 1 component that needs only stringHash must depend on
// Level 5 because AllUtils is one monolithic component.

}  // close namespace bad


// ============================================================
// AFTER DEMOTION (Correct approach)
// ============================================================

namespace good {

// Level 1: Hash utilities — demoted to their own component.
// Components at ANY level can use this without pulling in Level 3/5 code.
struct HashUtil {
    static std::size_t stringHash(const std::string& s) {
        return std::hash<std::string>{}(s);
    }

    static std::size_t combineHashes(std::size_t h1, std::size_t h2) {
        // Boost-style hash combine
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

// Level 3: Date utilities — depends on date math, not on DB/network.
struct DateUtil {
    // Uses: Level 1 HashUtil (for date cache keys), Level 2 date types
    static std::string formatDate(int y, int m, int d) {
        return std::to_string(y) + "-" + std::to_string(m) + "-" + std::to_string(d);
    }

    static std::size_t dateHash(int y, int m, int d) {
        auto h1 = HashUtil::stringHash(std::to_string(y));
        auto h2 = HashUtil::stringHash(std::to_string(m));
        auto h3 = HashUtil::stringHash(std::to_string(d));
        return HashUtil::combineHashes(HashUtil::combineHashes(h1, h2), h3);
    }
};

// Level 5: Database utilities — depends on Level 3 stuff
struct DbUtil {
    // Uses: Level 3 DateUtil, Level 1 HashUtil, plus external DB library
    static void saveRecord(const std::string& key, const std::string& val) {
        // In reality, this would link against libsqlite, libpq, etc.
        std::cout << "  [DbUtil] Saving: " << key << " = " << val << "\n";
    }

    // Uses DateUtil to generate a cache key
    static std::string makeCacheKey(const std::string& prefix, int y, int m, int d) {
        return prefix + "_" + DateUtil::formatDate(y, m, d);
    }
};

}  // close namespace good


// ============================================================
// A LEVEL 1 COMPONENT that uses only good::HashUtil
// (would fail to compile/link if it depended on bad::AllUtils
//  and AllUtils had heavy link dependencies)
// ============================================================

struct LevelOneHashUser {
    // This component is Level 1. It can use good::HashUtil (Level 1)
    // but NOT good::DateUtil (Level 3) or good::DbUtil (Level 5).

    static std::size_t hashPair(const std::string& a, const std::string& b) {
        auto ha = good::HashUtil::stringHash(a);
        auto hb = good::HashUtil::stringHash(b);
        return good::HashUtil::combineHashes(ha, hb);
    }
};


// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 3: Demotion Levelization Technique ===\n\n";

    std::cout << "PROBLEM (bad::AllUtils):\n";
    std::cout << "  All functionality is lumped into one component.\n";
    std::cout << "  A Level-1 client needing 'stringHash' must link against\n";
    std::cout << "  all of AllUtils, including Level-5 DB code.\n\n";

    std::cout << "SOLUTION (good:: demoted components):\n";
    std::cout << "  HashUtil  (Level 1) — hash functions only\n";
    std::cout << "  DateUtil  (Level 3) — date formatting, uses HashUtil\n";
    std::cout << "  DbUtil    (Level 5) — database ops, uses DateUtil + HashUtil\n\n";

    // Level 1 usage
    std::cout << "--- Level 1 Usage (only needs HashUtil) ---\n";
    auto h1 = good::HashUtil::stringHash("hello");
    auto h2 = good::HashUtil::stringHash("world");
    auto combined = good::HashUtil::combineHashes(h1, h2);
    std::cout << "  hash('hello') = " << h1 << "\n";
    std::cout << "  hash('world') = " << h2 << "\n";
    std::cout << "  combined = " << combined << "\n\n";

    auto pairHash = LevelOneHashUser::hashPair("key1", "key2");
    std::cout << "  LevelOneHashUser::hashPair = " << pairHash << "\n\n";

    // Level 3 usage
    std::cout << "--- Level 3 Usage (DateUtil uses HashUtil) ---\n";
    std::cout << "  formatDate(2024,3,15) = "
              << good::DateUtil::formatDate(2024, 3, 15) << "\n";
    std::cout << "  dateHash(2024,3,15) = "
              << good::DateUtil::dateHash(2024, 3, 15) << "\n\n";

    // Level 5 usage
    std::cout << "--- Level 5 Usage (DbUtil uses DateUtil and HashUtil) ---\n";
    good::DbUtil::saveRecord("user:alice", "active");
    std::string cacheKey = good::DbUtil::makeCacheKey("report", 2024, 3, 15);
    std::cout << "  Cache key: " << cacheKey << "\n\n";

    std::cout << "--- DEPENDENCY LEVELS ---\n";
    std::cout << "  good::HashUtil  : Level 1\n";
    std::cout << "  good::DateUtil  : Level 3 (uses Level 1)\n";
    std::cout << "  good::DbUtil    : Level 5 (uses Level 3 and 1)\n";
    std::cout << "  LevelOneHashUser: Level 1 (uses Level 1 only)\n\n";

    std::cout << "KEY INSIGHT:\n";
    std::cout << "  LevelOneHashUser links only against HashUtil.\n";
    std::cout << "  It does NOT drag in DateUtil or DbUtil.\n";
    std::cout << "  This is only possible because we DEMOTED hashUtil.\n";

    return 0;
}
