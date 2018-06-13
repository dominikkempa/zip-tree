#ifndef __ZIP_TREE_HPP_INCLUDED
#define __ZIP_TREE_HPP_INCLUDED

#include <cstdlib>
#include <cstdint>
#include <iostream>


//=============================================================================
// Node of a zip-tree.
//=============================================================================
template<typename key_type, typename value_type>
class node {
  private:

    //=========================================================================
    // Define common aliases.
    //=========================================================================
    typedef node<key_type, value_type> node_type;

  public:

    //=========================================================================
    // Key, value, rank, and pointers.
    //=========================================================================
    key_type m_key;
    value_type m_value;
    std::uint8_t m_rank;
    node_type *m_left;
    node_type *m_right;
    node_type *m_par;

    //=========================================================================
    // Constructor.
    //=========================================================================
    node(
        const key_type &key,
        const value_type &value,
        const std::uint8_t rank,
        node_type *left,
        node_type *right,
        node_type *par) {
      m_key = key;
      m_value = value;
      m_rank = rank;
      m_left = left;
      m_right = right;
      m_par = par;
    }
};

//=============================================================================
// Simple implementation of zip-tree. It works with any key_type as
// long as objects of key_type can be compared using "<" operator.
//=============================================================================
template<typename key_type, typename value_type>
class zip_tree {
  private:

    //=========================================================================
    // Define common aliases.
    //=========================================================================
    typedef node<key_type, value_type> node_type;

    //=========================================================================
    // Pointer to the root of the tree.
    //=========================================================================
    node_type *m_root;

  public:

    //=========================================================================
    // Constructor.
    //=========================================================================
    zip_tree() {
      m_root = 0;
    }

    //=========================================================================
    // Destructor.
    //=========================================================================
    ~zip_tree() {
      delete_subtree(m_root);
    }

    //=========================================================================
    // Insert a node with a given (key, value) pair into the tree.
    // Return true if the insertion took place and false otherwise (the
    // key was already in the tree). This is an optimized variant of the
    // insertion which does only a single downward pass in the tree.
    //=========================================================================
    bool insert(const key_type &key, const value_type &value) {
      std::uint8_t rank = random_rank();
      node_type *cur = m_root, *par = 0, **edgeptr = 0;
      while (cur && cur->m_rank > rank) {
        if (key < cur->m_key) {
          par = cur;
          edgeptr = &(cur->m_left);
          cur = cur->m_left;
        } else if (cur->m_key < key) {
          par = cur;
          edgeptr = &(cur->m_right);
          cur = cur->m_right;
        } else return false;
      }
      while (cur && cur->m_rank == rank && cur->m_key < key) {
        par = cur;
        edgeptr = &(cur->m_right);
        cur = cur->m_right;
      }
      std::pair<node_type*, node_type*> p = unzip(cur, key);
      if (cur && !p.first && !p.second) return false;
      node_type *newnode =
        new node_type(key, value, rank, p.first, p.second, par);
      if (p.first) p.first->m_par = newnode;
      if (p.second) p.second->m_par = newnode;
      if (!edgeptr) m_root = newnode;
      else *edgeptr = newnode;
      return true;
    }

    //=========================================================================
    // Delete the node with a given key from the tree.
    // Return true if the deletion took place.
    //=========================================================================
    bool erase(const key_type &key) {
      std::pair<node_type*, node_type**> p = find(key);
      if (!p.first) return false;
      else {
        if (!p.second) {
          m_root = zip(p.first->m_left, p.first->m_right);
          if (m_root)
            m_root->m_par = 0;
        } else {
          node_type *par = p.first->m_par;
          *(p.second) = zip(p.first->m_left, p.first->m_right);
          if (*(p.second))
            (*(p.second))->m_par = par;
        }
        delete p.first;
        return true;
      }
    }

    //=========================================================================
    // Print the tree.
    //=========================================================================
    void print() const {
      print(m_root, 0);
    }

    //=========================================================================
    // Search for a given key in the tree.
    // Return a pair containing the key and its value.
    //=========================================================================
    std::pair<bool, value_type> search(const key_type &key) const {
      std::pair<node_type*, node_type**> p = find(key);
      if (!p.first) return std::make_pair(false, value_type());
      else return std::make_pair(true, p.first->m_value);
    }

    //=========================================================================
    // Check if a tree is a correct zip-tree, i.e., if the order of keys
    // is correct, and whether rank[left[v]] < rank[v] and
    // rank[right[v]] <= rank[v] conditions hold for every node.
    //=========================================================================
    void check_correctness() const {
      if (m_root) {
        check_keys(m_root);
        check_ranks(m_root);
        check_parents(m_root);
        if (m_root->m_par != 0) {
          std::cerr << "\nError: m_root->m_par != 0\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

  public:

    //=========================================================================
    // Very simple non-const iterator.
    //=========================================================================
    class iterator {
      private:
        node_type *m_ptr;

      public:
        iterator(node_type *x)
          : m_ptr(x) {}

        iterator()
          : m_ptr(nullptr) {}

        const key_type& key() const {
          return m_ptr->m_key;
        }

        value_type& value() {
          return m_ptr->m_value;
        }

        inline iterator& operator++() {
          if (!m_ptr) {
            std::cerr << "\nError: ++ on NULL iterator\n";
            std::exit(EXIT_FAILURE);
          }
          m_ptr = next(m_ptr);
          return *this;
        }

        inline iterator& operator--() {
          if (!m_ptr)
            m_ptr = max_node(m_root);
          else m_ptr = prev(m_ptr);
          return *this;
        }

        inline iterator operator++(int) {
          if (!m_ptr) {
            std::cerr << "\nError: ++ on NULL iterator\n";
            std::exit(EXIT_FAILURE);
          }
          iterator ret = *this;
          m_ptr = next(m_ptr);
          return ret;
        }

        inline iterator operator--(int) {
          iterator ret = *this;
          if (!m_ptr)
            m_ptr = max_node(m_root);
          else m_ptr = prev(m_ptr);
          return ret;
        }

        bool operator == (const iterator &it) const {
          return m_ptr == it.m_ptr;
        }

        bool operator != (const iterator &it) const {
          return m_ptr != it.m_ptr;
        }
    };

    iterator begin() {
      return iterator(min_node(m_root));
    }

    iterator end() {
      return iterator(nullptr);
    }

  private:

    //=========================================================================
    // Zip-in two subtrees and return the root of the resulting tree.
    // We assume that any key in `x' is smaller than any key in `y'.
    //=========================================================================
    node_type* zip(node_type *x, node_type *y) {
      if (!x) return y;
      if (!y) return x;
      if (x->m_rank >= y->m_rank) {
        node_type *xright = x->m_right;
        if (xright && xright->m_rank >= y->m_rank) {
          x->m_right = zip(xright, y);
          x->m_right->m_par = x;
        } else {
          x->m_right = y;
          y->m_par = x;
          y->m_left = zip(xright, y->m_left);
          if (y->m_left)
            y->m_left->m_par = y;
        }
        return x;
      } else {
        node_type *yleft = y->m_left;
        if (yleft && yleft->m_rank >= x->m_rank) {
          y->m_left = zip(x, yleft);
          y->m_left->m_par = y;
        } else {
          y->m_left = x;
          x->m_par = y;
          x->m_right = zip(x->m_right, yleft);
          if (x->m_right)
            x->m_right->m_par = x;
        }
        return y;
      }
    }

    //=========================================================================
    // Split the subtree rooted in `x' into two subtrees with keys smaller
    // and larger than the given `key'. If `key' occurs in subtree `x' then
    // function returns a pair (nullptr, nullptr) and tree remains unchanged.
    // NOTE: Is a more elegant/shorter implementation of this function possible?
    //=========================================================================
    std::pair<node_type*, node_type*> unzip(
        node_type *x,
        const key_type &key) {
      if (!x) return std::make_pair(nullptr, nullptr);
      else if (key < x->m_key) {
        node_type *xleft = x->m_left;
        if (xleft && xleft->m_key < key) {
          std::pair<node_type*, node_type*> p = unzip(xleft->m_right, key);
          if (xleft->m_right && !p.first && !p.second) return p;
          xleft->m_right = p.first;
          if (p.first)
            p.first->m_par = xleft;
          x->m_left = p.second;
          if (p.second)
            p.second->m_par = x;
          return std::make_pair(xleft, x);
        } else {
          std::pair<node_type*, node_type*> p = unzip(xleft, key);
          if (xleft && !p.first && !p.second) return p;
          else return std::make_pair(p.first, x);
        }
      } else if (x->m_key < key) {
        node_type *xright = x->m_right;
        if (xright && key < xright->m_key) {
          std::pair<node_type*, node_type*> p = unzip(xright->m_left, key);
          if (xright->m_left && !p.first && !p.second) return p;
          xright->m_left = p.second;
          if (p.second)
            p.second->m_par = xright;
          x->m_right = p.first;
          if (p.first)
            p.first->m_par = x;
          return std::make_pair(x, xright);
        } else {
          std::pair<node_type*, node_type*> p = unzip(xright, key);
          if (xright && !p.first && !p.second) return p;
          else return std::make_pair(x, p.second);
        }
      } else return std::make_pair(nullptr, nullptr);
    }

    //=========================================================================
    // Search for a node with a given `key'. Return a pointer to the node and
    // the address of the pointer of which it is the target.
    //=========================================================================
    std::pair<node_type*, node_type**> find(const key_type &key) const {
      node_type *cur = m_root, **edgeptr = 0; 
      while (cur) {
        if (key < cur->m_key) {
          edgeptr = &(cur->m_left);
          cur = cur->m_left;
        } else if (cur->m_key < key) {
          edgeptr = &(cur->m_right);
          cur = cur->m_right;
        } else return std::make_pair(cur, edgeptr);
      }
      return std::make_pair(nullptr, nullptr);
    }

    //=========================================================================
    // Return random rank.
    //=========================================================================
    std::uint8_t random_rank() const {

      // Note the static below. This variable
      // will keep its value between calls, which
      // reduces the number of calls to rand().
      static std::uint32_t random_bits = 0;
      while (!random_bits)
        random_bits = rand();
      std::uint8_t rank = __builtin_ctz(random_bits);
      random_bits >>= (rank + 1);
      return rank;
    }

    //=========================================================================
    // Delete subtree rooted in `x'.
    //=========================================================================
    void delete_subtree(node_type *x) {
      if (x) {
        delete_subtree(x->m_left);
        delete_subtree(x->m_right);
        delete x;
      }
    }

    //=========================================================================
    // Print the subtree rooted in `x'.
    //=========================================================================
    void print(const node_type *x, std::uint32_t indent) const {
      if (x) {
        if (x->m_right) print(x->m_right, indent + 4);
        for (std::uint64_t j = 0; j < indent; ++j) std::cout << ' ';
        std::cout << "(" << x->m_key << ", rank = " << (int)x->m_rank << ")\n ";
        if (x->m_left) print(x->m_left, indent + 4);
      }
    }

    //=========================================================================
    // Check if all nodes in the subtree `x' have correctly ordered keys.
    //=========================================================================
    void check_keys(const node_type *x) const {
      if (x->m_left) check_keys_left(x->m_left, x->m_key);
      if (x->m_right) check_keys_right(x->m_right, x->m_key);
    }

    //=========================================================================
    // Check if all keys in the subtree rooted in `x' are < key.
    //=========================================================================
    void check_keys_left(const node_type *x, const key_type &key) const {
      if (!(x->m_key < key)) {
        std::cerr << "\nError: check_keys_left failed!\n";
        std::exit(EXIT_FAILURE);
      }
      if (x->m_left) check_keys_left(x->m_left, x->m_key);
      if (x->m_right) check_keys(x->m_right, x->m_key, key);
    }

    //=========================================================================
    // Check if all the keys in the subtree rooted in `x' are > key.
    //=========================================================================
    void check_keys_right(const node_type *x, const key_type &key) const {
      if (!(key < x->m_key)) {
        std::cerr << "\nError: check_keys_right_ failed!\n";
        std::exit(EXIT_FAILURE);
      }
      if (x->m_left) check_keys(x->m_left, key, x->m_key);
      if (x->m_right) check_keys_right(x->m_right, x->m_key);
    }

    //=========================================================================
    // Check if all the keys in the subtree rooted in `x' are (strictly)
    // between `key_left' and `key_right'.
    //=========================================================================
    void check_keys(
        const node_type *x,
        const key_type &key_left,
        const key_type &key_right) const {
      if (!(key_left < x->m_key) || !(x->m_key < key_right)) {
        std::cerr << "\nError: check_keys failed!\n";
        std::exit(EXIT_FAILURE);
      }
      if (x->m_left) check_keys(x->m_left, key_left, x->m_key);
      if (x->m_right) check_keys(x->m_right, x->m_key, key_right);
    }

    //=========================================================================
    // Check correctness of ranks in a subtree rooted in `x'.
    //=========================================================================
    void check_ranks(const node_type *x) const {
      if (x->m_left) {
        check_ranks(x->m_left);
        if (x->m_left->m_rank >= x->m_rank) {
          std::cerr << "\nError: check_ranks failed!\n";
          print();
          std::exit(EXIT_FAILURE);
        }
      }

      if (x->m_right) {
        check_ranks(x->m_right);
        if (x->m_right->m_rank > x->m_rank) {
          std::cerr << "\nError: check_ranks failed!\n";
          print();
          std::exit(EXIT_FAILURE);
        }
      }
    }

    //=========================================================================
    // Check correctness of parent pointer in the subtree rooted in `x'.
    //=========================================================================
    void check_parents(const node_type *x) const {
      if (x->m_left) {
        check_parents(x->m_left);
        if (x->m_left->m_par != x) {
          std::cerr << "\nError: check_parents failed!\n";
          std::exit(EXIT_FAILURE);
        }
      }
      if (x->m_right) {
        check_parents(x->m_right);
        if (x->m_right->m_par != x) {
          std::cerr << "\nError: check_parents failed!\n";
          std::exit(EXIT_FAILURE);
        }
      }
    }

    //=========================================================================
    // Compute the next node in inorder. We assume x != nullptr.
    //=========================================================================
    static node_type* next(node_type *x) {
      if (x->m_right)
        return min_node(x->m_right);
      else {
        while (x->m_par && x->m_par->m_right == x)
          x = x->m_par;
        if (!(x->m_par))
          return nullptr;
        else
          return x->m_par;
      }
    }

    //=========================================================================
    // Compute the prev node in inorder. We assume x != nullptr.
    //=========================================================================
    static node_type* prev(node_type *x) {
      if (x->m_left)
        return max_node(x->m_left);
      else {
        while (x->m_par && x->m_par->m_left == x)
          x = x->m_par;
        if (!(x->m_par))
          return nullptr;
        else return x->m_par;
      }
    }

    //=========================================================================
    // Return the leftmost node in the subtree rooted in `x'.
    //=========================================================================
    static node_type* min_node(node_type *x) {
      if (!x) return nullptr;
      while (x->m_left)
        x = x->m_left;
      return x;
    }

    //=========================================================================
    // Return the rightmost node in the subtree rooted in `x'.
    //=========================================================================
    static node_type* max_node(node_type *x) {
      if (!x) return nullptr;
      while (x->m_right)
        x = x->m_right;
      return x;
    }

};

#endif  // __ZIP_TREE_HPP_INCLUDED
