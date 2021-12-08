#ifndef RUNTIME_SEMANTICS_H
#define RUNTIME_SEMANTICS_H

#include <vector>

#include "node.h"

void process_semantics(Node *, int=0);

void iterate_children(std::vector<Node *>, unsigned int);

// Suggested interfaces
// Swapped with tokens to preserve data
void push(Token);
void pop(void);
int find(Token);

void print_vars();
int check_vars(std::string);

void write_asm(std::string, std::string);
void initialize_semantics(Node *, std::string="");

#endif
