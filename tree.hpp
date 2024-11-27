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

//version 27 attumpts 3

// Forward declarations of templated classes
template <class K, class V>
class BinaryTree; // Represents the binary search tree

template <class K, class V>
class BinaryTreeIterator; // Iterator for traversing the tree

template <class K, class V>
class BinaryTreeNode; // Represents a node in the tree



template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>; // Allows BinaryTree to access private members of the iterator

    // Constructor: Initializes the iterator with a given tree root and start flag.
    // If `start` is true, prepares the iterator to start traversing the tree.
    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start) : current(root)
    {
        if (start)
        {
            incr(); // Initialize traversal by finding the first node
        }
    }

public:
    // Overloaded inequality operator: Checks if the iterator still has nodes to visit.
    bool operator!=(BinaryTreeIterator<K, V> &other)
    {
        return current != other.current || !working_stack.empty();
    }

    // Advances the iterator to the next node in sorted order.
    void incr()
    {
        // Traverse to the leftmost node, pushing each visited node onto the stack
        while (current != nullptr)
        {
            working_stack.push(current);
            current = current->left;
        }

        // If the stack is not empty, pop the top node and move to its right child
        if (!working_stack.empty())
        {
            current = working_stack.top();
            working_stack.pop();
        }
        else
        {
            current = nullptr; // End of traversal
        }
    }

    // Overloaded pre-increment operator: Advances the iterator.
    void operator++() { incr(); }

    // Dereference operator: Retrieves the key-value pair of the current node
    // and moves to the right child for the next traversal step.
    std::pair<K, V> operator*()
    {
        auto result = std::make_pair(current->key, current->value);
        current = current->right;
        return result;
    }

private:
    BinaryTreeNode<K, V> *current; // Pointer to the current node in the tree
    std::stack<BinaryTreeNode<K, V> *> working_stack; // Stack for maintaining traversal state
};

template <class K, class V>
class BinaryTree
{
public:
    // Constructor: Initializes an empty tree with no root.
    BinaryTree() : root(nullptr) {}

    // Overloaded [] operator: 
    // - If the key exists, returns a reference to the associated value.
    // - If the key does not exist, creates a new node and returns its value.
    V &operator[](const K &key)
    {
        if (root == nullptr)
        {
            root = new BinaryTreeNode<K, V>(key); // Create a new root node if tree is empty
        }
        else
        {
            root = balanceInsert(root, key); // Insert the key while maintaining AVL balance
        }
        return root->find(key); // Find and return the value associated with the key
    }

    // Checks if the tree contains the specified key.
    bool contains(const K &key)
    {
        return root && root->contains(key); // Delegate to the root node
    }

    // Erases a node with the specified key from the tree.
    void erase(const K &key)
    {
        if (root)
        {
            root = root->erase(key); // Recursively remove the node
        }
    }

    // Destructor: Frees all nodes in the tree and cleans up resources.
    ~BinaryTree()
    {
        if (root)
        {
            root->freetree(); // Recursively delete all nodes in the tree
            delete root;      // Delete the root node
            root = nullptr;
        }
    }

    // Returns an iterator pointing to the beginning of the tree (smallest key).
    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator<K, V>(root, true);
    }

    // Returns an iterator pointing to the end of the tree (dummy node).
    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator<K, V>(nullptr, false);
    }

    // Copy constructor: Creates a deep copy of the tree.
    BinaryTree(const BinaryTree &other) : root(nullptr)
    {
        if (other.root)
        {
            root = other.root->copy_tree(); // Recursively copy all nodes
        }
    }

    // Copy assignment operator: Deep copies another tree into this tree.
    BinaryTree &operator=(const BinaryTree &other)
    {
        if (this != &other) // Avoid self-assignment
        {
            if (root)
            {
                root->freetree(); // Free the current tree
                delete root;
            }
            root = other.root ? other.root->copy_tree() : nullptr; // Copy the other tree
        }
        return *this;
    }

    // Move constructor: Transfers ownership of another tree's resources.
    BinaryTree(BinaryTree &&move) noexcept : root(move.root)
    {
        move.root = nullptr; // Nullify the moved tree's root to avoid double deletion
    }

    // Move assignment operator: Transfers ownership of another tree's resources.
    BinaryTree &operator=(BinaryTree &&move) noexcept
    {
        if (&move != this) // Avoid self-move
        {
            if (root)
            {
                root->freetree(); // Free the current tree
                delete root;
            }
            root = move.root; // Transfer ownership
            move.root = nullptr; // Nullify the moved tree's root
        }
        return *this;
    }

    // Checks if the tree is AVL-balanced.
    bool is_avl() const
    {
        return check_avl(root); // Recursively verify AVL properties
    }

protected:
    BinaryTreeNode<K, V> *root; // Pointer to the root node of the tree

    // Balances the tree after inserting a new key.
    BinaryTreeNode<K, V> *balanceInsert(BinaryTreeNode<K, V> *node, const K &key)
    {
        if (!node)
        {
            return new BinaryTreeNode<K, V>(key); // Create a new node if the subtree is empty
        }
        if (key < node->key)
        {
            node->left = balanceInsert(node->left, key); // Insert into the left subtree
        }
        else if (key > node->key)
        {
            node->right = balanceInsert(node->right, key); // Insert into the right subtree
        }
        return node->balance(); // Balance the node after insertion
    }

    // Checks if a subtree is AVL-balanced by verifying balance factors recursively.
    bool check_avl(BinaryTreeNode<K, V> *node) const
    {
        if (!node)
        {
            return true; // An empty subtree is balanced
        }

        int leftHeight = getHeight(node->left);  // Height of the left subtree
        int rightHeight = getHeight(node->right); // Height of the right subtree
        int Difference = leftHeight - rightHeight; // Balance factor

        if (std::abs(Difference) > 1)
            return false; // If balance factor exceeds 1, the subtree is unbalanced

        return check_avl(node->left) && check_avl(node->right); // Check left and right subtrees
    }
};

template <class K, class V>
class BinaryTreeNode
{
    friend class BinaryTree<K, V>;           // Allows BinaryTree to access private members
    friend class BinaryTreeIterator<K, V>;   // Allows BinaryTreeIterator to access private members

public:
    // Constructor: Initializes the node with a given key, sets height to 1, and child pointers to null.
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr), height(1) {}

    // Recursively deletes all nodes in the subtree rooted at this node.
    void freetree()
    {
        if (left)
        {
            left->freetree();  // Delete the left subtree
            delete left;       // Free memory for the left child
            left = nullptr;
        }
        if (right)
        {
            right->freetree(); // Delete the right subtree
            delete right;      // Free memory for the right child
            right = nullptr;
        }
    }

    // Erases a node with the specified key from the subtree.
    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < this->key)
        {
            if (left)
            {
                left = left->erase(k); // Recursively erase from the left subtree
            }
        }
        else if (k > this->key)
        {
            if (right)
            {
                right = right->erase(k); // Recursively erase from the right subtree
            }
        }
        else // This node matches the key
        {
            if (!left) // No left child, replace with right child
            {
                BinaryTreeNode<K, V> *temp = right;
                delete this;
                return temp;
            }
            if (!right) // No right child, replace with left child
            {
                BinaryTreeNode<K, V> *temp = left;
                delete this;
                return temp;
            }

            // Both children exist; find the in-order successor (smallest node in the right subtree)
            BinaryTreeNode<K, V> *temp = right;
            while (temp->left)
            {
                temp = temp->left;
            }

            // Swap the key and value with the successor
            std::swap(this->key, temp->key);
            std::swap(this->value, temp->value);

            // Recursively erase the successor
            right = right->erase(temp->key);
        }

        return balance(); // Balance the subtree after deletion
    }

    // Finds the node with the specified key, creating a new node if the key does not exist.
    V &find(const K &key)
    {
        if (key == this->key)
        {
            return this->value; // Found the key, return its value
        }

        if (key < this->key) // Search in the left subtree
        {
            if (!left)
            {
                left = new BinaryTreeNode<K, V>(key); // Create a new left child if it doesn't exist
            }
            return left->find(key);
        }

        if (!right) // Search in the right subtree
        {
            right = new BinaryTreeNode<K, V>(key); // Create a new right child if it doesn't exist
        }
        return right->find(key);
    }

    // Checks if the subtree contains a specific key.
    bool contains(const K &k)
    {
        if (k == key)
            return true; // Found the key
        if (k < key)
            return left && left->contains(k); // Search in the left subtree
        return right && right->contains(k);  // Search in the right subtree
    }

    // Creates a deep copy of the subtree rooted at this node.
    BinaryTreeNode<K, V> *copy_tree() const
    {
        auto new_node = new BinaryTreeNode<K, V>(this->key); // Create a new node with the same key
        new_node->value = this->value; // Copy the value
        if (this->left)
        {
            new_node->left = this->left->copy_tree(); // Recursively copy the left subtree
        }
        if (this->right)
        {
            new_node->right = this->right->copy_tree(); // Recursively copy the right subtree
        }
        return new_node;
    }

    // Balances the subtree rooted at this node using AVL rotations.
    BinaryTreeNode<K, V> *balance()
    {
        updateHeight(); // Update the height of the current node
        int balanceFactor = getBalanceFactor(); // Compute the balance factor

        // Perform rotations based on the balance factor
        if (balanceFactor > 1)
        {
            if (left && left->getBalanceFactor() < 0)
            {
                left = left->rotateLeft(); // Perform left rotation on the left child if needed
            }
            return rotateRight(); // Perform right rotation on the current node
        }
        else if (balanceFactor < -1)
        {
            if (right && right->getBalanceFactor() > 0)
            {
                right = right->rotateRight(); // Perform right rotation on the right child if needed
            }
            return rotateLeft(); // Perform left rotation on the current node
        }

        return this; // Return the balanced node
    }

    // Updates the height of the current node based on its children.
    void updateHeight()
    {
        height = 1 + std::max(getHeight(left), getHeight(right));
    }

    // Returns the balance factor of the node (difference between left and right subtree heights).
    int getBalanceFactor()
    {
        return getHeight(left) - getHeight(right);
    }

    // Performs a right rotation to balance the tree.
    BinaryTreeNode<K, V> *rotateRight()
    {
        BinaryTreeNode<K, V> *newRoot = left; // The left child becomes the new root
        left = newRoot->right; // The left child's right subtree becomes the current node's left subtree
        newRoot->right = this; // The current node becomes the right child of the new root
        updateHeight(); // Update heights
        newRoot->updateHeight();
        return newRoot; // Return the new root
    }

    // Performs a left rotation to balance the tree.
    BinaryTreeNode<K, V> *rotateLeft()
    {
        BinaryTreeNode<K, V> *newRoot = right; // The right child becomes the new root
        right = newRoot->left; // The right child's left subtree becomes the current node's right subtree
        newRoot->left = this; // The current node becomes the left child of the new root
        updateHeight(); // Update heights
        newRoot->updateHeight();
        return newRoot; // Return the new root
    }

    K key; // The key stored in the node
    V value; // The value associated with the key
    BinaryTreeNode<K, V> *left; // Pointer to the left child
    BinaryTreeNode<K, V> *right; // Pointer to the right child
    int height; // Height of the node for AVL balancing
};

// Function to get the height of a node, returning 0 for null nodes.
template <class K, class V>
int getHeight(BinaryTreeNode<K, V> *node)
{
    return node ? node->height : 0;
}

std::vector<std::string> sortfile(const std::string &filename)
{
    // Open the file in binary mode for reading
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        // Throw a runtime error if the file cannot be opened
        throw std::runtime_error("fail");
    }

    BinaryTree<std::string, int> tree; // Create a BinaryTree to store unique strings
    std::string line;
    while (std::getline(file, line)) // Read each line from the file
    {
        tree[line] = 0; // Insert the line into the tree with a default value of 0
    }

    std::vector<std::string> sortedLines; // Vector to store sorted lines
    for (const auto &[key, value] : tree) // Use the BinaryTree iterator for traversal
    {
        sortedLines.push_back(key); // Collect keys (lines) in sorted order
    }

    return sortedLines; // Return the sorted lines
}

#endif 