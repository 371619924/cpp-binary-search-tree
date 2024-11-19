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
TEST(TreeTest, AdditionalFunctionalityTests)
{
    BinaryTree<std::string, int> tree;

    // 测试查找不存在的键会插入默认值
    EXPECT_EQ(tree["missing"], 0); // 默认值应为 0
    tree["missing"] = 42;          // 设置值为 42
    EXPECT_EQ(tree["missing"], 42); // 验证插入是否成功

    // 测试覆盖值
    tree["key"] = 10;
    EXPECT_EQ(tree["key"], 10); // 检查初始插入
    tree["key"] = 20;
    EXPECT_EQ(tree["key"], 20); // 检查覆盖是否成功

    // 测试是否包含某个键
    EXPECT_TRUE(tree.contains("key")); // 键 "key" 应该存在
    EXPECT_FALSE(tree.contains("nonexistent")); // 键 "nonexistent" 不应该存在

    // 测试删除功能
    tree["to_delete"] = 99; // 插入待删除键
    EXPECT_TRUE(tree.contains("to_delete")); // 删除前应存在
    tree.erase("to_delete"); // 删除键
    EXPECT_FALSE(tree.contains("to_delete")); // 删除后应不存在

    // 测试空树操作
    BinaryTree<std::string, int> emptyTree;
    EXPECT_FALSE(emptyTree.contains("any")); // 空树不应该包含任何键
    EXPECT_EQ(emptyTree["default"], 0); // 查找不存在的键应返回默认值
}

TEST(TreeTest, IteratorTests)
{
    BinaryTree<std::string, int> tree;
    std::vector<std::pair<std::string, int>> keyValuePairs = {
        {"B", 2}, {"A", 1}, {"D", 4}, {"C", 3}, {"E", 5}};
    
    // 插入测试键值对
    for (const auto &[key, value] : keyValuePairs)
    {
        tree[key] = value;
    }

    // 验证迭代器按中序遍历顺序输出
    std::string resultKeys;
    std::vector<int> resultValues;
    for (const auto &[key, value] : tree)
    {
        resultKeys += key;
        resultValues.push_back(value);
    }
    EXPECT_EQ(resultKeys, "ABCDE"); // 验证键顺序
    EXPECT_EQ(resultValues, (std::vector<int>{1, 2, 3, 4, 5})); // 验证值顺序
}

TEST(TreeTest, LargeScaleTests)
{
    BinaryTree<int, int> tree;

    // 插入大量数据
    for (int i = 0; i < 10000; ++i)
    {
        tree[i] = i * 10; // 插入键值对
    }

    // 验证插入是否正确
    for (int i = 0; i < 10000; ++i)
    {
        EXPECT_EQ(tree[i], i * 10); // 检查每个键的值
    }

    // 测试大规模删除
    for (int i = 0; i < 5000; ++i)
    {
        tree.erase(i); // 删除前半部分键
    }

    // 验证删除后是否存在
    for (int i = 0; i < 5000; ++i)
    {
        EXPECT_FALSE(tree.contains(i)); // 前半部分键应不存在
    }

    for (int i = 5000; i < 10000; ++i)
    {
        EXPECT_TRUE(tree.contains(i)); // 后半部分键应存在
    }
}
