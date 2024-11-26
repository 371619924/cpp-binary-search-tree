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

    BinaryTree(BinaryTree &&move) noexcept : root(std::move(move.root))
    {
        move.root = nullptr;
    }

    BinaryTree &operator=(BinaryTree &&move) noexcept
    {
        if (&move != this)
        {
            if (root)
            {
                root->freetree();
                delete root;
            }
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
        {
            root->freetree();
            delete root;
            root = nullptr;
        }
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
        {
            return nullptr;
        }
        auto new_node = new BinaryTreeNode<K, V>(node->key);
        new_node->value = node->value;
        new_node->height = node->height;
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
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr), height(1) {}

    void freetree()
    {
        if (left)
        {
            left->freetree();
            delete left;
            left = nullptr;
        }
        if (right)
        {
            right->freetree();
            delete right;
            right = nullptr;
        }
    }

    V &find(const K &key)
    {
        if (key == this->key)
        {
            return this->value;
        }

        if (key < this->key)
        {
            if (!left)
            {
                left = new BinaryTreeNode<K, V>(key);
            }
            return left->find(key);
        }

        if (!right)
        {
            right = new BinaryTreeNode<K, V>(key);
        }
        return right->find(key);
    }

    void updateHeight()
    {
        height = 1 + std::max(getHeight<K,V>(left), getHeight<K,V>(right));
    }

    int getBalanceFactor()
    {
        return getHeight(left) - getHeight(right);
    }

    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < this->key)
        {
            if (left)
            {
                left = left->erase(k);
            }
        }
        else if (k > this->key)
        {
            if (right)
            {
                right = right->erase(k);
            }
        }
        else
        {
            if (!left)
            {
                BinaryTreeNode<K, V> *temp = right;
                delete this;
                return temp;
            }
            if (!right)
            {
                BinaryTreeNode<K, V> *temp = left;
                delete this;
                return temp;
            }

            BinaryTreeNode<K, V> *temp = right;
            while (temp->left)
            {
                temp = temp->left;
            }

            std::swap(this->key, temp->key);
            std::swap(this->value, temp->value);

            right = right->erase(temp->key);
        }

        return balance();
    }

    bool contains(const K &k)
    {
        if (k == key)
            return true;
        if (k < key)
            return left && left->contains(k);
        return right && right->contains(k);
    }

    BinaryTreeNode<K, V> *rotateRight()
    {
        BinaryTreeNode<K, V> *newRoot = left;
        left = newRoot->right;
        newRoot->right = this;
        updateHeight();
        newRoot->updateHeight();
        return newRoot;
    }

    BinaryTreeNode<K, V> *rotateLeft()
    {
        BinaryTreeNode<K, V> *newRoot = right;
        right = newRoot->left;
        newRoot->left = this;
        updateHeight();
        newRoot->updateHeight();
        return newRoot;
    }

    BinaryTreeNode<K, V> *balance()
    {
        updateHeight();
        int balanceFactor = getBalanceFactor();

        if (balanceFactor > 1)
        {
            if (left && left->getBalanceFactor() < 0)
            {
                left = left->rotateLeft();
            }
            return rotateRight();
        }

        if (balanceFactor < -1)
        {
            if (right && right->getBalanceFactor() > 0)
            {
                right = right->rotateRight();
            }
            return rotateLeft();
        }

        return this;
    }

    K key;
    V value;
    BinaryTreeNode<K, V> *left;
    BinaryTreeNode<K, V> *right;
    int height;
};

template <class K, class V>
int getHeight(BinaryTreeNode<K, V> *node)
{
    return node ? node->height : 0;
}

std::vector<std::string> sortfile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Can't Open...");
    }

    BinaryTree<std::string, int> tree;
    std::string line;
    while (std::getline(file, line))
    {
        tree[line] = 0;
    }

    std::vector<std::string> sortedLines;
    for (const auto &[key, value] : tree)
    {
        sortedLines.push_back(key);
    }

    return sortedLines;
}

#endif // TREE_HPP
