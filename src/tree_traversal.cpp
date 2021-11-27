#include <iostream>
#include <cstdlib>
#include <fstream>
#include <set>

#include "tree_traversal.h"

// Store the string version of Token_Types
// For printing purposes
// Keep it here to avoid undefined behavior
std::map<Token_Type, std::string> tk_strings = {
  // End of file and error tokens
  { TK_EOF    , "End of File"},
  { TK_ERROR  , "Token Error" },

  // Identifier
  { TK_ID     , "Identifier" },

  // Integer
  { TK_INT    , "Integer" },

  // Keywords
  { TK_START  , "Keyword: Start" },     // start
  { TK_STOP   , "Keyword: Stop" },      // stop
  { TK_LOOP   , "Keyword: Loop" },      // loop
  { TK_WHILE  , "Keyword: While" },     // while
  { TK_FOR    , "Keyword: For" },       // for
  { TK_LABEL  , "Keyword: Label" },     // label
  { TK_EXIT   , "Keyword: Exit" },      // exit
  { TK_LISTEN , "Keyword: Listen" },    // listen
  { TK_TALK   , "Keyword: Talk" },      // talk
  { TK_PROGRAM, "Keyword: Program" },   // program
  { TK_IF     , "Keyword: If" },        // if
  { TK_THEN   , "Keyword: Then" },      // then
  { TK_ASSIGN , "Keyword: Assign" },    // assign
  { TK_DECLARE, "Keyword: Declare" },   // declare
  { TK_JUMP   , "Keyword: Jump" },      // jump
  { TK_ELSE   , "Keyword: Else" },      // else

  // Operators and delimiters
  // Note: Do not use assign for = since it's a keyword
  { TK_EQUALS         , "Operator: Equals" },        // =
  { TK_GREATER_THAN   , "Operator: Greater Than" },  // >
  { TK_LESS_THAM      , "Operator: Less Than" },     // <
  { TK_EQUALS_EQUALS  , "Operator: Equals Equals" }, // == give literal name for now
  { TK_COLON          , "Operator: Colon" },         // :
  { TK_COLON_EQUALS   , "Operator: Colon-Equals" },  // :=  Assignment op?
  { TK_PLUS           , "Operator: Plus" },          // +
  { TK_MINUS          , "Operator: Minus" },         // -
  { TK_STAR           , "Operator: Star" },          // *
  { TK_SLASH          , "Operator: Slash" },         // /
  { TK_PERCENT        , "Operator: Percent" },       // %

  { TK_PERIOD         , "Deliminator: Period" },                // .
  { TK_L_PAREN        , "Deliminator: Left Parenthesis"  },     // (
  { TK_R_PAREN        , "Deliminator: Right Parenthesis" },     // )
  { TK_COMMA          , "Deliminator: Comma" },                 // ,
  { TK_L_BRACE        , "Deliminator: Left Curly Brace" },      // {
  { TK_R_BRACE        , "Deliminator: Right Curly Brace" },     // }
  { TK_SEMICOLON      , "Deliminator: Semicolon" },             // ;
  { TK_L_BRACKET      , "Deliminator: Left Square Bracket" },   // [
  { TK_R_BRACKET      , "Deliminator: Right Square Bracket" },  // ]
};

// Print pre-order traversal of the given tree
void print_pre_order(Node *root) {
  if (root != nullptr) {
    // Indent by 2x depth of node
    std::string indent = std::string(root->depth * 2, ' ');

    // Traverse root, display first letter of node strings
    std::cout << indent << "D"<< root->depth << " - " << root->func_label << ": ";

    // Followed by list of token data strings from node
    print_tokens(root->consumed_tokens);

    // Recursively traverse the child nodes and their child nodes
    print_children(root->children);
  }
}

// Print all the nodes inside of vector and their children
void print_children(std::vector<Node *> words) {
  // Recursively print sub-nodes
  for (Node *node: words) {
    if (node != nullptr) {
      print_pre_order(node);
    }
  }
}

// Print out all tokens consumed
void print_tokens(std::vector<Token> tokens) {
  for (Token tk: tokens) {
    std::cout << " Token(L" << tk.line_num
      << " " << tk_strings[tk.token_ID]
      << ": '" << tk.token_instance << "'"
      << ") ";
  }

  // Move on to new line after all tokens have been printed
  std::cout << std::endl;
}
