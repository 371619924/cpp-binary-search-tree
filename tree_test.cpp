#include <gtest/gtest.h>
#include <string>
#include <algorithm>
#include <random>
#include <ranges>
#include "tree.hpp"

TEST(TreeTest, BasicTests)
{
    auto rng = std::default_random_engine{};
    std::vector<std::string> testStrings = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    std::vector<int> testValues = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    BinaryTree<std::string, int> b;

    for (auto i : std::views::iota(0, 100))
    {
        // These tests are NOT complete.  In particular it does
        // not include tests for the iterator and similar functionality
        // since those will crash in the current form, but you add tests
        // for all the functionality here and you'll probably be OK if you also
        // check for code coverage.

        // But this should give you a good idea of strategy to use...
        (void)i;
        BinaryTree<std::string, int> c;

        // This reorders the testStrings vector which you can use
        // for various testing purposes.  Since we do this a hundred
        // times and the testStrings array is different each time,
        // we expect this to provide good coverage if you test
        // all the functionality.
        std::shuffle(testStrings.begin(), testStrings.end(), rng);

        auto j = 0;
        for (auto s : testStrings)
        {
            b[s] = testValues[j];
            c[s] = testValues[j];
            EXPECT_EQ(testValues[j], b[s]);
        }

        std::string res = "";
        for (const auto &[key, value] : c)
        {
            res += key;
        }
        EXPECT_EQ(res, "ABCDEFGHIJ");
    }
    b["fubar"] = 43;
    b["baz"] = 62;
    EXPECT_EQ(b["fubar"], 43);
    EXPECT_EQ(b["baz"], 62);
}
