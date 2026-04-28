// hierarchical_reuse_demo.cpp
//
// Demonstrates hierarchical reuse and the "text partitioning optimization"
// example from Chapter 0, sections 0.4 and 0.8 of Lakos's Large-Scale C++.
//
// The problem: given a list of words, find the optimal way to partition
// them across multiple columns to minimize total lines used.
//
// Lakos uses this example to show how component-based decomposition allows
// reuse at multiple levels. We implement:
//
//   Level 1: word_scorer   - scores a single line assignment (no deps)
//   Level 2: line_packer   - packs words into lines (uses word_scorer)
//   Level 3: col_optimizer - finds optimal column width (uses line_packer)
//
// Book reference: Chapter 0, §0.8 "Quantifying Hierarchical Reuse"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <climits>

// ============================================================
// LEVEL 1: word_scorer — purely functional, zero dependencies
// Counts how many lines are needed for a given set of word-lengths
// and a given line width.
// ============================================================

namespace word_scorer {

/// Returns the number of lines required to fit all words (given by their
/// character lengths) into lines of at most 'lineWidth' characters,
/// separating adjacent words by one space.
/// Returns INT_MAX if any single word exceeds lineWidth.
int lineCount(const std::vector<int>& wordLengths, int lineWidth)
{
    if (lineWidth <= 0) return INT_MAX;

    int lines = 1;
    int usedOnCurrentLine = 0;

    for (int len : wordLengths) {
        if (len > lineWidth) return INT_MAX;  // word too long
        if (usedOnCurrentLine == 0) {
            usedOnCurrentLine = len;
        } else if (usedOnCurrentLine + 1 + len <= lineWidth) {
            usedOnCurrentLine += 1 + len;
        } else {
            ++lines;
            usedOnCurrentLine = len;
        }
    }
    return lines;
}

/// Returns the "wasted space" (trailing spaces on each line) for a given
/// arrangement. Lower wasted space = better layout quality.
int wastedSpace(const std::vector<int>& wordLengths, int lineWidth)
{
    if (lineWidth <= 0) return INT_MAX;
    int waste = 0;
    int usedOnCurrentLine = 0;

    for (int len : wordLengths) {
        if (len > lineWidth) return INT_MAX;
        if (usedOnCurrentLine == 0) {
            usedOnCurrentLine = len;
        } else if (usedOnCurrentLine + 1 + len <= lineWidth) {
            usedOnCurrentLine += 1 + len;
        } else {
            waste += lineWidth - usedOnCurrentLine;
            usedOnCurrentLine = len;
        }
    }
    waste += lineWidth - usedOnCurrentLine;
    return waste;
}

}  // close namespace word_scorer


// ============================================================
// LEVEL 2: line_packer — uses word_scorer
// Given a list of words and a target line width, packs words into lines
// and returns the actual formatted lines.
// ============================================================

namespace line_packer {

/// Returns a vector of lines (each line is a string) that packs
/// the given words into at most 'lineWidth' characters per line.
std::vector<std::string> pack(const std::vector<std::string>& words,
                               int lineWidth)
{
    std::vector<std::string> lines;
    std::string currentLine;

    for (const auto& word : words) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if (static_cast<int>(currentLine.size()) + 1 +
                   static_cast<int>(word.size()) <= lineWidth) {
            currentLine += " " + word;
        } else {
            lines.push_back(currentLine);
            currentLine = word;
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}

/// Returns the word lengths from a sentence.
std::vector<int> getLengths(const std::vector<std::string>& words)
{
    std::vector<int> lengths;
    lengths.reserve(words.size());
    for (const auto& w : words) {
        lengths.push_back(static_cast<int>(w.size()));
    }
    return lengths;
}

}  // close namespace line_packer


// ============================================================
// LEVEL 3: col_optimizer — uses word_scorer and line_packer
// Finds the optimal column width that minimizes total lines,
// using the scoring function from word_scorer.
// ============================================================

namespace col_optimizer {

struct Result {
    int  optimalWidth;   // best column width found
    int  lineCount;      // number of lines at optimal width
    int  wastedSpace;    // wasted space at optimal width
};

/// Finds the column width in [minWidth, maxWidth] that minimizes
/// total line count, breaking ties by minimizing wasted space.
Result findOptimalWidth(const std::vector<std::string>& words,
                         int minWidth,
                         int maxWidth)
{
    auto lengths = line_packer::getLengths(words);

    Result best{minWidth, INT_MAX, INT_MAX};

    for (int w = minWidth; w <= maxWidth; ++w) {
        int lc = word_scorer::lineCount(lengths, w);   // Level 1
        int ws = word_scorer::wastedSpace(lengths, w); // Level 1

        if (lc < best.lineCount ||
            (lc == best.lineCount && ws < best.wastedSpace)) {
            best = {w, lc, ws};
        }
    }
    return best;
}

}  // close namespace col_optimizer


// ============================================================
// Unit tests for each level
// ============================================================

static void testWordScorer()
{
    // Completely independent test — no other component needed
    std::vector<int> lens = {5, 5, 3};  // "hello", "world", "foo"

    assert(word_scorer::lineCount(lens, 11) == 2);  // "hello world" / "foo"
    assert(word_scorer::lineCount(lens, 15) == 1);  // all fit
    assert(word_scorer::lineCount(lens, 4)  == INT_MAX);  // "hello" > 4

    assert(word_scorer::wastedSpace(lens, 11) == 8);  // "foo" + 8 trailing
    assert(word_scorer::wastedSpace(lens, 15) == 2);

    std::cout << "  [PASS] word_scorer tests\n";
}

static void testLinePacker()
{
    // Only needs word_scorer (Level 1) — isolated from col_optimizer
    std::vector<std::string> words = {"hello", "world", "foo", "bar"};
    auto lines = line_packer::pack(words, 11);

    assert(lines.size() == 2);
    assert(lines[0] == "hello world");
    assert(lines[1] == "foo bar");

    auto lengths = line_packer::getLengths(words);
    assert(lengths == std::vector<int>({5, 5, 3, 3}));

    std::cout << "  [PASS] line_packer tests\n";
}

static void testColOptimizer()
{
    // Uses both lower levels; but still a focused unit test
    std::vector<std::string> words = {"Large", "scale", "C++",
                                       "physical", "design"};
    auto result = col_optimizer::findOptimalWidth(words, 10, 30);

    std::cout << "  col_optimizer: optimal width=" << result.optimalWidth
              << " lines=" << result.lineCount
              << " waste=" << result.wastedSpace << "\n";
    assert(result.lineCount >= 1);
    assert(result.optimalWidth >= 10);

    std::cout << "  [PASS] col_optimizer tests\n";
}


// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 0: Hierarchical Reuse Demo ===\n\n";

    std::cout << "Running hierarchical tests:\n";
    std::cout << "Level 1 (word_scorer):\n";
    testWordScorer();

    std::cout << "Level 2 (line_packer):\n";
    testLinePacker();

    std::cout << "Level 3 (col_optimizer):\n";
    testColOptimizer();

    // Full application usage
    std::vector<std::string> sentence = {
        "Large", "scale", "C++", "process", "and",
        "architecture", "by", "John", "Lakos"
    };

    std::cout << "\n--- Full Application: Text Layout Optimizer ---\n";
    std::cout << "Words: ";
    for (const auto& w : sentence) std::cout << w << " ";
    std::cout << "\n\n";

    for (int width : {15, 20, 25, 30}) {
        auto lines = line_packer::pack(sentence, width);
        auto lens  = line_packer::getLengths(sentence);
        int  waste = word_scorer::wastedSpace(lens, width);

        std::cout << "Width=" << width
                  << " -> " << lines.size() << " line(s), "
                  << waste << " wasted chars\n";
        for (size_t i = 0; i < lines.size(); ++i) {
            std::cout << "  [" << i+1 << "] " << lines[i] << "\n";
        }
    }

    auto best = col_optimizer::findOptimalWidth(sentence, 15, 30);
    std::cout << "\nOptimal width: " << best.optimalWidth
              << " (lines=" << best.lineCount
              << ", waste=" << best.wastedSpace << ")\n";

    std::cout << "\n--- Dependency hierarchy (acyclic!) ---\n";
    std::cout << "col_optimizer  ← Level 3\n";
    std::cout << "  uses ↓\n";
    std::cout << "line_packer    ← Level 2\n";
    std::cout << "  uses ↓\n";
    std::cout << "word_scorer    ← Level 1\n";
    std::cout << "  uses ↓\n";
    std::cout << "(std library)  ← Level 0\n";

    return 0;
}
