// We are going to build a binary search tree with an explicit
// stack based iterator.  We are doing this as a "header-only"
// c++ implementation, as it is templated and therefore needs
// to be included to be used.  This provides basically the same
// high level functionality as std::map, that is, a key/value
// data store with a sorted order

// We need to include the following headers...

#include <memory>
#include <stack>
#include <utility>

#define HERE                             \
    {                                    \
        std::cout << "IMPLEMENT HERE\n"; \
    }

// C++ require declaration before use, so we define
// our three classes here.
template <class K, class V>
class BinaryTree;
template <class K, class V>
class BinaryTreeIterator;
template <class K, class V>
class BinaryTreeNode;

// This iterator is returned for both start and
// end but only the start iterator matters, the end
// iterator is effectively ignored.

// It is considered "undefined behavior" (that is,
// things are allowed to crash in obscure ways if)

// A: The iterator has a longer lifetime than the
// underlying tree (translation: don't return the iterator)

// B: Nodes are added or removed from the tree
// (translation: its OK to use the iterator to change
// the data associated with keys using the iterator in a
// for loop, but it is not OK to
// add new keys or remove keys)
template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>;

    // The constructor.  For end you don't actually
    // need to do anything (the end is effectively a dummy).
    // For the start iterator, you should set current to root
    // and then call incr() so that the iterator is ready to
    // actually start the traversal
    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start)
    {
        if (start)
        {
            current = root;
            incr(); // Prepare the iterator for traversal
        }
        else
        {
            current = nullptr;
        }
    }

public:
    // This should return TRUE if there is still
    // items left in the iteration:
    // Basically, check to see if the stack is empty
    // and current is null, and, if not, there is
    // still more work to do...
    bool operator!=(BinaryTreeIterator<K, V> &other)
    {
        return current != other.current || !working_stack.empty();
    }

    // This is the heart of the tree traversal algorithm.
    // It needs to be called once in the constructor and
    // once by each invocation of the ++ operator.
    // The idea in english...
    //
    // While current is not null, push the current
    // pointer onto the stack and take current->left
    // and make it the new current.
    //
    // Once current is null, then take the top of the stack
    // and set it to current...
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

    // This should just call incr
    void operator++()
    {
        incr();
    }

    // And this visits the node itself...
    // In visiting the node, it should
    // both set current to be the current node's right
    // and then return a pair constructed from the old
    // current node's key and value.
    std::pair<K, V> operator*()
    {
        return {current->key, current->value};
    }

private:
    // A pointer to the current node
    BinaryTreeNode<K, V> *current;

    // And a stack for the traversal visit of the tree
    // nodes.
    std::stack<BinaryTreeNode<K, V> *> working_stack;
};

// The class for the binary tree itself.
template <class K, class V>
class BinaryTree
{
public:
    BinaryTree() : root(nullptr) {}

    // The [] operation is for both getting and setting.
    // If the key exists in the tree a reference to the
    // associated value is returned.  Otherwise, it will
    // create a new tree node and return that value.
    V &operator[](const K &key)
    {
        auto *current = root;
        BinaryTreeNode<K, V> **parentLink = &root;

        while (current)
        {
            if (key == current->key)
            {
                return current->value; // Return value if found
            }
            parentLink = (key < current->key) ? &current->left : &current->right;
            current = (key < current->key) ? current->left : current->right;
        }

        // Create new node if key not found
        *parentLink = new BinaryTreeNode<K, V>(key);
        return (*parentLink)->value;
    }

    // Check if the tree contains a given key
    bool contains(const K &key)
    {
        auto *current = root;
        while (current)
        {
            if (key == current->key)
            {
                return true;
            }
            current = (key < current->key) ? current->left : current->right;
        }
        return false;
    }

    // Erases a node if a key matches.  If the
    // key does not match it simply is an operation
    // that does nothing.
    void erase(const K &key)
    {
        root = eraseNode(root, key);
    }

    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator(root, true);
    }
    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator(root, false);
    }

private:
    BinaryTreeNode<K, V> *root;

    BinaryTreeNode<K, V> *eraseNode(BinaryTreeNode<K, V> *node, const K &key)
    {
        if (!node)
        {
            return nullptr;
        }

        if (key < node->key)
        {
            node->left = eraseNode(node->left, key);
        }
        else if (key > node->key)
        {
            node->right = eraseNode(node->right, key);
        }
        else
        {
            if (!node->left)
            {
                auto *rightChild = node->right;
                delete node;
                return rightChild;
            }
            else if (!node->right)
            {
                auto *leftChild = node->left;
                delete node;
                return leftChild;
            }

            auto *minNode = findMin(node->right);
            node->key = minNode->key;
            node->value = minNode->value;
            node->right = eraseNode(node->right, minNode->key);
        }
        return node;
    }

    BinaryTreeNode<K, V> *findMin(BinaryTreeNode<K, V> *node)
    {
        while (node && node->left)
        {
            node = node->left;
        }
        return node;
    }
};

// And the class for the binary tree node itself.
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
        {
            left->freetree();
        }
        if (right)
        {
            right->freetree();
        }
        delete this;
    }

private:
    K key;
    V value;
    BinaryTreeNode<K, V> *left;
    BinaryTreeNode<K, V> *right;
};
