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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "zip_tree.hpp"


long double wallclock() {
  timeval tim;
  gettimeofday(&tim, NULL);
  return tim.tv_sec + (tim.tv_usec / 1000000.0L);
}

std::uint64_t random_int(std::uint64_t p, std::uint64_t r) {
  std::uint64_t r30 = RAND_MAX * rand() + rand();
  std::uint64_t s30 = RAND_MAX * rand() + rand();
  std::uint64_t t4  = rand() & 0xf;
  std::uint64_t r64 = (r30 << 34) + (s30 << 4) + t4;
  return p + r64 % (r - p + 1);
}

std::string random_string() {
  uint64_t hash = random_int(0,
      std::numeric_limits<std::uint64_t>::max() - 1);
  std::stringstream ss;
  ss << hash;
  return ss.str();
}

int main() {
  srand(time(0) + getpid());

  // Try different sequences of operations
  // and compare the timing results to
  // red-black trees (std::map class)
  // from c++ standard library.
  {
    typedef std::uint64_t key_type;
    typedef std::string value_type;

    // Allocate test data.
    static const std::uint64_t n_items = 4000000;
    typedef std::pair<key_type, value_type> pair_type;
    pair_type *data = new pair_type[n_items];
    for (std::uint64_t i = 0; i < n_items; ++i) {
      key_type key =
        random_int(0, std::numeric_limits<std::uint64_t>::max() - 1);
      value_type value = random_string();
      data[i] = std::make_pair(key, value);
    }

    // Test random insertions.
    fprintf(stderr, "insert(random):\n");

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;
      long double elapsed = wallclock() - start;
      fprintf(stderr, "\tredblack: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);
      long double elapsed = wallclock() - start;
      fprintf(stderr, "\tzip-tree: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
      delete tree;
    }

    // Test random insertions.
    fprintf(stderr, "insert(sorted):\n");
    std::sort(data, data + n_items);

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;
      long double elapsed = wallclock() - start;
      fprintf(stderr, "\tredblack: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);
      long double elapsed = wallclock() - start;
      fprintf(stderr, "\tzip-tree: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
      delete tree;
    }

    fprintf(stderr, "delete(random)\n");
    std::random_shuffle(data, data + n_items);

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;

      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        m.erase(data[i].first);
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tredblack: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);

      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->erase(data[i].first);
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tzip-tree: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
      delete tree;
    }

    fprintf(stderr, "delete(sorted)\n");
    std::sort(data, data + n_items);

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;

      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        m.erase(data[i].first);
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tredblack: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);

      long double start = wallclock();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->erase(data[i].first);
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tzip-tree: %.2Lf ns/op\n",
          (1000000000.L * elapsed) / n_items);
      delete tree;
    }

    fprintf(stderr, "search(random):\n");
    std::random_shuffle(data, data + n_items);

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;
      long double start = wallclock();
      std::uint64_t checksum = 0;
      for (std::uint64_t i = 0; i < n_items; ++i) {
        value_type value = m[data[i].first];
        checksum += (std::uint64_t)value[0];
      }
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tredblack: %.2Lf ns/op (checksum = %lu)\n",
          (1000000000.L * elapsed) / n_items, checksum);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);

      long double start = wallclock();
      std::uint64_t checksum = 0;
      for (std::uint64_t i = 0; i < n_items; ++i) {
        std::pair<bool, value_type> ret = tree->search(data[i].first);
        checksum += (std::uint64_t)ret.second[0];
      }
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tzip-tree: %.2Lf ns/op (checksum = %lu)\n",
          (1000000000.L * elapsed) / n_items, checksum);
      delete tree;
    }

    fprintf(stderr, "iterate-all:\n");

    // Test red-black tree.
    {
      typedef std::map<key_type, value_type> map_type;
      map_type m;
      for (std::uint64_t i = 0; i < n_items; ++i)
        m[data[i].first] = data[i].second;
      long double start = wallclock();
      std::uint64_t checksum = 0;
      for (map_type::iterator it = m.begin(); it != m.end(); ++it) {
        value_type value = it->second;
        checksum += (std::uint64_t)value[0];
      }
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tredblack: %.2Lf ns/op (checksum = %lu)\n",
          (1000000000.L * elapsed) / n_items, checksum);
    }

    // Test zip-tree.
    {
      typedef zip_tree<key_type, value_type> zip_tree_type;
      zip_tree_type *tree = new zip_tree_type();
      for (std::uint64_t i = 0; i < n_items; ++i)
        tree->insert(data[i].first, data[i].second);

      long double start = wallclock();
      std::uint64_t checksum = 0;
      for (zip_tree_type::iterator it = tree->begin(); it != tree->end(); ++it) {
        value_type value = it.value();
        checksum += (std::uint64_t)value[0];
      }
      long double elapsed = wallclock() - start;

      fprintf(stderr, "\tzip-tree: %.2Lf ns/op (checksum = %lu)\n",
          (1000000000.L * elapsed) / n_items, checksum);
      delete tree;
    }

    // Clean up.
    delete[] data;
  }
}
