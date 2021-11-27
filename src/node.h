#ifndef NODE_H
#define NODE_H

// Reuse file from P0
// Only here because they are used in the node vars
#include <vector>

// Tokens may be duplicate, use vector
#include <set>
#include <string>

#include "token.h"

// Modified struct for bst off https://www.geeksforgeeks.org/binary-tree-set-1-introduction/
struct Node {
  // Passed values
  Node(std::string label, unsigned int depth) {
    this->func_label = label;
    this->depth = depth;
  }

  // Store label of BNF function and depth
  std::string func_label;
  unsigned int depth;

  // Use vector to avoid having to resize arrays
  // Point to children
  // Since vector, no max size
  std::vector<Node *> children;

  // Store or dispose tokens consumed
  std::vector<Token> consumed_tokens;

// Use vector chains
/*   // Binary Tree */
/*   Node *left; */
/*   Node *right; */
/*   Node *next; */
};


#endif
