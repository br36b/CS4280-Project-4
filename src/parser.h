#ifndef PARSER_H
#define PARSER_H

#include <fstream>

#include "token.h"
#include "node.h"

// To assist in <stat> first sets
bool is_statement_keyword();

// Cycle tokens
void get_next_token();

// Add child to node
void add_child(Node *, Node *);

// Auxiliary Function
Node *parser(std::ifstream&);

// BNF Functions
Node *program();
Node *block(int);
Node *vars(int);
Node *expr(int);

Node *N(int);
Node *A(int);
Node *M(int);
Node *R(int);

Node *stats(int);
Node *m_stat(int);
Node *stat(int);

// Must rename some of these to avoid C++ keyword errors
Node *in(int);
Node *out(int);
Node *if_statement(int);
Node *loop(int);
Node *assign(int);
Node *goto_statement(int);
Node *label(int);

Node *RO(int);

// Error function
void error(Token_Type, Token_Type);

#endif
