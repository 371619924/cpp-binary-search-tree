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

    // Deep copy constructor (new:)
    BinaryTree(const BinaryTree &other)
    {
        root = copy_tree(other.root);
    }

    // Deep copy assignment operator (new:)
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

    // Move constructor
    BinaryTree(BinaryTree &&move) : root(move.root)
    {
        move.root = nullptr;
    }

    // Move assignment operator
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

    // Sort file function (new:)
    static std::vector<std::string> sortfile(const std::string &filename)
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

protected:
    BinaryTreeNode<K, V> *root;

    // Helper function for deep copy (new:)
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
        {
            return this->value;
        }

        if (key < this->key)
        {
            if (left)
            {
                return left->find(key);
            }
            else
            {
                left = new BinaryTreeNode<K, V>(key);
                return left->value;
            }
        }

        if (right)
        {
            return right->find(key);
        }
        else
        {
            right = new BinaryTreeNode<K, V>(key);
            return right->value;
        }
    }

protected:
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
            if (!left)
            {
                auto tmp = right;
                delete this;
                return tmp;
            }
            if (!right)
            {
                auto tmp = left;
                delete this;
                return tmp;
            }

            auto tmp = left;
            while (tmp->right)
            {
                tmp = tmp->right;
            }
            key = tmp->key;
            value = tmp->value;
            left = left->erase(tmp->key);
        }
        return this;
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

#endif // TREE_HPP
