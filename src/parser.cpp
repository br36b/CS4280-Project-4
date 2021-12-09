#include <iostream>

#include <map>
#include <string>

#include "parser.h"
#include "scanner.h"

Token temp_tk;
std::ifstream *in_fp = nullptr;

// Might as well make this unsigned
unsigned int current_line = 1;

// Store the string version of Token_Types
// For printing purposes
// Keep it here to avoid undefined behavior
std::map<Token_Type, std::string> token_strings = {
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
  { TK_LESS_THAN      , "Operator: Less Than" },     // <
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

// Fetch the next token from the scanner using the global variables
void get_next_token(Node *n) {
  // Store the consumed tokens before getting new one
  if (n != nullptr) {
    n->consumed_tokens.push_back(temp_tk);
  }

  // Fetch new token from scanner using globals
  temp_tk = scanner(*in_fp, current_line);
}

// Auxiliary for parser
// Just the old test scanner with small changes
// Will not reach this function if it starts off with no data
Node *parser(std::ifstream &in_stream) {
  std::cout << "\nParsing..." << std::endl;

  // Assign global file pointer from parameter
  in_fp = &in_stream;
  bool has_data = (in_fp->peek() != EOF);

  // Create main root
  Node * root = nullptr;

  // Keep getting tokens until EOF is hit
  // Parser will end up hitting it anyways before it moves on
  while (has_data) {
    get_next_token(root);

    // Assign root of BNF
    root = program();

    // Exit once no more tokens are in file
    if (temp_tk.token_ID == TK_EOF) {
      std::cout << "Parser: Successfully reached the end of file!" << std::endl;
    }
    // Needs to end with EOF since it is the base token
    else {
      error(TK_EOF, temp_tk.token_ID);
    }

    // Update to make sure to exit once EOF is hit
    has_data = (in_fp->peek() != EOF);
  }

  return root;
}

// Display parser errors and exit program
void error(Token_Type valid_tk, Token_Type invalid_tk) {
  std::cout << "\nParser Error"
    << "\n\tLine: " << current_line
    << "\n\tExpected Token: " << token_strings[valid_tk]
    << "\n\tReceived Token: " << token_strings[invalid_tk]
    << "\n\t" << token_strings[temp_tk.token_ID]
    << " Instance: " << temp_tk.token_instance
    << std::endl;

  exit(EXIT_FAILURE);
}

// Words from document
// Make sure first sets of <stat> are legal
bool is_statement_keyword() {
  // Compare to global token
  switch (temp_tk.token_ID) {
    case TK_LISTEN:   // <in>
    case TK_TALK:     // <out>
    case TK_START:    // <block>
    case TK_IF:       // <if>
    case TK_WHILE:    // <loop>
    case TK_ASSIGN:   // <assign>
    case TK_JUMP:     // <goto>
    case TK_LABEL:    // <label>
      return true;
    // Otherwise they are one of the unused keywords
    default:
      return false;
  }
}

// Takes two nodes
// Adds it's it as a child
void add_child(Node *base, Node *res) {
  base->children.push_back(res);
}

// <program> -> <vars> program <block>
Node *program() {
  // Base level for program
  unsigned int depth = 0;

  // Create sub-root
  Node *temp = new Node("<program>", depth);

  // <vars>
  add_child(temp, vars(depth));

  // <program>
  if (temp_tk.token_ID == TK_PROGRAM) {
    get_next_token(temp);

    // <block>
    add_child(temp, block(depth));

    return temp;
  }

  // Expects program token, error out otherwise
  error(TK_PROGRAM, temp_tk.token_ID);

  // Auto exits, just for warnings
  return nullptr;
}

// <block> -> start <vars> <stats> stop
Node *block(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<block>", depth);

  // Follow up with valid tokens
  if (temp_tk.token_ID == TK_START) {
    get_next_token(temp);

    // <vars>
    add_child(temp, vars(depth));

    // <stats>
    add_child(temp, stats(depth));

    // Stop is final token of block
    if (temp_tk.token_ID == TK_STOP) {
      get_next_token(temp);

      return temp;
    }
    // Expected stop
    else {
      error(TK_STOP, temp_tk.token_ID);
    }
  }

  // Expected start
  error(TK_START, temp_tk.token_ID);

  // Auto exits, just for warnings
  return nullptr;
}

// <vars> -> empty | declare Identifier = Integer ; <vars>
Node *vars(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<vars>", depth);

  // declare
  if (temp_tk.token_ID == TK_DECLARE) {
    get_next_token(temp);

    // Identifier
    if (temp_tk.token_ID == TK_ID) {
      get_next_token(temp);

      // =
      if (temp_tk.token_ID == TK_EQUALS) {
        get_next_token(temp);

        // Integer
        if (temp_tk.token_ID == TK_INT) {
          get_next_token(temp);

          // ;
          if (temp_tk.token_ID == TK_SEMICOLON) {
            get_next_token(temp);

            // <vars>
            // Recursive at this point
            add_child(temp, vars(depth));

            return temp;
          }
          // Expected ;
          else {
            error(TK_SEMICOLON, temp_tk.token_ID);
          }
        }
        // Expected Integer
        else {
          error(TK_INT, temp_tk.token_ID);
        }
      }
      // Expected =
      else {
        error(TK_EQUALS, temp_tk.token_ID);
      }
    }
    // Expected Identifier
    else {
      error(TK_ID, temp_tk.token_ID);
    }
  }
  // If not declare then there is no right side expansion
  // Empty set is null
  // return empty node once tree is up
  return nullptr;
}

// <expr> -> <N> + <expr> | <N>
Node *expr(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<expr>", depth);

  // <N> in both cases
  add_child(temp, N(depth));

  // +
  if (temp_tk.token_ID == TK_PLUS) {
    get_next_token(temp);

    // <expr>
    add_child(temp, expr(depth));

    return temp;
  }

  // Situation where <N> was alone
  return temp;
}

// <N> -> <A> / <N> | <A> * <N> | <A>
Node *N(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<N>", depth);

  // <A> in all 3 cases
  add_child(temp, A(depth));

  // /
  if (temp_tk.token_ID == TK_SLASH) {
    get_next_token(temp);

    // <N>
    add_child(temp, N(depth));

    return temp;
  }
  // *
  else if (temp_tk.token_ID == TK_STAR) {
    get_next_token(temp);

    // <N>
    add_child(temp, N(depth));

    return temp;
  }

  // Otherwise it's just <A> alone
  return temp;
}

// <A> -> <M> - <A> | <M>
Node *A(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<A>", depth);

  // <M> in both cases
  add_child(temp, M(depth));

  // -
  if (temp_tk.token_ID == TK_MINUS) {
    get_next_token(temp);

    // <A>
    add_child(temp, A(depth));

    return temp;
  }

  // Otherwise it's just <M> alone
  return temp;
}

// <M> -> . <M> | <R>
Node *M(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<M>", depth);

  // .
  if (temp_tk.token_ID == TK_PERIOD) {
    get_next_token(temp);

    // <M>
    add_child(temp, M(depth));

    return temp;
  }

  // Otherwise it was an <R>
  add_child(temp, R(depth));

  return temp;
}

// <R> -> ( <expr> ) | Identifier | Integer
Node *R(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<R>", depth);

  // (
  if (temp_tk.token_ID == TK_L_PAREN) {
    get_next_token(temp);

    // <expr>
    add_child(temp, expr(depth));

    // )
    if (temp_tk.token_ID == TK_R_PAREN) {
      get_next_token(temp);

      return temp;
    }
    // Expected )
    else {
      error(TK_R_PAREN, temp_tk.token_ID);
    }
  }
  // Identifier
  else if (temp_tk.token_ID == TK_ID) {
    get_next_token(temp);

    return temp;
  }
  // Integer
  else if (temp_tk.token_ID == TK_INT) {
    get_next_token(temp);

    return temp;
  }

  // Expected (
  error(TK_L_PAREN, temp_tk.token_ID);

  // Auto exits, just for warnings
  return nullptr;
}

// <stats> -> <stat> <m_stat>
// Only one evaluation
Node *stats(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<stats>", depth);

  // <stat>
  add_child(temp, stat(depth));

  // <m_stat>
  add_child(temp, m_stat(depth));

  return temp;
}

// <m_stat> -> empty | <stat> <m_stat>
Node *m_stat(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<m_stat>", depth);

  // Have to check if it's a keyword match or just empty
  bool is_valid = is_statement_keyword();

  // Then it is a <stat>
  if (is_valid) {
    // <stat>
    add_child(temp, stat(depth));

    // <m_stat>
    add_child(temp, m_stat(depth));

    return temp;
  }

  // Otherwise it was an empty set, which is still valid
  return nullptr;
}

// <stat> -> <in> ; | <out> ; | <block> | <if> ; | <loop> ; | <assign> ; | <goto> ; | <label> ;
Node *stat(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<stat>", depth);

  // Check first sets of word above

  // <in> -> listen
  if (temp_tk.token_ID == TK_LISTEN) {
    // <in>
    add_child(temp, in(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <out> -> talk
  else if (temp_tk.token_ID == TK_TALK) {
    // <out>
    add_child(temp, out(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <block> -> start
  else if (temp_tk.token_ID == TK_START) {
    // <block>
    add_child(temp, block(depth));

    return temp;
  }
  // <if> -> if
  else if (temp_tk.token_ID == TK_IF) {
    // if
    add_child(temp, if_statement(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <loop> -> while
  else if (temp_tk.token_ID == TK_WHILE) {
    // <loop>
    add_child(temp, loop(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <assign> -> assign
  else if (temp_tk.token_ID == TK_ASSIGN) {
    // <assign>
    add_child(temp, assign(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <goto> -> jump
  else if (temp_tk.token_ID == TK_JUMP) {
    // <goto>
    add_child(temp, goto_statement(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }
  // <label> -> label
  else if (temp_tk.token_ID == TK_LABEL) {
    // <label>
    add_child(temp, label(depth));

    // ;
    if (temp_tk.token_ID == TK_SEMICOLON) {
      // Valid ending
      get_next_token(temp);

      return temp;
    }
    // Expected ;
    else {
      error(TK_SEMICOLON, temp_tk.token_ID);
    }
  }

  // If it hits none of the statements than it is an invalid statement first-set
  // Just use error function here once outside of code since too unique words
  std::cout << "\nParser Error"
    << "\n\tLine: " << current_line
    << "\n\tExpected Token: Statement Sub-Tokens: "
    << "\n\t\t" << token_strings[TK_LISTEN]   // <in>
    << "\n\t\t" << token_strings[TK_TALK]     // <out>
    << "\n\t\t" << token_strings[TK_START]    // <block>
    << "\n\t\t" << token_strings[TK_IF]       // <if>
    << "\n\t\t" << token_strings[TK_WHILE]    // <loop>
    << "\n\t\t" << token_strings[TK_ASSIGN]   // <assign>
    << "\n\t\t" << token_strings[TK_JUMP]     // <goto>
    << "\n\t\t" << token_strings[TK_LABEL]    // <label>
    << "\n\tReceived Token: " << token_strings[temp_tk.token_ID]
    << std::endl;

  exit(EXIT_FAILURE);
}

// <in> -> listen Identifier
Node *in(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<in>", depth);

  // listen
  if (temp_tk.token_ID == TK_LISTEN) {
    get_next_token(temp);

    // Identifier
    if (temp_tk.token_ID == TK_ID) {
      get_next_token(temp);
      return temp;
    }
    // Expected Identifier
    else {
      error(TK_ID, temp_tk.token_ID);
    }
  }

  // Expected listen
  error(TK_LISTEN, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <out> -> talk <expr>
Node *out(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<out>", depth);

  // talk
  if (temp_tk.token_ID == TK_TALK) {
    get_next_token(temp);

    // <expr>
    add_child(temp, expr(depth));

    return temp;
  }
  // Expected talk
  error(TK_TALK, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <if> -> if [ <expr> <RO> <expr> ] then <stat>
//          | if [ <expr> <RO> <expr> ] then <stat> else <stat>
Node *if_statement(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<if>", depth);

  // if
  if (temp_tk.token_ID == TK_IF) {
    get_next_token(temp);

    // [
    if (temp_tk.token_ID == TK_L_BRACKET) {
      get_next_token(temp);

      // <expr>
      add_child(temp, expr(depth));

      // <RO>
      add_child(temp, RO(depth));

      // <expr>
      add_child(temp, expr(depth));

      // ]
      if (temp_tk.token_ID == TK_R_BRACKET) {
        get_next_token(temp);

        // then
        if (temp_tk.token_ID == TK_THEN) {
          get_next_token(temp);

          // <stat>
          add_child(temp, stat(depth));

          // Optional else
          if (temp_tk.token_ID == TK_ELSE) {
            get_next_token(temp);

            // <stat>
            add_child(temp, stat(depth));

            return temp;
          }

          // Otherwise his is valid as is
          return temp;
        }
        // Expected then
        else {
          error(TK_THEN, temp_tk.token_ID);
        }
      }
      // Expected ]
      else {
        error(TK_R_BRACKET, temp_tk.token_ID);
      }
    }
    // Expected [
    else {
      error(TK_L_BRACKET, temp_tk.token_ID);
    }
  }
  // Expected if
  error(TK_IF, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <loop> -> while [ <expr> <RO> <expr> ] <stat>
Node *loop(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<loop>", depth);

  // while
  if (temp_tk.token_ID == TK_WHILE) {
    get_next_token(temp);

    // [
    if (temp_tk.token_ID == TK_L_BRACKET) {
      get_next_token(temp);

      // <expr>
      add_child(temp, expr(depth));

      // <RO>
      add_child(temp, RO(depth));

      // <expr>
      add_child(temp, expr(depth));

      // ]
      if (temp_tk.token_ID == TK_R_BRACKET) {
        get_next_token(temp);

        add_child(temp, stat(depth));

        return temp;
      }
      // Expected ]
      else {
        error(TK_R_BRACKET, temp_tk.token_ID);
      }
    }
    // Expected [
    else {
      error(TK_L_BRACKET, temp_tk.token_ID);
    }
  }
  // Expected while
  error(TK_WHILE, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <assign> -> assign Identifier = <expr>
Node *assign(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<assign>", depth);

  // <assign>
  if (temp_tk.token_ID == TK_ASSIGN) {
    get_next_token(temp);

    // Identifier
    if (temp_tk.token_ID == TK_ID) {
      get_next_token(temp);

      // =
      if (temp_tk.token_ID == TK_EQUALS) {
        get_next_token(temp);

        // <expr>
        add_child(temp, expr(depth));

        return temp;
      }
      // Expected =
      else {
        error(TK_EQUALS, temp_tk.token_ID);
      }
    }
    // Expected Identifier
    else {
      error(TK_ID, temp_tk.token_ID);
    }
  }
  // Expected assign
  error(TK_ASSIGN, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <RO> -> > | < | == | { == } (three tokens) | %
Node *RO(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<RO>", depth);

  // >
  if (temp_tk.token_ID == TK_GREATER_THAN) {
    get_next_token(temp);

    return temp;
  }
  // <
  else if (temp_tk.token_ID == TK_LESS_THAN) {
    get_next_token(temp);

    return temp;
  }
  // ==
  else if (temp_tk.token_ID == TK_EQUALS_EQUALS) {
    get_next_token(temp);

    return temp;
  }
  // {
  else if (temp_tk.token_ID == TK_L_BRACE) {
    get_next_token(temp);

    // ==
    if (temp_tk.token_ID == TK_EQUALS_EQUALS) {
      get_next_token(temp);

      // }
      if (temp_tk.token_ID == TK_R_BRACE) {
        get_next_token(temp);

        return temp;
      }
      // Expected }
      else {
        error(TK_R_BRACE, temp_tk.token_ID);
      }
    }
    // Expected ==
    else {
      error(TK_EQUALS_EQUALS, temp_tk.token_ID);
    }
  }
  // %
  else if (temp_tk.token_ID == TK_PERCENT) {
    get_next_token(temp);

    return temp;
  }

  // If it hits none of the statements than it is an invalid character
  // Just use error function here once outside of code since too many operators
  std::cout << "\nParser Error"
    << "\n\tLine: " << current_line
    << "\n\tExpected Token: Relational Operator (> | < | == | { == } | %)"
    << "\n\tReceived Token: " << token_strings[temp_tk.token_ID]
    << std::endl;

  exit(EXIT_FAILURE);
}

// <label> -> label Identifier
Node *label(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<label>", depth);

  // label
  if (temp_tk.token_ID == TK_LABEL) {
    get_next_token(temp);

    // Identifier
    if (temp_tk.token_ID == TK_ID) {
      get_next_token(temp);

      return temp;
    }
    // Expected Identifier
    else {
      error(TK_ID, temp_tk.token_ID);
    }
  }
  // Expected label
  error(TK_LABEL, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}

// <goto> -> jump Identifier
Node *goto_statement(int depth) {
  // Increment depth for function chain calls
  depth++;

  // Create sub-root
  Node *temp = new Node("<goto>", depth);

  // jump
  if (temp_tk.token_ID == TK_JUMP) {
    get_next_token(temp);

    // Identifier
    if (temp_tk.token_ID == TK_ID) {
      get_next_token(temp);

      return temp;
    }
    // Expected Identifier
    else {
      error(TK_ID, temp_tk.token_ID);
    }
  }
  // Expected jump
  error(TK_JUMP, temp_tk.token_ID);

  // Automatically exits before this
  return nullptr;
}
