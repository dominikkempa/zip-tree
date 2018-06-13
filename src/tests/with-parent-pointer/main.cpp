#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <map>
#include <sstream>
#include <limits>
#include <vector>
#include <ctime>
#include <unistd.h>

#include "zip_tree.hpp"


std::uint64_t random_int(std::uint64_t p, std::uint64_t r) {
  std::uint64_t r30 = RAND_MAX * rand() + rand();
  std::uint64_t s30 = RAND_MAX * rand() + rand();
  std::uint64_t t4  = rand() & 0xf;
  std::uint64_t r64 = (r30 << 34) + (s30 << 4) + t4;
  return p + r64 % (r - p + 1);
}

std::string random_string() {
  uint64_t hash = random_int(0, std::numeric_limits<std::uint64_t>::max() - 1);
  std::stringstream ss;
  ss << hash;
  return ss.str();
}

int main() {
  srand(time(0) + getpid());

  // Check random sequences of operations
  // and compare the result to std::map.
  {
    typedef std::uint64_t key_type;
    typedef std::string value_type;
    typedef zip_tree<key_type, value_type> zip_tree_type;

    static const std::uint64_t n_tests = 200000;
    for (std::uint64_t i = 0; i < n_tests; ++i) {
      if ((i + 1) % 100 == 0)
        fprintf(stderr, "testing: %.2Lf%%\r", 100.L * (i + 1) / n_tests);

      zip_tree_type *tree = new zip_tree_type();
      std::map<key_type, value_type> s;
      for (std::uint64_t j = 0; j < 100; ++j) {
        std::uint64_t op = random_int(0, 2);
        if (op == 0) {
          std::uint64_t key = random_int(0, 10);
          std::string value = random_string();
          bool res = tree->insert(key, value);
          if (s.find(key) == s.end()) {
            s[key] = value;
            if (res == false) {
              fprintf(stderr, "\nError: wrong insertion result\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            if (res == true) {
              fprintf(stderr, "\nError: wrong insertion result\n");
              std::exit(EXIT_FAILURE);
            }
          }
        } else if (op == 1) {
          std::uint64_t key = random_int(0, 10);
          bool res = tree->erase(key);
          if (s.find(key) != s.end()) {
            s.erase(key);
            if (res == false) {
              fprintf(stderr, "\nError: wrong erase result\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            if (res == true) {
              fprintf(stderr, "\nError: wrong erase result\n");
              std::exit(EXIT_FAILURE);
            }
          }
        } else {
          std::uint64_t key = random_int(0, 10);
          std::map<key_type, value_type>::iterator it = s.find(key);
          if (it != s.end()) {
            std::pair<bool, value_type> p = tree->search(key);
            if (p.first == false) {
              fprintf(stderr, "\nError: key not found in zip tree!\n");
              std::exit(EXIT_FAILURE);
            } else if (p.second != it->second) {
              fprintf(stderr, "\nError: value different than in zip tree!\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            std::pair<bool, value_type> p = tree->search(key);
            if (p.first == true) {
              fprintf(stderr, "\nError: key is found in zip tree!\n");
              std::exit(EXIT_FAILURE);
            }
          }
        }

        {
          std::map<key_type, value_type>::iterator it2 = s.begin();
          for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it) {
            if (it.key() != it2->first ||
                it.value() != it2->second) {
              fprintf(stderr, "\nError: zip tree iterators failed\n");
              std::exit(EXIT_FAILURE);
            }
            ++it2;
          }
          if (it2 != s.end()) {
            fprintf(stderr, "\nError: zip tree iterators failed\n");
            std::exit(EXIT_FAILURE);
          }
        }

        tree->check_correctness();
      }

      delete tree;
    }
    fprintf(stderr, "\n");
  }

  // Same check, but even more paranoid: we simulate
  // all operations manually using std::vector.
  {
    typedef std::uint64_t key_type;
    typedef std::string value_type;
    typedef zip_tree<key_type, value_type> zip_tree_type;

    static const std::uint64_t n_tests = 200000;
    for (std::uint64_t i = 0; i < n_tests; ++i) {
      if ((i + 1) % 100 == 0)
        fprintf(stderr, "testing: %.2Lf%%\r", 100.L * (i + 1) / n_tests);

      zip_tree_type *tree = new zip_tree_type();
      typedef std::pair<key_type, value_type> pair_type;
      std::vector<pair_type> v;
      for (std::uint64_t j = 0; j < 100; ++j) {
        std::uint64_t op = random_int(0, 2);
        if (op == 0) {
          std::uint64_t key = random_int(0, 10);
          std::string value = random_string();
          bool res = tree->insert(key, value);
          bool found = false;
          for (std::uint64_t t = 0; t < v.size(); ++t) {
            if (v[t].first == key) {
              found = true;
              break;
            }
          }
          if (found == false) {
            v.push_back(std::make_pair(key, value));
            std::sort(v.begin(), v.end());
            if (res == false) {
              fprintf(stderr, "\nError: wrong insertion result\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            if (res == true) {
              fprintf(stderr, "\nError: wront insertion result\n");
              std::exit(EXIT_FAILURE);
            }
          }
        } else if (op == 1) {
          std::uint64_t key = random_int(0, 10);
          bool res = tree->erase(key);
          bool found = false;
          std::uint64_t idx = 0;
          for (std::uint64_t t = 0; t < v.size(); ++t) {
            if (v[t].first == key) {
              found = true;
              idx = t;
              break;
            }
          }
          if (found == true) {
            v.erase(v.begin() + idx);
            if (res == false) {
              fprintf(stderr, "\nError: wrong erase result\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            if (res == true) {
              fprintf(stderr, "\nError: wrong erase result\n");
              std::exit(EXIT_FAILURE);
            }
          }
        } else {
          std::uint64_t key = random_int(0, 10);
          bool found = false;
          std::uint64_t idx = 0;
          for (std::uint64_t t = 0; t < v.size(); ++t) {
            if (v[t].first == key) {
              found = true;
              idx = t;
              break;
            }
          }
          if (found == true) {
            std::pair<bool, value_type> p = tree->search(key);
            if (p.first == false) {
              fprintf(stderr, "\nError: key not found in zip tree!\n");
              std::exit(EXIT_FAILURE);
            } else if (p.second != v[idx].second) {
              fprintf(stderr, "\nError: value different than in zip tree!\n");
              std::exit(EXIT_FAILURE);
            }
          } else {
            std::pair<bool, value_type> p = tree->search(key);
            if (p.first == true) {
              fprintf(stderr, "\nError: key is found in zip tree!\n");
              std::exit(EXIT_FAILURE);
            }
          }
        }

        {
          bool ok = true;
          std::uint64_t tree_elems = 0;
          for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it) {
            if (it.key() != v[tree_elems].first ||
                it.value() != v[tree_elems].second) {
              ok = false;
              break;
            }
            ++tree_elems;
          }
          if (tree_elems != v.size())
            ok = false;
          if (!ok) {
            std::cerr << "\nError: zip tree iterators failed\n";
            std::cerr << "v:\n";
            for (std::uint64_t tt = 0; tt < v.size(); ++tt)
              std::cerr << "\t" << "key = " << v[tt].first << ", value = "
                << v[tt].second << "\n";
            std::cerr << "tree:\n";
            for (zip_tree_type::iterator it = tree->begin();
                it != tree->end(); ++it) {
              std::cerr << "\t" << "key = " << it.key() << ", value = "
                << it.value() << "\n";
            }
            std::exit(EXIT_FAILURE);
          }
        }

        tree->check_correctness();
      }

      delete tree;
    }

    fprintf(stderr, "\n");
  }

  {
    typedef std::uint64_t key_type;
    typedef std::string value_type;
    typedef zip_tree<key_type, value_type> zip_tree_type;

    zip_tree_type *tree = new zip_tree_type();
    tree->insert(5, "five");
    tree->insert(6, "fix");
    tree->insert(2, "two");
    tree->insert(10, "ten");
    tree->erase(6);
    tree->insert(7, "seven");
    tree->erase(5);
    tree->insert(9, "nine");

    std::cout << "tree:\n";
    for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it) {
      std::cout << "\t" << "key = " << it.key() << ", value = "
        << it.value() << "\n";
    }

    for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it)
      it.value() = "newvalue";

    std::cout << "tree:\n";
    for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it) {
      std::cout << "\t" << "key = " << it.key() << ", value = "
        << it.value() << "\n";
    }

    delete tree;
  }
}
