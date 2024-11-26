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
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>


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
    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start) : current(root)
    {
        if (start) incr();
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
        while (current)
        {
            working_stack.push(current);
            current = current->left;
        }
        if (!working_stack.empty())
        {
            current = working_stack.top();
            working_stack.pop();
        } 
        else current = nullptr;
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
        auto pair = std::make_pair(current->key, current->value);
        current = current->right;
        return pair;
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
    BinaryTree() : root(nullptr) 
    {
    }

    // The [] operation is for both getting and setting.
    // If the key exists in the tree a reference to the
    // associated value is returned.  Otherwise, it will
    // create a new tree node and return that value.

    // What you should do is first check if the root is null
    // and, if so, just create a new node.
    // then simply invoke root->find(key) and return
    // that value as find is already set up to return
    // a reference.
    V &operator[](const K &key)
    {
        if (!root)
        {
            root = new BinaryTreeNode<K, V>(key);
        }
        else
        {
            root = manageInsert(root, key);
        }

        return root->find(key);
    }

    // This should return false if the tree
    // is empty, otherwise return root->contains
    bool contains(const K &key)
    {
        return root && root->contains(key);
    }

    // Erases a node if a key matches.  If the
    // key does not match it simply is an operation
    // that does nothing.
    void erase(const K &key)
    {
        if (root) root = root->erase(key);
    }

    // And the destructor for the binary tree.
    // In order to prevent memory leaks and keep with
    // the C++ "RAII" convention, it should see
    // if there is a root and, if so, call freetree()
    // on the root.
    ~BinaryTree()
    {
        if (root)
        {
            root->freetree();
            delete root;
            root = nullptr;
        }
    }

    // This returns the iterators.
    BinaryTreeIterator<K, V> begin()
    {
        return BinaryTreeIterator<K, V>(root, true);
    }
    BinaryTreeIterator<K, V> end()
    {
        return BinaryTreeIterator<K, V>(nullptr, false);
    }

    BinaryTree(const BinaryTree &other) : root(nullptr)
    {
        if (other.root)
        {
            root = other.root->deepCopy();
        }
    }

    BinaryTree &operator=(const BinaryTree &other)
    {
        if (this != &other)
        {
            if (root)
            {
                root->freetree();
                delete root;
            }
            root = other.root ? other.root->deepCopy() : nullptr;
        }
        return *this;
    }

    // Realization: Need to add deliberate disabling of copy and make
    // the move eliminate the existing pointer (give the BinaryTree move
    // semantics rather than copy semantics)
    BinaryTree(BinaryTree &&move) noexcept : root(move.root)
    {
        move.root = nullptr;
    }

    // Similarly for the assigment operator
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

    bool is_avl() const
    {
        return check_avl(root);
    }

protected:
    BinaryTreeNode<K, V> *root;

    BinaryTreeNode<K, V> *manageInsert(BinaryTreeNode<K, V>* node, const K& key)
    {
        if (!node)
        {
            return new BinaryTreeNode<K, V>(key);
        }
        if (key < node->key)
        {
            node->left = manageInsert(node->left, key);
        }
        else if (key > node->key)
        {
            node->right = manageInsert(node->right, key);
        }
        return node->rebalance();
    }

    bool check_avl(BinaryTreeNode<K, V> *node) const
    {
        if (!node) return true;
        int levelDifference = getLevel<K, V>(node->left) - getLevel<K, V>(node->right);
        if (std::abs(levelDifference) > 1) return false;

        return check_avl(node->left) && check_avl(node->right);
    }

};

// And the class for the binary tree node itself.
template <class K, class V>
class BinaryTreeNode
{
    friend class BinaryTree<K, V>;
    friend class BinaryTreeIterator<K, V>;

public:
    // The constructor, it simply setts the key and the left/right pointers.
    // Data defaults to whatever the default value is for the data type.
    BinaryTreeNode(const K &keyin) : key(keyin), left(nullptr), right(nullptr), level(1) 
    {
    }

    // This should recursively free the tree.
    // It should call freetree on left and 
    // right and then, as the last act,
    // call delete on this.

    // Yes, you can "suicide" an object in C++,
    // and this is a case where you want to do it.
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



    // Removing a node from a binary tree, returning
    // a pointer to the now modified tree.

    // If you aren't at the node you are looking for, just set
    // your new left or right pointer (as approriate, and if it exists) 
    // to the  result of recursively calling erase, and then return this.

    // If this node does match the key however, check the
    // child nodes.  If there is no left, assign the right node to a temporary
    // value, delete this, and return the right node.
    // If there is no right node, do the same thing.
    // If there is both a left and right node, check if the left node
    // doesn't have a right node of its own.  If so, set that node's right
    // to the current right, assign left to a temporary, and delete this and
    // return that left node.

    // Finally, if the left node DOES have a right node, keep
    // following the right hand side of that subtree until you
    // find a node without a right hand branch.  In that case set
    // the node above it's right to that node's left, assign that
    // node to a temporary, have its left point to the current node's left
    // its right to the current node's right, delete this and return that
    // node.
    BinaryTreeNode<K, V> *erase(const K &k)
    {
        if (k < this->key)
        {
            if (left) left = left->erase(k);
        }
        else if (k > this->key)
        {
            if (right) right = right->erase(k);
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
            while (temp->left) temp = temp->left;
            std::swap(this->key, temp->key);
            std::swap(this->value, temp->value);
            right = right->erase(temp->key);
        }
        return rebalance();
    }

    // This seeks to recursively find a key, creating
    // a new node if necessary.
    // If k == this node's key, return the value.
    // If k < this key it is to the left.
    // If there is no left node, create it with k as the key.
    // Then recursively return find on the left.  Similar for
    // the right.
    V &find(const K &k)
    {
        if (k == this->key) return value;
        if (k < this->key)
        {
            if (!left) left = new BinaryTreeNode<K, V>(k);
            return left->find(k);
        }
        else
        {
            if (!right) right = new BinaryTreeNode<K, V>(k);
            return right->find(k);
        }
    }

    // And contains is a recursive search that doesn't
    // create new nodes, just checks if the key exists.
    bool contains(const K &k)
    {
        if (k == this->key) return true;
        if (k < this->key) return left && left->contains(k);
        return right && right->contains(k);
    }


    BinaryTreeNode<K, V> *deepCopy() const
    {
        BinaryTreeNode<K, V> *newNode = new BinaryTreeNode<K, V>(this->key);
        newNode->value = this->value;
        if (this->left) newNode->left = this->left->deepCopy();
        if (this->right) newNode->right = this->right->deepCopy();
        return newNode;
    }

    BinaryTreeNode<K, V> *rebalance()
    {
        updateLevel();
        int levelDifference = getLevel<K, V>(left) - getLevel<K, V>(right);

        if (levelDifference > 1)
        {
            if (getLevel<K, V>(left->right) > getLevel<K, V>(left->left))
            {
                left = left->toLeft();
            }
            return toRight();
        }
        else if (levelDifference < -1)
        {
            if (getLevel<K, V>(right->left) > getLevel<K, V>(right->right))
            {
                right = right->toRight();
            }
            return toLeft();
        }
        return this;
    }


    void updateLevel()
    {
        level = 1 + std::max(getLevel<K, V>(left), getLevel<K, V>(right));
    }

    BinaryTreeNode<K, V> *toLeft()
    {
        BinaryTreeNode<K, V> *newRoot = right;
        right = newRoot->left;
        newRoot->left = this;
        updateLevel();
        newRoot->updateLevel();
        return newRoot;
    }

    BinaryTreeNode<K, V> *toRight()
    {
        BinaryTreeNode<K, V> *newRoot = left;
        left = newRoot->right;
        newRoot->right = this;
        updateLevel();
        newRoot->updateLevel();
        return newRoot;
    }

    K key;
    V value;
    BinaryTreeNode<K, V> *left;
    BinaryTreeNode<K, V> *right;
    int level;
};

template <class K, class V>
int getLevel(BinaryTreeNode<K, V> *node)
{
    return node ? node->level : 0;
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