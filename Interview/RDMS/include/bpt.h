#ifndef BPT_H_
#define BPT_H_

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <string>

#include "predefined.h"

namespace bpt {

/* offsets */
#define OFFSET_META 0
#define OFFSET_BLOCK OFFSET_META + sizeof(meta_t)
#define SIZE_NO_CHILDREN sizeof(leaf_node_t) - BP_ORDER * sizeof(record_t)

typedef struct {
  size_t order;
  size_t key_size;
  size_t value_size;
  size_t internal_node_num; // 内部节点数目
  size_t leaf_node_num;     // 叶子节点数目
  size_t height;            // 树高
  off_t slot;               // where to store new block
  off_t root_offset;        // where is the root of internal nodes
  off_t leaf_offset;        // where is the first leaf
} meta_t;                   // meta data for the b+ tree

struct index_t {
  key_t key;
  off_t child; // child's offset
};

// branch
struct internal_node_t {
  typedef index_t *child_t; // index_t* 表示了child的指针 这个写法没见过用于模板 MARK

  off_t parent;
  off_t next;
  off_t prev;
  size_t n; // how many children
  index_t children[BP_ORDER];
}; // internal node block

struct record_t {
  key_t key;
  value_t value;
};

// 存储真实数据的叶子节点
struct leaf_node_t {
  typedef record_t *child_t;

  off_t parent;
  off_t next;
  off_t prev;
  size_t n;
  record_t children[BP_ORDER];
}; // leaf node block

class bplus_tree {
public:
  bplus_tree(const std::string &path, bool force_empty = false);

  // abstract operations
  int search(const key_t &key, value_t *value) const; // 找到后放到value中
  int search_range(key_t *left, const key_t &right, value_t *values, size_t max,
                   bool *next = nullptr) const;

  int remove(const key_t &key);
  int insert(const key_t &key, value_t value); // TODO 移动构造
  int update(const key_t &key, value_t value);
  meta_t get_meta() const { return meta; }

private:
  void init_from_empty();
  // find index
  off_t search_index(const key_t &key) const;
  // find leaf
  off_t search_leaf(off_t index, const key_t &key) const;
  off_t search_leaf(const key_t &key) const {
    return search_leaf(search_index(key), key);
  }
  // remove internal node
  void remove_from_index(off_t offset, internal_node_t &node, const key_t &key);
  // borrow key
  /* borrow one key from other internal node */
  bool borrow_key(bool from_right, internal_node_t &borrower, off_t offset);
  /* borrow one record from other leaf */
  bool borrow_key(bool from_right, leaf_node_t &borrower);

  /* change one's parent key to another key */
  void change_parent_child(off_t parent, const key_t &o, const key_t &n);

  /* merge right leaf to left leaf */
  void merge_leafs(leaf_node_t *left, leaf_node_t *right);
  void merge_keys(index_t *where, internal_node_t &left,
                  internal_node_t &right);

  /* insert into leaf without split */
  void insert_record_no_split(leaf_node_t *leaf, const key_t &key,
                              const value_t &value);

  /* add key to the internal node */
  void insert_key_to_index(off_t offset, const key_t &key, off_t value,
                           off_t after);
  void insert_key_to_index_no_split(internal_node_t &node, const key_t &key,
                                    off_t value);

  /* change children's parent */
  void reset_index_children_parent(index_t *begin, index_t *end, off_t parent);

  template <class T> void node_create(off_t offset, T *node, T *next);
  template <class T> void node_remove(T *prev, T *node);

private:
  void open_file(const char *mode = "rb+") const {
    if (fp_level == 0)
      fp = fopen(path.c_str(), mode);
    ++fp_level;
  }

  void close_file() const {
    if (fp_level == 1)
      fclose(fp);
    --fp_level;
  }

  // read block from disk TODO use mmap
  int map(void *block, off_t offset, size_t size) const {
    open_file();
    fseek(fp, offset, SEEK_SET);
    size_t rd = fread(block, size, 1, fp);
    close_file();
    return rd - 1;
  }
  // write block to disk TODO use ummap
  int unmap(void *block, off_t offset, size_t size) const {
    open_file();
    fseek(fp, offset, SEEK_SET);
    size_t wd = fwrite(block, size, 1, fp);
    close_file();
    return wd - 1;
  }

  template <class T> int map(T *block, off_t offset) const {
    return map(block, offset, sizeof(T));
  }
  template <class T> int unmap(T *block, off_t offset) const {
    return unmap(block, offset, sizeof(T));
  }

  // alloc from disk
  off_t alloc(size_t size) {
    off_t slot = meta.slot;
    meta.slot += size;
    return slot;
  }

  off_t alloc(leaf_node_t *leaf) {
    leaf->n = 0;
    meta.leaf_node_num++;
    return alloc(sizeof(leaf_node_t));
  }

  off_t alloc(internal_node_t *node) {
    node->n = 1;
    meta.internal_node_num++;
    return alloc(sizeof(internal_node_t));
  }

  void unalloc(leaf_node_t *leaf, off_t offset) { --meta.leaf_node_num; }
  void unalloc(internal_node_t *leaf, off_t offset) {
    --meta.internal_node_num;
  }

private:
  std::string path;
  meta_t meta;

  mutable FILE *fp;
  mutable int fp_level;
};

} // namespace bpt

#endif // BPT_H_
