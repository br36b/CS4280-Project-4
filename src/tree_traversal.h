#ifndef TREE_TRAVERSAL_H
#define TREE_TRAVERSAL_H

#include "node.h"

// Functions for traversals
void print_pre_order(Node *);
void print_in_order(Node *);
void print_post_order(Node *);

// Print all node words
void print_children(std::vector<Node *>);
void print_tokens(std::vector<Token>);

#endif
