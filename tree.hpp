#ifndef TREE_HPP
#define TREE_HPP

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
// Tree Iterator Class
template <class K, class V>
class BinaryTreeIterator : public std::input_iterator_tag
{
    friend class BinaryTree<K, V>;
    // The constructor.  For end you don't actually
    // need to do anything (the end is effectively a dummy).
    // For the start iterator, you should set current to root
    // and then call incr() so that the iterator is ready to
    // actually start the traversal

    BinaryTreeIterator(BinaryTreeNode<K, V> *root, bool start):current(root)
    {
       
        if(start){
            incr();
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
    void operator++() { incr(); }

    // And this visits the node itself...
    // In visiting the node, it should
    // both set current to be the current node's right
    // and then return a pair constructed from the old
    // current node's key and value.

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
        if (root == nullptr)
        {
            root = new BinaryTreeNode<K, V>(key);
        }
        else
        {
            root = balanceInsert(root, key);
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

    BinaryTree(const BinaryTree &other) : root(nullptr)
    {
        if (other.root)
        {
            root = other.root->copy_tree();
        }
        
    }

    BinaryTree &operator=(const BinaryTree &other)
    {
        if (this != &other)
        {
            if (root){
                root->freetree();
                delete root;
            }
            root = other.root ? other.root->copy_tree() : nullptr;
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
    bool is_avl() const
    {
        return check_avl(root);
    }

protected:
    BinaryTreeNode<K, V> *root;

    BinaryTreeNode<K, V> *balanceInsert(BinaryTreeNode<K, V> *node, const K &key)
    {
        if (!node)
        {
            return new BinaryTreeNode<K, V>(key);
        }
        if (key < node->key)
        {
            node->left = balanceInsert(node->left, key);
        }
        else if (key > node->key)
        {
            node->right = balanceInsert(node->right, key);
        }
        return node->balance();
    }
    bool check_avl(BinaryTreeNode<K, V> *node) const
    {
        if (!node){
            return true;}

        // 计算左右子树的高度差（平衡因子）
        int leftHeight = getHeight(node->left);
        int rightHeight = getHeight(node->right);
        int Difference = leftHeight - rightHeight; // 这个是平衡因子

        // 如果平衡因子的绝对值大于1，则树不平衡
        if (std::abs(Difference) > 1)
            return false;

        // 递归检查左右子树
        return check_avl(node->left) && check_avl(node->right);
    }

   

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

        bool contains(const K &k)
        {
            if (k == key)
                return true;
            if (k < key)
                return left && left->contains(k);
            return right && right->contains(k);
        }

        BinaryTreeNode<K, V> *copy_tree() const
        {
            
            auto new_node = new BinaryTreeNode<K, V>(this->key);
            new_node->value = this->value;
            if (this->left){
                new_node->left = this->left->copy_tree();}
            if (this->right){
                new_node->right = this->right->copy_tree();}
            return new_node;
        }
    };

    

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
        else if (balanceFactor < -1)
        {
            if (right && right->getBalanceFactor() > 0)
            {
                right = right->rotateRight();
            }
            return rotateLeft();
        }
     

        return this;
    }

    void updateHeight()
    {
        height = 1 + std::max(getHeight<K, V>(left), getHeight<K, V>(right));
    }

    int getBalanceFactor()
    {
        return getHeight(left) - getHeight(right);
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
        throw std::runtime_error("fail");
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
