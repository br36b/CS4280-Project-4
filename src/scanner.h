#ifndef SCANNER_H
#define SCANNER_H

#include <fstream>

#include "token.h"

int find_col(char);
bool remove_comments(std::ifstream &, unsigned int &, char &);
Token scanner(std::ifstream &, unsigned int &);

#endif
