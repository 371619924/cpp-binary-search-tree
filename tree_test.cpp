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
        (void)i; 
        BinaryTree<std::string, int> c; 
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

TEST(TreeTest, AdditionalFunctionalityTests)
{
    BinaryTree<std::string, int> tree;

    EXPECT_EQ(tree["missing"], 0);
    tree["missing"] = 42;
    EXPECT_EQ(tree["missing"], 42);

    tree["key"] = 10;
    EXPECT_EQ(tree["key"], 10);
    tree["key"] = 20;
    EXPECT_EQ(tree["key"], 20);

    EXPECT_TRUE(tree.contains("key"));
    EXPECT_FALSE(tree.contains("nonexistent"));

    tree["to_delete"] = 99;
    EXPECT_TRUE(tree.contains("to_delete"));
    tree.erase("to_delete");
    EXPECT_FALSE(tree.contains("to_delete"));

    BinaryTree<std::string, int> emptyTree;
    EXPECT_FALSE(emptyTree.contains("any"));
    EXPECT_EQ(emptyTree["default"], 0);
}

TEST(TreeTest, IteratorTests)
{
    BinaryTree<std::string, int> tree;
    std::vector<std::pair<std::string, int>> keyValuePairs = {
        {"B", 2}, {"A", 1}, {"D", 4}, {"C", 3}, {"E", 5}};
    
    for (const auto &[key, value] : keyValuePairs)
    {
        tree[key] = value;
    }

    std::string resultKeys;
    std::vector<int> resultValues;
    for (const auto &[key, value] : tree)
    {
        resultKeys += key;
        resultValues.push_back(value);
    }
    EXPECT_EQ(resultKeys, "ABCDE");
    EXPECT_EQ(resultValues, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(TreeTest, LargeScaleTests)
{
    BinaryTree<int, int> tree;

    for (int i = 0; i < 10000; ++i)
    {
        tree[i] = i * 10;
    }

    for (int i = 0; i < 10000; ++i)
    {
        EXPECT_EQ(tree[i], i * 10);
    }

    for (int i = 0; i < 5000; ++i)
    {
        tree.erase(i);
    }

    for (int i = 0; i < 5000; ++i)
    {
        EXPECT_FALSE(tree.contains(i));
    }

    for (int i = 5000; i < 10000; ++i)
    {
        EXPECT_TRUE(tree.contains(i));
    }
}

TEST(TreeTest, MoveAndCopyConstructors)
{
    BinaryTree<std::string, int> tree;
    tree["A"] = 1;
    tree["B"] = 2;
    tree["C"] = 3;

    // 测试拷贝构造函数
    BinaryTree<std::string, int> copyTree = tree;
    EXPECT_EQ(copyTree["A"], 1);
    EXPECT_EQ(copyTree["B"], 2);
    EXPECT_EQ(copyTree["C"], 3);

    // 测试移动构造函数
    BinaryTree<std::string, int> movedTree = std::move(tree);
    EXPECT_EQ(movedTree["A"], 1);
    EXPECT_EQ(movedTree["B"], 2);
    EXPECT_EQ(movedTree["C"], 3);
    EXPECT_EQ(tree["A"], 0); // 检查源树是否为空
}

TEST(TreeTest, MoveAndCopyAssignmentOperators)
{
    BinaryTree<std::string, int> tree;
    tree["A"] = 1;
    tree["B"] = 2;
    tree["C"] = 3;

    // 测试拷贝赋值操作符
    BinaryTree<std::string, int> copyTree;
    copyTree = tree;
    EXPECT_EQ(copyTree["A"], 1);
    EXPECT_EQ(copyTree["B"], 2);
    EXPECT_EQ(copyTree["C"], 3);

    // 测试移动赋值操作符
    BinaryTree<std::string, int> movedTree;
    movedTree = std::move(tree);
    EXPECT_EQ(movedTree["A"], 1);
    EXPECT_EQ(movedTree["B"], 2);
    EXPECT_EQ(movedTree["C"], 3);
    EXPECT_EQ(tree["A"], 0); // 检查源树是否为空
}

TEST(TreeTest, SortFileTest) {
    const std::string filename = "test_file.txt";
    std::ofstream outFile(filename);
    outFile << "banana\napple\ncherry\n";
    outFile.close();

    // 调用全局函数 sortfile
    auto sortedLines = sortfile(filename);

    EXPECT_EQ(sortedLines, (std::vector<std::string>{"apple", "banana", "cherry"}));
    std::remove(filename.c_str());
}

TEST(PerformanceTest, TestPerformance)
{
    auto start = std::chrono::system_clock::now();
    auto range = 50000;
    {
        BinaryTree<int, int> c;
        for (auto x : std::views::iota(0, range))
        {
            c[x] = x;
            EXPECT_TRUE(c.contains(x));
        }
        for (auto x : std::views::iota(0, range))
        {
            EXPECT_TRUE(c.contains(x));
            c.erase(x);
            EXPECT_FALSE(c.contains(x));
        }
    }
    auto second = std::chrono::system_clock::now();
    {
        BinaryTree<int, int> c;
        for (auto x : std::views::iota(0, range * 10))
        {
            c[x] = x;
            EXPECT_TRUE(c.contains(x));
        }
        for (auto x : std::views::iota(0, range * 10))
        {
            EXPECT_TRUE(c.contains(x));
            c.erase(x);
            EXPECT_FALSE(c.contains(x));
        }
    }
    auto done = std::chrono::system_clock::now();
    auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(second - start).count();
    auto t2 = std::chrono::duration_cast<std::chrono::microseconds>(done - second).count();
    std::cout << "Time for " << range << "  insertions:  " << t1 << "us\n";
    std::cout << "Time for " << (10 * range) << " insertions: " << t2 << "us\n";
    std::cout << "If AVL is working right, the second should take no more than 10 + lg(10) times more than the first\n";
    EXPECT_TRUE(t2 < (t1 * 10 * 4));
}