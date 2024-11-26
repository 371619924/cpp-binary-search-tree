#ifndef TREE_HPP
#define TREE_HPP

#include <memory>
#include <stack>
#include <utility>
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>

// Forward declarations
template <class K, class V>
class BinaryTree;
template <class K, class V>
class BinaryTreeIterator;
template <class K, class V>
class BinaryTreeNode;

// 全局函数声明
std::vector<std::string> sortfile(const std::string &filename);

// Tree Iterator Class
template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>;

    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start)
    {
        if (start)
        {
            current = root;
            incr();
        }
        else
        {
            current = nullptr;
        }
    }

public:
    bool operator!=(BinaryTreeIterator<K, V> &other)
    {
        return current != other.current || !working_stack.empty();
    }
    bool operator==(const BinaryTreeIterator<K, V> &other) const
    {
        return current == other.current && working_stack.empty() == other.working_stack.empty();
    }

    void incr()
    {
        while (current != nullptr)
        {
            working_stack.push(current);
            current = current->left;
        }

        if (!working_stack.empty())
        {
            current = working_stack.top();
            working_stack.pop();
            // After visiting the current node, move to its right child
            if (current != nullptr)
            {
                working_stack.push(current->right);
            }
        }
        else
        {
            current = nullptr;
        }
    }

    void operator++() { incr(); }

    std::pair<K, V> operator*()
    {
        auto result = std::make_pair(current->key, current->value);
        current = current->right;
        return result;
    }

private:
    BinaryTreeNode<K, V> *current;
    std::stack<BinaryTreeNode<K, V> *> working_stack;
};

// Tree Class
template <class K, class V>
class BinaryTree
{
public:
    BinaryTree() : root(nullptr) {}

    BinaryTree(const BinaryTree &other)
    {
        root = copy_tree(other.root);
    }

    BinaryTree &operator=(const BinaryTree &other)
    {
        if (this != &other)
        {
            if (root)
                root->freetree();
            root = copy_tree(other.root);
        }
        return *this;
    }

    BinaryTree(BinaryTree &&move) : root(move.root)
    {
        move.root = nullptr;
    }

    BinaryTree &operator=(BinaryTree &&move)
    {
        if (this != &move)
        {
            if (root)
                root->freetree();
            root = move.root;
            move.root = nullptr;
        }
        return *this;
    }

    V &operator[](const K &key)
    {
        if (root == nullptr)
        {
            root = new BinaryTreeNode<K, V>(key);
            return root->value;
        }
        return root->find(key);
    }

    bool contains(const K &key)
    {
        return root && root->contains(key);
    }

    void erase(const K &key)
    {
        if (root)
            root = root->erase(key);
    }

    ~BinaryTree()
    {
        if (root)
            root->freetree();
    }

    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator(root, true);
    }

    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator(root, false);
    }

protected:
    BinaryTreeNode<K, V> *root;

    BinaryTreeNode<K, V> *copy_tree(BinaryTreeNode<K, V> *node)
    {
        if (!node)
            return nullptr;
        auto new_node = new BinaryTreeNode<K, V>(node->key);
        new_node->value = node->value;
        new_node->left = copy_tree(node->left);
        new_node->right = copy_tree(node->right);
        return new_node;
    }
};

// Tree Node Class
template <class K, class V>
class BinaryTreeNode
{
    friend class BinaryTree<K, V>;
    friend class BinaryTreeIterator<K, V>;

public:
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr) {}

    void freetree()
    {
        if (left)
            left->freetree();
        if (right)
            right->freetree();
        delete this;
    }

    V &find(const K &key)
    {
        if (key == this->key)
            return this->value; // 返回节点值的引用

        if (key < this->key)
        {
            if (!left)
                left = new BinaryTreeNode<K, V>(key);
            return left->find(key); // 递归调用返回引用
        }
        else
        {
            if (!right)
                right = new BinaryTreeNode<K, V>(key);
            return right->find(key); // 递归调用返回引用
        }
    }

    BinaryTreeNode<K, V> *balance()
    {
        int balance_factor = get_balance();
        if (balance_factor > 1) // 左子树更重
        {
            if (left && left->get_balance() < 0) // LR
                left = left->rotate_left();
            return rotate_right();
        }
        if (balance_factor < -1) // 右子树更重
        {
            if (right && right->get_balance() > 0) // RL
                right = right->rotate_right();
            return rotate_left();
        }
        return this;
    }

protected:
    int height; // 用于存储节点的高度

    int get_height(BinaryTreeNode<K, V> *node) const
    {
        return node ? node->height : 0;
    }

    int get_balance() const
    {
        return get_height(left) - get_height(right);
    }

    BinaryTreeNode<K, V> *rotate_right()
    {
        BinaryTreeNode<K, V> *new_root = left;
        left = new_root->right;
        new_root->right = this;
        update_height();
        new_root->update_height();
        return new_root;
    }

    BinaryTreeNode<K, V> *rotate_left()
    {
        BinaryTreeNode<K, V> *new_root = right;
        right = new_root->left;
        new_root->left = this;
        update_height();
        new_root->update_height();
        return new_root;
    }

    void update_height()
    {
        height = 1 + std::max(get_height(left), get_height(right));
    }
    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < key)
        {
            if (left)
                left = left->erase(k);
        }
        else if (k > key)
        {
            if (right)
                right = right->erase(k);
        }
        else
        {
            if (!left || !right)
            {
                BinaryTreeNode<K, V> *tmp = left ? left : right;
                delete this;
                return tmp;
            }
            else
            {
                BinaryTreeNode<K, V> *tmp = left;
                while (tmp->right)
                    tmp = tmp->right;
                key = tmp->key;
                value = tmp->value;
                left = left->erase(tmp->key);
            }
        }
        update_height();
        // 延迟平衡和旋转操作，只有在需要时才进行
        int balance_factor = get_balance();
        if (balance_factor > 1 || balance_factor < -1)
        {
            return balance(); // 重新平衡树
        }

        return this; // 返回当前节点，避免不必要的递归
    }

    bool contains(const K &k)
    {
        if (k == key)
            return true;
        if (k < key)
            return left && left->contains(k);
        return right && right->contains(k);
    }

    K key;
    V value;
    BinaryTreeNode<K, V> *left;
    BinaryTreeNode<K, V> *right;
};

// 全局函数定义
std::vector<std::string> sortfile(const std::string &filename)
{
    BinaryTree<std::string, int> tree;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Error opening file");
    }

    std::string line;
    while (std::getline(file, line))
    {
        tree[line] = 0; // Arbitrary value
    }
    file.close();

    std::vector<std::string> sorted_lines;
    for (const auto &[key, value] : tree)
    {
        sorted_lines.push_back(key);
    }
    return sorted_lines;
}

#endif // TREE_HPP
