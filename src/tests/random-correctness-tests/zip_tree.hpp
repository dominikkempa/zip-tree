#ifndef __ZIP_TREE_HPP_INCLUDED
#define __ZIP_TREE_HPP_INCLUDED

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "utils.hpp"


//=============================================================================
// Node of a zip-tree.
//=============================================================================
template<typename key_type, typename value_type>
class node {
  public:

    //=========================================================================
    // Useful typedefs.
    //=========================================================================
    typedef node<key_type, value_type> node_type;

    //=========================================================================
    // Key, value, rank, and pointers to children.
    //=========================================================================
    key_type m_key;
    value_type m_value;
    std::uint8_t m_rank;
    node_type *m_left;
    node_type *m_right;

    //=========================================================================
    // Constructor.
    //=========================================================================
    node(
        const key_type &key,
        const value_type &value,
        const std::uint8_t rank,
        node_type *left,
        node_type *right) {
      m_key = key;
      m_value = value;
      m_rank = rank;
      m_left = left;
      m_right = right;
    }
};

//=============================================================================
// This class implements a basic subset of functionality of std::set.
// It works with any KeyType as long as objects of KeyType can be compared
// using "<" operator.
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
      m_root = nullptr;
    }

    //=========================================================================
    // Destructor.
    //=========================================================================
    ~zip_tree() {
      delete_subtree(m_root);
    }

    //=========================================================================
    // Return a bool value set to true if the insertion took place and
    // false otherwise (i.e., when the key was already in the tree).
    //=========================================================================
    bool insert(
        const key_type &key,
        const value_type &value) {

      std::pair<node_type*, node_type**> p = find(key);
      if (p.first != nullptr)
        return false;

      std::uint8_t rank = random_rank();
      node_type *cur = m_root, **edgeptr = nullptr;
      while (cur != nullptr && cur->m_rank > rank) {
        if (key < cur->m_key) {
          edgeptr = &(cur->m_left);
          cur = cur->m_left;
        } else {
          edgeptr = &(cur->m_right);
          cur = cur->m_right;
        }
      }

      while (cur != nullptr && cur->m_rank == rank && cur->m_key < key) {
        edgeptr = &(cur->m_right);
        cur = cur->m_right;
      }

      std::pair<node_type*, node_type*> pp = unzip(cur, key);
      node_type *newnode = new node_type(key, value, rank, pp.first, pp.second);
      if (edgeptr == nullptr)
        m_root = newnode;
      else *edgeptr = newnode;

      return true;
    }

    //=========================================================================
    // Delete the node with a given key from the tree.
    // Return true if the deletion took place.
    //=========================================================================
    bool erase(
        const key_type &key) {
      std::pair<node_type*, node_type**> p = find(key);
      if (p.first == nullptr) return false;
      else {
        if (p.second == nullptr)
          m_root = zip(p.first->m_left, p.first->m_right);
        else *(p.second) = zip(p.first->m_left, p.first->m_right);
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
      if (p.first == nullptr)
        return std::make_pair(false, value_type());
      else return std::make_pair(true, p.first->m_value);
    }

    //=========================================================================
    // Check if a tree is a correct zip-tree, i.e., if the order of keys
    // is correct, and whether rank[left[v]] < rank[v] and
    // rank[right[v]] <= rank[v] conditions hold for every node.
    //=========================================================================
    void self_check() const {
      if (m_root != nullptr) {

        // Check keys.
        std::vector<const node_type*> nodes;
        collect_nodes(m_root, nodes);
        for (std::uint64_t i = 0; i + 1 < nodes.size(); ++i) {
          if (!(nodes[i]->m_key < nodes[i + 1]->m_key)) {
            fprintf(stderr, "\nError: check-keys failed!\n");
            print();
            std::exit(EXIT_FAILURE);
          }
        }

        // Check ranks.
        check_ranks(m_root);
      }
    }

  private:

    //=========================================================================
    // Collect, left-to-right, all nodes from subtree rooted in `x'.
    //=========================================================================
    void collect_nodes(
        const node_type *x,
        std::vector<const node_type*> &v) const {
      if (x->m_left != nullptr)
        collect_nodes(x->m_left, v);
      v.push_back(x);
      if (x->m_right != nullptr)
        collect_nodes(x->m_right, v);
    }

    //=========================================================================
    // Self-check of a subtree rooted in `x'.
    //=========================================================================
    void check_ranks(const node_type *x) const {
      if (x->m_left != nullptr) {
        check_ranks(x->m_left);
        if (x->m_left->m_rank >= x->m_rank) {
          fprintf(stderr, "\nError: check-ranks failed!\n");
          print();
          std::exit(EXIT_FAILURE);
        }
      }

      if (x->m_right != nullptr) {
        check_ranks(x->m_right);
        if (x->m_right->m_rank > x->m_rank) {
          fprintf(stderr, "\nError: check-ranks failed!\n");
          print();
          std::exit(EXIT_FAILURE);
        }
      }
    }

#if 0
    //=========================================================================
    // Zip-in two subtrees and return the root of the resulting tree.
    // We assume that any key in `x' is smaller than any key in `y'.
    //
    // This version is incorrect. The problem can be seen on the tree 
    // with (key, rank) pairs: (3, 0), (5, 1), (2, 1), (7, 5), (10, 1)
    // or on this one: (1, 0), (4, 1), (0, 1), (5, 3), (6, 0), (7, 0).
    //=========================================================================
    node_type* zip(
        node_type *x,
        node_type *y) {
      if (x == nullptr) return y;
      else if (y == nullptr) return x;
      else {
        if (x->m_rank >= y->m_rank) {
          node_type *temp = x->m_right;
          x->m_right = y;
          y->m_left = zip(temp, y->m_left);
          return x;
        } else {
          node_type *temp = y->m_left;
          y->m_left = x;
          x->m_right = zip(x->m_right, temp);
          return y;
        }
      }
    }
#else
    //=========================================================================
    // Zip-in two subtrees and return the root of the resulting tree.
    // We assume that any key in `x' is smaller than any key in `y'.
    // Note: can this function be written more nicely using recursion.
    //=========================================================================
    node_type* zip(
        node_type *x,
        node_type *y) {
      if (x == nullptr) return y;
      else if (y == nullptr) return x;
      else {
        if (x->m_rank >= y->m_rank) {
          node_type *temp = x->m_right, *partemp = x;
          while (temp != nullptr &&
              temp->m_rank >= y->m_rank && temp->m_key < y->m_key) {
            partemp = temp;
            temp = temp->m_right;
          }
          partemp->m_right = y;
          y->m_left = zip(temp, y->m_left);
          return x;
        } else {
          if (y->m_left != nullptr && y->m_left->m_rank >= x->m_rank) {
            y->m_left = zip(x, y->m_left);
            return y;
          } else {
            node_type *temp = y->m_left;
            y->m_left = x;
            x->m_right = zip(x->m_right, temp);
            return y;
          }
        }
      }
    }
#endif

    //=========================================================================
    // Split the subtree rooted in `x' into two subtrees with key smaller
    // and larger than the given `key'. It assumes that key does not occur
    // in the subtree rooted in `x'. Note: There is room for optimization.
    //=========================================================================
    std::pair<node_type*, node_type*> unzip(
        node_type *x,
        const key_type &key) {
      if (x == nullptr) return std::make_pair(nullptr, nullptr);
      else if (key < x->m_key) {
        if (x->m_left != nullptr && x->m_left->m_key < key) {
          std::pair<node_type*, node_type*> p = unzip(x->m_left->m_right, key);
          x->m_left->m_right = p.first;
          node_type *first = x->m_left;
          x->m_left = p.second;
          return std::make_pair(first, x);
        } else return std::make_pair(unzip(x->m_left, key).first, x);
      } else {
        if (x->m_right != nullptr && key < x->m_right->m_key) {
          std::pair<node_type*, node_type*> p = unzip(x->m_right->m_left, key);
          x->m_right->m_left = p.second;
          node_type *second = x->m_right;
          x->m_right = p.first;
          return std::make_pair(x, second);
        } else return std::make_pair(x, unzip(x->m_right, key).second);
      }
    }


    //=========================================================================
    // Search for a node with a given `key'.
    // Return a pointer to the node and the address of the pointer of which
    // it is the target.
    //=========================================================================
    std::pair<node_type*, node_type**> find(const key_type &key) const {
      node_type *cur = m_root, **edgeptr = nullptr; 
      while (cur != nullptr && (key < cur->m_key || cur->m_key < key)) {
        if (key < cur->m_key) {
          edgeptr = &(cur->m_left);
          cur = cur->m_left;
        } else {
          edgeptr = &(cur->m_right);
          cur = cur->m_right;
        }
      }
      return std::make_pair(cur, edgeptr);
    }

    //=========================================================================
    // Return random rank.
    //=========================================================================
    inline std::uint8_t random_rank() const {
      std::uint64_t ret = 0;
      while (utils::random_int<std::uint32_t>(0, 1) == 0)
        ++ret;
      return ret;
    }

    //=========================================================================
    // Print the subtree rooted in `p'.
    //=========================================================================
    void print(node_type* p, std::uint32_t indent) const {
      if (p != nullptr) {
        if (p->m_right != nullptr) print(p->m_right, indent + 4);
        if (indent != 0)
          std::cout << std::setw(indent) << ' ';
        std::cout << "(" << p->m_key << ", rank = "
                  << (std::uint32_t)p->m_rank << ")\n ";
        if (p->m_left != nullptr) print(p->m_left, indent + 4);
      }
    }

    //=========================================================================
    // Delete subtree rooted in `x'.
    //=========================================================================
    void delete_subtree(node_type *x) {
      if (x != nullptr) {
        delete_subtree(x->m_left);
        delete_subtree(x->m_right);
        delete x;
      }
    }
};

#endif  // __ZIP_TREE_HPP_INCLUDED
