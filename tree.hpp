#include <memory>  // For smart pointers (used in tree node memory management)
#include <stack>   // For the stack used in the tree traversal iterator
#include <utility> // For std::pair (used in the iterator to return key-value pairs)

#define HERE                             \
    {                                    \
        std::cout << "IMPLEMENT HERE\n"; \
    }

template <class K, class V>
class BinaryTree; // Forward declaration of BinaryTree class
template <class K, class V>
class BinaryTreeIterator; // Forward declaration of BinaryTreeIterator class
template <class K, class V>
class BinaryTreeNode; // Forward declaration of BinaryTreeNode class

// This iterator is returned for both start and
// end but only the start iterator matters, the end
// iterator is effectively ignored.
template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>; // Allows BinaryTree to access private members of the iterator

    // The constructor.  For end you don't actually
    // need to do anything (the end is effectively a dummy).
    // For the start iterator, you should set current to root
    // and then call incr() so that the iterator is ready to
    // actually start the traversal
    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start)
    {
        if (start)
        {
            current = root; // Initialize current to root if start is true
            incr(); // Prepare the iterator for traversal by calling incr()
        }
        else
        {
            current = nullptr; // End iterator just sets current to nullptr
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
        // The iterator is not equal to the other if either current is different or
        // the working stack isn't empty
        return current != other.current || !working_stack.empty();
    }

    // This is the heart of the tree traversal algorithm.
    // It needs to be called once in the constructor and
    // once by each invocation of the ++ operator.
    // The idea in English...
    //
    // While current is not null, push the current
    // pointer onto the stack and take current->left
    // and make it the new current.
    //
    // Once current is null, then take the top of the stack
    // and set it to current...
    void incr()
    {
        while (current != nullptr) // Traverse left subtree
        {
            working_stack.push(current); // Push current node onto stack
            current = current->left; // Move to the left child
        }

        if (!working_stack.empty()) // If stack is not empty, continue traversal
        {
            current = working_stack.top(); // Pop the node from stack
            working_stack.pop(); // Remove the top element from stack
        }
        else
        {
            current = nullptr; // No more nodes to visit
        }
    }

    // This should just call incr
    void operator++()
    {
        incr(); // Move to the next node in the in-order traversal
    }

    // And this visits the node itself...
    // In visiting the node, it should
    // both set current to be the current node's right
    // and then return a pair constructed from the old
    // current node's key and value.
    std::pair<K, V> operator*()
    {
        auto result = std::make_pair(current->key, current->value); // Return current node's key-value pair
        current = current->right; // Move to the right subtree after visiting current
        return result;
    }

private:
    BinaryTreeNode<K, V> *current; // Pointer to the current node in the traversal

    // A stack for the traversal visit of the tree nodes.
    std::stack<BinaryTreeNode<K, V> *> working_stack;
};

// The class for the binary tree itself.
template <class K, class V>
class BinaryTree
{
public:
    BinaryTree() : root(nullptr) // Constructor initializing the root to nullptr
    {
    }

    // The [] operation is for both getting and setting.
    // If the key exists in the tree a reference to the
    // associated value is returned.  Otherwise, it will
    // create a new tree node and return that value.
    V &operator[](const K &key)
    {
        if (root == nullptr) // If the tree is empty, create a new node
        {
            root = new BinaryTreeNode<K, V>(key); // Create a new node with the key
            return root->value; // Return the default constructed value associated with the key
        }
        return root->find(key); // Find the key and return the associated value
    }

    // This should return false if the tree
    // is empty, otherwise return root->contains
    bool contains(const K &key)
    {
        return root && root->contains(key); // Check if root is non-null and contains the key
    }

    // Erases a node if a key matches.  If the
    // key does not match it simply is an operation
    // that does nothing.
    void erase(const K &key)
    {
        if (root) // If the tree is non-empty
        {
            root = root->erase(key); // Call erase on the root to remove the key
        }
    }

    // Destructor that frees the memory of the tree by calling freetree
    ~BinaryTree()
    {
        if (root)
        {
            root->freetree(); // Recursively frees all nodes in the tree
        }
    }

    // This returns the iterators.
    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator(root, true); // Create a start iterator
    }

    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator(root, false); // Create an end iterator (dummy)
    }

    // Move constructor that transfers ownership of the root node
    BinaryTree(BinaryTree &&move) : root(move.root)
    {
        move.root = nullptr; // Nullify the root of the moved-from tree
    }

    // Move assignment operator that transfers ownership of the root node
    BinaryTree &operator=(BinaryTree &&move)
    {
        if (&move != this) // Avoid self-assignment
        {
            if (root) // Free the old tree if necessary
            {
                root->freetree();
            }
            root = move.root; // Transfer ownership of the root
            move.root = nullptr; // Nullify the root of the moved-from tree
        }
        return *this;
    }

protected:
    BinaryTreeNode<K, V> *root; // Pointer to the root node of the tree
};

// And the class for the binary tree node itself.
template <class K, class V>
class BinaryTreeNode
{
    friend class BinaryTree<K, V>; // Allow BinaryTree to access private members
    friend class BinaryTreeIterator<K, V>; // Allow BinaryTreeIterator to access private members

public:
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr) // Constructor to initialize key and child nodes
    {
    }

    // This should recursively free the tree.
    // It should call freetree on left and
    // right and then, as the last act,
    // call delete on this.
    void freetree()
    {
        if (left) // If left child exists, free it recursively
        {
            left->freetree();
        }
        if (right) // If right child exists, free it recursively
        {
            right->freetree();
        }
        delete this; // Delete the current node
    }

    // Add this function inside BinaryTreeNode
    V &find(const K &key)
    {
        // 基本情况：如果当前节点的键匹配，返回该节点的值
        if (key == this->key) // If the current node's key matches the search key
        {
            return this->value; // Return the value of the current node
        }

        // 如果键较小，递归查找左子树
        if (key < this->key) // If the key is smaller than the current node's key
        {
            if (left) // If left child exists, search recursively
            {
                return left->find(key);
            }
            else
            {
                // 如果左子树为空，创建一个新节点
                left = new BinaryTreeNode<K, V>(key); // Create a new node in the left subtree
                return left->value; // Return the value of the new node
            }
        }

        // 如果键较大，递归查找右子树
        if (right) // If right child exists, search recursively
        {
            return right->find(key);
        }
        else
        {
            // 如果右子树为空，创建一个新节点
            right = new BinaryTreeNode<K, V>(key); // Create a new node in the right subtree
            return right->value; // Return the value of the new node
        }
    }

protected:
    // Removing a node from a binary tree, returning
    // a pointer to the now modified tree.
    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < key) // If the key is smaller than the current node's key
        {
            if (left) // Recursively call erase on left subtree
            {
                left = left->erase(k);
            }
        }
        else if (k > key) // If the key is greater than the current node's key
        {
            if (right) // Recursively call erase on right subtree
            {
                right = right->erase(k);
            }
        }
        else // If the current node's key matches the key to be erased
        {
            if (!left) // If there's no left child, return right child
            {
                auto tmp = right; // Temporarily hold the right subtree
                delete this; // Delete the current node
                return tmp; // Return the right child
            }
            if (!right) // If there's no right child, return left child
            {
                auto tmp = left; // Temporarily hold the left subtree
                delete this; // Delete the current node
                return tmp; // Return the left child
            }

            // If there are both left and right children, find the in-order successor
            auto tmp = left;
            while (tmp->right) // Traverse to the rightmost node of the left subtree
            {
                tmp = tmp->right;
            }
            key = tmp->key; // Replace current node's key with the in-order successor's key
            value = tmp->value; // Replace current node's value with the in-order successor's value
            left = left->erase(tmp->key); // Recursively erase the in-order successor node
        }
        return this; // Return the modified node
    }

    // And contains is a recursive search that doesn't
    // create new nodes, just checks if the key exists.
    bool contains(const K &k)
    {
        if (k == key) // If the current node's key matches
        {
            return true; // Return true, key is found
        }
        if (k < key) // If the key is smaller, check the left subtree
        {
            return left && left->contains(k); // Recursively check left subtree
        }
        return right && right->contains(k); // Otherwise, check the right subtree
    }

    K key; // The key of the current node
    V value; // The value associated with the key
    BinaryTreeNode<K, V> *left; // Pointer to the left child
    BinaryTreeNode<K, V> *right; // Pointer to the right child
};
