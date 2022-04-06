#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <list>
#include <sys/types.h>

#include "../include/bpt.h"

using std::binary_search;
using std::lower_bound;
using std::swap;
using std::upper_bound;

namespace bpt {
OPERATOR_KEYCMP(index_t);
OPERATOR_KEYCMP(record_t);

// helper iterating function
template<class T> inline typename T::child_t begin(T &node) { return node.children; }
template<class T> inline typename T::child_t end(T &node) { return node.children + node.n; }

// helper searching function
inline index_t *find(internal_node_t &node, const key_t &key) {
  return upper_bound(begin(node), end(node) - 1, key); // 查找[begin, end)区域中第一个大于 key 的元素
}
inline record_t *find(leaf_node_t &node, const key_t &key) {
  return lower_bound(begin(node), end(node) - 1, key); // 在 [begin, end) 区域内查找不小于 key 的元素
}

  bplus_tree::bplus_tree(const std::string& p, bool force_empty) :
  path(p), fp(nullptr), fp_level(0){
    if (!force_empty) {
      // read tree form file
      if (map(&meta, OFFSET_META) != 0) {
        force_empty = true;
      }
    }

    if (force_empty) {
      open_file("w+");
      init_from_empty();
      close_file();
    }
  }

  int bplus_tree::search(const key_t &key, value_t *value) const {
    leaf_node_t leaf;
    map(&leaf, search_leaf(key)); // 先从头找到key的索引 在从索引到的branch中找到包含key的leaf
    // 在叶子中寻找
    record_t *record = find(leaf, key);
    if (record != leaf.children+leaf.n) {
      *value = record->value;
      return keycmp(record->key, key);
    } else {
      return -1;
    }
  }

  int bplus_tree::search_range(key_t *left, const key_t &right, value_t *values,
                               size_t max, bool *next) const {
    if (left == nullptr || keycmp(*left, right) > 0) { // 虚空范围 以及 [7 , 5) 不符合
      return -1;
    }

    off_t off_left =  search_leaf(*left);
    off_t off_right = search_leaf(right);
    off_t off = off_left; // 游标
    size_t i = 0;
    record_t *b, *e;

    leaf_node_t leaf;
    while(off != off_right && off != 0 && i < max) {
      map(&leaf, off); // 读取叶子

      if (off_left == off)
        b = find(leaf, *left); // 从叶子的某部分开始
      else
        b = begin(leaf); // 从叶子的头部开始

      e = leaf.children + leaf.n;

      // 遍历叶子
      for (;b != e && i < max; ++b, ++i) {
        values[i] = b->value;
      }
      off = leaf.next; // 下一片叶子
    }

    if (i < max) {
      map(&leaf, off_right);

      b = find(leaf, *left);
      e = upper_bound(begin(leaf), end(leaf), right);
      for (;b != e && i < max; ++b, ++i) {
        values[i] = b->value;
      }
    }

    // mark for next iteration
    if (next != nullptr) {
      if (i == max && b != e) {
        *next = true;
        *left = b->key;
      } else {
        *next = false;
      }
    }
    return i;
  }

  int bplus_tree::remove(const key_t& key) {
    internal_node_t parent;
    leaf_node_t leaf;

    // find parent node
    off_t parent_off = search_index(key);
    map(&parent, parent_off);

    // find current node
    index_t *where = find(parent, key);
    off_t offset = where->child;
    map(&leaf, offset);

    // verify
    if (!binary_search(begin(leaf), end(leaf), key)) {
      return -1;
    }

    size_t min_n = meta.leaf_node_num = 1 ? 0 : meta.order / 2;
    assert(leaf.n >= min_n && leaf.n <= meta.order);

    // delete the key
    record_t *to_delete = find(leaf, key);
    std::copy(to_delete+ 1, end(leaf), to_delete);
    // merge or borrow
    if (leaf.n < min_n) { // 低于填充率
      // first borrow from left
      bool borrowed = false;
      if (leaf.prev != 0) {
        borrowed = borrow_key(false, leaf);
      }
      if (!borrowed && leaf.next != 0) {
        borrowed = borrow_key(true, leaf);
      }

      if (!borrowed) {
        assert(leaf.next != 0 || leaf.prev != 0);

        key_t index_key;

        // merge
        if (where == end(parent) - 1) {
          // | prev | leaf
          assert(leaf.prev != 0);
          leaf_node_t prev;
          map(&prev, leaf.prev);
          index_key = begin(prev)->key;

          merge_leafs(&prev, &leaf);
          node_remove(&prev, &leaf);
          unmap(&leaf, offset);
        } else {
          // | leaf | next
          assert(leaf.next != 0);
          leaf_node_t next;
          map(&next, leaf.prev);
          index_key = begin(leaf)->key;

          merge_leafs(&leaf, &next);
          node_remove(&leaf, &next);
          unmap(&leaf, offset);
        }

        // remove parent's key
        remove_from_index(parent_off, parent, index_key);
      } else {
        unmap(&leaf, offset);
      }
    } else {
      unmap(&leaf, offset);
    }

    return 0;
  }

  int bplus_tree::insert(const key_t& key, value_t value) {
    off_t parent = search_index(key);
    off_t offset = search_leaf(parent, key);
    leaf_node_t leaf;
    map(&leaf, offset);

    // check if we have the same key
    if (binary_search(begin(leaf), end(leaf), key)) {
      return 1;
    }

    if (leaf.n == meta.order) {// 满了 需要分裂
      leaf_node_t new_leaf;
      node_create(offset, &leaf, &new_leaf);

      // find even split point
      size_t point = leaf.n / 2;
      bool place_right = keycmp(key, leaf.children[point].key) > 0;
      if (place_right) {
        ++point;
      }

      // split
      std::copy(leaf.children + point, leaf.children + leaf.n, new_leaf.children);
      new_leaf.n = leaf.n - point;
      leaf.n = point;

      // which part do we put the key
      if (place_right)
        insert_record_no_split(&new_leaf, key, value);
      else
        insert_record_no_split(&leaf, key, value);

      // save leave
      unmap(&leaf, offset);
      unmap(&new_leaf, leaf.next);

      // insert new index key
      insert_key_to_index(parent, new_leaf.children[0].key, offset, leaf.next);
    } else { // 直接插入
      insert_record_no_split(&leaf, key, value);
      unmap(&leaf, offset);
    }

    return 0;
  }

  int bplus_tree::update(const key_t &key, value_t value) {

  }

  void bplus_tree::remove_from_index(off_t offset, internal_node_t &node,
                                     const key_t &key) {}

  bool bplus_tree::borrow_key(bool from_right, internal_node_t &borrower,
                              off_t offset) {}

  bool bplus_tree::borrow_key(bool from_right, leaf_node_t &borrower) {}

  void bplus_tree::change_parent_child(off_t parent, const key_t &o,
                                       const key_t &n) {}

  void bplus_tree::merge_leafs(leaf_node_t *left, leaf_node_t *right) {}

  void bplus_tree::merge_keys(index_t *where, internal_node_t &node,
                              internal_node_t &next) {}

  void bplus_tree::insert_record_no_split(leaf_node_t *leaf, const key_t &key,
                                          const value_t &value) {
    record_t *where = upper_bound(begin(*leaf), end(*leaf), key);
    std::copy_backward(where, end(*leaf), end(*leaf)+1);

    where->key = key;
    where->value = value;
    leaf->n++;
  }

  void bplus_tree::insert_key_to_index(off_t offset, const key_t &key,
                                       off_t old, off_t after) {}

  void bplus_tree::insert_key_to_index_no_split(internal_node_t &node,
                                                const key_t &key, off_t value) {

  }

  void bplus_tree::reset_index_children_parent(index_t *begin, index_t *end,
                                               off_t parent) {}

  // 从头找到key的索引
  off_t bplus_tree::search_index(const key_t &key) const {
    off_t org = meta.root_offset;
    int height = meta.height;
    while(height > 1) {
      internal_node_t node;
      map(&node, org); // node 是一个从root_offset开始的迭代器
      index_t *i = upper_bound(begin(node), end(node)-1, key); 
      org = i->child;
      --height;
    }
    return org;
  }

  // 查找当前 index 下的node中的leaf
  off_t bplus_tree::search_leaf(off_t index, const key_t &key) const {
    internal_node_t node;
    map(&node, index);

    index_t *i = upper_bound(begin(node), end(node)-1, key);
    return i->child;
  }

  // from offset create a new node "next" for the "node"
  template <class T> void bplus_tree::node_create(off_t offset, T *node, T *next) {
    // new sibling node
    next->parent = node->parent; // the same parent
    next->next = node->next; // "next" will carry the next_offset
    next->prev = offset;
    node->next = alloc(next);
    // update next node's prev
    if (next->next != 0) {
      T old_next;
      map(&old_next, next->next, SIZE_NO_CHILDREN);
      old_next.prev = node->next;
      unmap(&old_next, next->next, SIZE_NO_CHILDREN);
    }
    unmap(&meta, OFFSET_META);
  }

  template <class T> void bplus_tree::node_remove(T *prev, T *node) {
    unmap(&meta, OFFSET_META);
  }

  void bplus_tree::init_from_empty() {
    // init defalut meta
    bzero(&meta, sizeof(meta_t));
    meta.order = BP_ORDER;
    meta.key_size = sizeof(key_t);
    meta.value_size = sizeof(value_t);
    meta.height = 1;
    meta.slot = OFFSET_BLOCK;

    // init root node
    internal_node_t root;
    root.next = root.prev = root.parent = 0;
    meta.root_offset = alloc(&root);

    // init empey leaf
    leaf_node_t leaf;
    leaf.next = leaf.prev = 0;
    leaf.parent = meta.root_offset;
    meta.leaf_offset = root.children[0].child = alloc(&leaf);

    // save
    unmap(&meta, OFFSET_META);
    unmap(&root, meta.root_offset);
    unmap(&leaf, meta.leaf_offset);
    //unmap(&leaf, root.children[0].child);
  }


  }; // namespace bpt
