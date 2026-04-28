// software_capital_demo.cpp
//
// Demonstrates the core concept from Chapter 0: Software Capital.
//
// A well-designed library of components is "capital" that compounds over time.
// This demo shows three levels of the hierarchy:
//   Level 1: math_util  - lowest-level math primitives
//   Level 2: text_util  - text utilities that USE math_util
//   Level 3: app        - application that uses both
//
// KEY POINT: Each level is independently testable because dependencies are
// acyclic (Level1 → no deps, Level2 → Level1, Level3 → Level1,Level2).
//
// Book reference: Chapter 0, sections 0.7-0.10

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

// ============================================================
// LEVEL 1 COMPONENT: math_util
//   A low-level utility component with no dependencies on this codebase.
//   Independently testable.
// ============================================================

// math_util.h  (embedded here for demo; normally in a separate file)
namespace math_util {

/// Returns the number of digits in a non-negative integer.
int digitCount(int n);

/// Returns the minimum number of lines needed to fit words of given lengths
/// into lines of maxWidth, where words are separated by single spaces.
int minLines(const std::vector<int>& wordLengths, int maxWidth);

}  // close namespace math_util

// math_util.cpp  (implementation)
namespace math_util {

int digitCount(int n)
{
    if (n == 0) return 1;
    int count = 0;
    while (n > 0) { ++count; n /= 10; }
    return count;
}

int minLines(const std::vector<int>& wordLengths, int maxWidth)
{
    int lines = 1;
    int currentLineWidth = 0;
    for (int len : wordLengths) {
        if (currentLineWidth == 0) {
            currentLineWidth = len;
        } else if (currentLineWidth + 1 + len <= maxWidth) {
            currentLineWidth += 1 + len;
        } else {
            ++lines;
            currentLineWidth = len;
        }
    }
    return lines;
}

}  // close namespace math_util


// ============================================================
// LEVEL 2 COMPONENT: text_util
//   Depends on math_util. Independently testable in isolation from
//   higher-level components.
// ============================================================

// text_util.h  (embedded here for demo)
namespace text_util {

/// Returns word lengths from a sentence.
std::vector<int> wordLengths(const std::string& sentence);

/// Returns the "column cost" of fitting a sentence into a given width.
/// Uses math_util::minLines internally.
int columnCost(const std::string& sentence, int columnWidth);

}  // close namespace text_util

// text_util.cpp  (implementation using math_util)
namespace text_util {

std::vector<int> wordLengths(const std::string& sentence)
{
    std::vector<int> lengths;
    int wordLen = 0;
    for (char c : sentence) {
        if (c == ' ') {
            if (wordLen > 0) {
                lengths.push_back(wordLen);
                wordLen = 0;
            }
        } else {
            ++wordLen;
        }
    }
    if (wordLen > 0) lengths.push_back(wordLen);
    return lengths;
}

int columnCost(const std::string& sentence, int columnWidth)
{
    auto lengths = wordLengths(sentence);
    return math_util::minLines(lengths, columnWidth);  // Uses Level 1!
}

}  // close namespace text_util


// ============================================================
// Unit tests for Level 1 and Level 2 (run independently)
// ============================================================

static void testMathUtil()
{
    // Test math_util independently — NO DEPENDENCY on text_util
    assert(math_util::digitCount(0)    == 1);
    assert(math_util::digitCount(9)    == 1);
    assert(math_util::digitCount(99)   == 2);
    assert(math_util::digitCount(100)  == 3);

    // "hello world foo bar" words=[5,5,3,3], width=12
    // Line 1: "hello world" (11), Line 2: "foo bar" (7) → 2 lines
    std::vector<int> wl = {5, 5, 3, 3};
    assert(math_util::minLines(wl, 12) == 2);
    assert(math_util::minLines(wl, 20) == 1);

    std::cout << "  [PASS] math_util tests\n";
}

static void testTextUtil()
{
    // Test text_util independently — only needs math_util (Level 1)
    auto lengths = text_util::wordLengths("hello world foo");
    assert(lengths.size() == 3);
    assert(lengths[0] == 5);  // "hello"
    assert(lengths[1] == 5);  // "world"
    assert(lengths[2] == 3);  // "foo"

    int cost = text_util::columnCost("hello world foo", 12);
    assert(cost == 2);  // "hello world" + "foo"

    std::cout << "  [PASS] text_util tests\n";
}


// ============================================================
// LEVEL 3: Application — uses both levels
// ============================================================

static void runApplication()
{
    const std::string text = "Large scale C++ is about physical design discipline and software capital";
    const int columnWidth = 30;

    std::cout << "\n--- Application Demo ---\n";
    std::cout << "Text: \"" << text << "\"\n";
    std::cout << "Column width: " << columnWidth << "\n";
    std::cout << "Lines needed: "
              << text_util::columnCost(text, columnWidth) << "\n";

    auto lengths = text_util::wordLengths(text);
    std::cout << "Word count: " << lengths.size() << "\n";
    std::cout << "Digit count of word count: "
              << math_util::digitCount(static_cast<int>(lengths.size())) << "\n";
}


// ============================================================
// Main: demonstrates hierarchical reuse
// ============================================================

int main()
{
    std::cout << "=== Chapter 0: Software Capital Demo ===\n\n";

    std::cout << "Running independent unit tests by level:\n";
    std::cout << "Level 1 (math_util) - tests Level 1 only:\n";
    testMathUtil();

    std::cout << "Level 2 (text_util) - tests Level 2 + Level 1:\n";
    testTextUtil();

    std::cout << "\nKey insight:\n";
    std::cout << "  - math_util can be tested with ZERO external dependencies.\n";
    std::cout << "  - text_util can be tested with ONE external dependency (math_util).\n";
    std::cout << "  - Neither requires the full application to be built.\n";
    std::cout << "  - This is 'Hierarchical Testability' (Lakos §2.14).\n";

    runApplication();

    std::cout << "\nDependency structure (acyclic DAG):\n";
    std::cout << "  Application\n";
    std::cout << "    ↓ uses\n";
    std::cout << "  text_util\n";
    std::cout << "    ↓ uses\n";
    std::cout << "  math_util\n";
    std::cout << "    ↓ uses\n";
    std::cout << "  (C++ standard library only)\n";

    return 0;
}
