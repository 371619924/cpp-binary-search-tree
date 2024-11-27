#ifndef TREE_HPP
#define TREE_HPP


#include <memory>      
#include <stack>      
#include <utility>     
#include <vector>      
#include <fstream>     
#include <string>      
#include <algorithm>   
#include <stdexcept>   
//version 27 attmpt
// Forward declarations of template classes used in the tree
template <class K, class V> class BinaryTree; 
template <class K, class V> class BinaryTreeIterator; 
template <class K, class V> class BinaryTreeNode; 

// Iterator class for the BinaryTree. It will iterate through the tree nodes.
template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>; // BinaryTree can access private members of this iterator

    // The constructor initializes the iterator to either the root of the tree (start) or nullptr (end)
    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start) : current(root)
    {
        if (start)
        {
            incr(); // Ensure the iterator is positioned at the first node (in-order traversal start)
        }
    }

public:
    // Compares iterators to check if they point to different nodes
    bool operator!=(BinaryTreeIterator<K, V> &other) 
    {
        // Iterator is not equal if current node or stack state differ
        return current != other.current || !working_stack.empty();
    }

    // The core logic for traversing the tree in-order.
    // Pushes left children onto the stack until null, then pops from the stack
    void incr()
    {
        while (current != nullptr)
        {
            working_stack.push(current); // Push current node to the stack
            current = current->left;     // Move to left child
        }

        if (!working_stack.empty())
        {
            current = working_stack.top(); // Pop the stack to get the next node
            working_stack.pop();
        }
        else
        {
            current = nullptr; // No more nodes to visit
        }
    }

    // Increments the iterator to the next node
    void operator++() { incr(); }

    // Dereferencing the iterator returns a pair of the current node's key and value
    std::pair<K, V> operator*() 
    {
        auto result = std::make_pair(current->key, current->value); // Get current node's key-value pair
        current = current->right; // Move to the right child for the next iteration
        return result;
    }

private:
    BinaryTreeNode<K, V> *current;  // The current node being pointed to
    std::stack<BinaryTreeNode<K, V> *> working_stack; // Stack for holding nodes during in-order traversal
};

// BinaryTree class which is the actual tree structure that holds nodes
template <class K, class V>
class BinaryTree
{
public:
    BinaryTree() : root(nullptr) {} // Constructor initializes the root to nullptr

    // Overload the [] operator for accessing and inserting key-value pairs
    V &operator[](const K &key)
    {
        if (root == nullptr)
        {
            root = new BinaryTreeNode<K, V>(key); // If the tree is empty, create the root node
        }
        else
        {
            root = balanceInsert(root, key); // Insert the new key while maintaining tree balance
        }

        return root->find(key); // Return the reference to the value associated with the key
    }

    // Checks if a key exists in the tree
    bool contains(const K &key)
    {
        return root && root->contains(key); // Return true if key is found, false otherwise
    }

    // Removes a node with a given key from the tree
    void erase(const K &key)
    {
        if (root)
        {
            root = root->erase(key); // Erase node and re-balance the tree
        }
    }

    ~BinaryTree()
    {
        if (root)
        {
            root->freetree(); // Free the entire tree starting from root
            delete root;      // Delete the root pointer
            root = nullptr;
        }
    }

    // Returns an iterator pointing to the first node (start of in-order traversal)
    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator<K, V>(root, true); // Start iteration from the root
    }

    // Returns an iterator representing the end of the tree (empty iterator)
    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator<K, V>(nullptr, false); // End iterator does not point to any node
    }

    // Copy constructor creates a deep copy of another tree
    BinaryTree(const BinaryTree &other) : root(nullptr)
    {
        if (other.root)
        {
            root = other.root->copy_tree(); // Copy the tree structure from another tree
        }
    }

    // Copy assignment operator performs deep copy
    BinaryTree &operator=(const BinaryTree &other)
    {
        if (this != &other)
        {
            if (root)
            {
                root->freetree(); // Free current tree before copying
                delete root;
            }
            root = other.root ? other.root->copy_tree() : nullptr; // Copy the tree from another instance
        }
        return *this;
    }

    // Move constructor for efficient transferring ownership of resources
    BinaryTree(BinaryTree &&move) noexcept : root(move.root)
    {
        move.root = nullptr; // Nullify the original tree's root after moving ownership
    }

    // Move assignment operator for efficient transferring ownership
    BinaryTree &operator=(BinaryTree &&move) noexcept
    {
        if (&move != this)
        {
            if (root)
            {
                root->freetree(); // Free current tree before moving
                delete root;
            }
            root = move.root; // Transfer ownership of the tree
            move.root = nullptr; // Nullify the moved tree's root
        }
        return *this;
    }

    // Check if the tree is balanced according to AVL rules
    bool is_avl() const
    {
        return check_avl(root); // Recursively check if the tree is AVL
    }

protected:
    BinaryTreeNode<K, V> *root; // Root node of the tree

    // Insert a key while maintaining balance
    BinaryTreeNode<K, V> *balanceInsert(BinaryTreeNode<K, V> *node, const K &key)
    {
        if (!node)
        {
            return new BinaryTreeNode<K, V>(key); // Create a new node if current is null
        }
        if (key < node->key)
        {
            node->left = balanceInsert(node->left, key); // Insert in left subtree if key is smaller
        }
        else if (key > node->key)
        {
            node->right = balanceInsert(node->right, key); // Insert in right subtree if key is larger
        }
        return node->balance(); // Balance the tree after insertion
    }

    // Recursively check if the tree is balanced at all levels
    bool check_avl(BinaryTreeNode<K, V> *node) const
    {
        if (!node)
        {
            return true; // Base case: an empty node is balanced
        }

        // Check if the balance factor (height difference) is within AVL limits
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);
        int balanceFactor = leftHeight - rightHeight;

        // If the tree is unbalanced, return false
        if (std::abs(balanceFactor) > 1)
            return false;

        // Recursively check the left and right subtrees
        return check_avl(node->left) && check_avl(node->right);
    }
};

// Node class that represents each element in the binary tree
template <class K, class V>
class BinaryTreeNode
{
    friend class BinaryTree<K, V>; // BinaryTree can access private members of the node
    friend class BinaryTreeIterator<K, V>; // BinaryTreeIterator can access private members

public:
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr), height(1) {} // Node constructor

    // Free memory of the left and right subtrees recursively
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

    // Remove a node with the specified key and return the modified tree
    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < this->key) // If key is smaller, look in the left subtree
        {
            left = left->erase(k);
        }
        else if (k > this->key) // If key is larger, look in the right subtree
        {
            right = right->erase(k);
        }
        else // If key is found, delete this node
        {
            if (!left && !right) // No children
            {
                delete this;
                return nullptr;
            }
            else if (!left) // Only right child
            {
                BinaryTreeNode<K, V> *temp = right;
                delete this;
                return temp;
            }
            else if (!right) // Only left child
            {
                BinaryTreeNode<K, V> *temp = left;
                delete this;
                return temp;
            }
            else // Both children exist, need to find in-order successor
            {
                BinaryTreeNode<K, V> *temp = right;
                while (temp->left != nullptr)
                    temp = temp->left; // Find the smallest node in the right subtree
                key = temp->key;
                right = right->erase(temp->key); // Remove the in-order successor from the right subtree
            }
        }
        return this->balance(); // Rebalance the tree after erasure
    }

    // Find a node with the given key and return its associated value
    V &find(const K &keyin)
    {
        if (keyin < key) // If the key is smaller, search the left subtree
        {
            return left->find(keyin);
        }
        else if (keyin > key) // If the key is larger, search the right subtree
        {
            return right->find(keyin);
        }
        else // Key found, return the value
        {
            return value;
        }
    }

    // Check if the current node contains a key
    bool contains(const K &keyin)
    {
        if (keyin < key) // If key is smaller, look in the left subtree
        {
            return left && left->contains(keyin);
        }
        else if (keyin > key) // If key is larger, look in the right subtree
        {
            return right && right->contains(keyin);
        }
        return true; // If key matches, return true
    }

    // Return the height of the node in the tree
    int getHeight()
    {
        return height;
    }

    // Balance the current node to ensure AVL properties
    BinaryTreeNode<K, V> *balance()
    {
        height = std::max(left ? left->height : 0, right ? right->height : 0) + 1; // Update height
        int balanceFactor = getHeightDifference(); // Calculate balance factor

        if (balanceFactor > 1) // Left-heavy
        {
            if (left->getHeightDifference() < 0) // Left-right case
            {
                left = left->leftRotate(); // Perform left rotation on left child
            }
            return rightRotate(); // Perform right rotation
        }
        if (balanceFactor < -1) // Right-heavy
        {
            if (right->getHeightDifference() > 0) // Right-left case
            {
                right = right->rightRotate(); // Perform right rotation on right child
            }
            return leftRotate(); // Perform left rotation
        }
        return this; // Node is balanced
    }

    // Perform a left rotation on the current node
    BinaryTreeNode<K, V> *leftRotate()
    {
        BinaryTreeNode<K, V> *newRoot = right;
        right = newRoot->left;
        newRoot->left = this;
        height = std::max(left ? left->height : 0, right ? right->height : 0) + 1; // Update height
        return newRoot;
    }

    // Perform a right rotation on the current node
    BinaryTreeNode<K, V> *rightRotate()
    {
        BinaryTreeNode<K, V> *newRoot = left;
        left = newRoot->right;
        newRoot->right = this;
        height = std::max(left ? left->height : 0, right ? right->height : 0) + 1; // Update height
        return newRoot;
    }

    // Calculate the difference in height between the left and right subtrees
    int getHeightDifference()
    {
        int leftHeight = left ? left->height : 0;
        int rightHeight = right ? right->height : 0;
        return leftHeight - rightHeight; // Balance factor
    }

    // Copy the tree starting from the current node
    BinaryTreeNode<K, V> *copy_tree()
    {
        BinaryTreeNode<K, V> *node = new BinaryTreeNode<K, V>(key);
        if (left) 
        {
            node->left = left->copy_tree(); // Recursively copy left subtree
        }
        if (right) 
        {
            node->right = right->copy_tree(); // Recursively copy right subtree
        }
        return node;
    }

private:
    K key;        // Key of the node
    V value;      // Value associated with the key
    BinaryTreeNode<K, V> *left, *right;  // Left and right children
    int height;   // Height of the node for AVL balancing
};
#endif
