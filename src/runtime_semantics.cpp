#include <iostream>

#include "runtime_semantics.h"

// Assume no more than 100 items in a program
const int MAX_SIZE = 100;

// Store stack of file
static Token tk_stack[MAX_SIZE];

// Store total variables stored
static unsigned int total_vars;

// Store the base scope of execution
static unsigned int base_scope;

// Store the local scope when blocks are used
static unsigned int block_indent_scope;

// Helper function to recursively call children
void iterate_children(std::vector<Node *> children, unsigned int var_count) {

  for (auto child: children) {
    if (child != nullptr) {
      process_semantics(child, var_count);
    }
  }
}

// Find if a variable was declared before usage
int check_vars(std::string instance) {
  // Offset by one for arrays
  int position = total_vars - 1;

  // Iterate through each item currently in the stack
  // Check all the way to 0th index
  while (position > -1) {
    if (tk_stack[position].token_instance == instance) {
      return (total_vars - 1) - position;
    }
    position--;
  }

  // Default to negative value if no
  return -1;
}

// Helper functions to work with stack items
void push(Token tk) {
  // Make sure that there is still room in the stack
  if (total_vars >= MAX_SIZE) {
    std::cout << "\nSemantic Error: Max number of stack items exceeded. Limit 100. Total Items: "
      << total_vars << std::endl;

    exit(EXIT_FAILURE);
  }

  // Make sure no duplicate vars are declared in the same scope
  for (unsigned int current_scope = base_scope; current_scope < total_vars; current_scope++) {

    if (tk_stack[current_scope].token_instance == tk.token_instance) {
      std::cout << "Semantic Error: There was a variable already declared in this scope. Variable: "
        << tk.token_instance << " on line " << tk.line_num << std::endl;

      exit(EXIT_FAILURE);
    }
  }

  // Push the variable to the global index in stack
  tk_stack[total_vars] = tk;

  // Increment count to resemble total variables in stack
  total_vars++;
}

// Remove a token from the stack
void pop() {
  // Loop to remove current scope tokens
  for (unsigned int current_scope = total_vars; current_scope > base_scope; current_scope--) {
    // One less variable is in the stack
    total_vars--;

    // Reset the value within
    tk_stack[current_scope].token_instance = "";
    tk_stack[current_scope].line_num = 0;
  }
}

// Find the index of a token
int find(Token tk) {
  // Loop to find token
  for (unsigned int current_scope = total_vars; current_scope >= base_scope; current_scope--) {
    // Calculate the distance from the top of the stack
    // Offset by one since arrays start at 0
    return (total_vars - 1) - current_scope;
  }

  // Negative if no match was found
  return -1;
}

// Function to help keep track of the current stack at given times
void print_vars() {
  for (unsigned int index = 0; index < MAX_SIZE; index++) {
    if (tk_stack[index].token_instance == "") {
      std::cout << std::endl;
      break;
    }

    // Print out what is inside of the token
    std::cout << tk_stack[index].token_instance << " ";
  }
}

// Handle main recursive check
void process_semantics(Node * root, int var_count) {
  // Make sure there is something inside of the root node
  // All possible children get checked when recursively calling
  if (root == nullptr) { return; }

  std::string label = root->func_label;

  std::cout << "Next Process Point: " << label << std::endl;

  print_vars();

  // <program> -> <vars> program <block>
  if (label == "<program>") {
    unsigned int local_var_count = 0;

    // Evaluate slot for <vars> and <block>

    iterate_children(root->children, local_var_count);
  }
  // <vars> -> empty | declare Identifier = Integer ; <vars>
  else if (label == "<vars>") {

    // Identifier
    int position = find(root->consumed_tokens[1]);

    base_scope = total_vars;

    // If not found then it is valid
    if (position == -1 || position > var_count) {
      push(root->consumed_tokens[1]);

      var_count++;
    }
    // If found within the stack of currently stored
    else if (position < var_count) {
      std::cout << "Semantic Error: Variable declared more than once."
        << "\n\t Instance: " << root->consumed_tokens[1].token_instance
        << "\n\t Line: " << root->consumed_tokens[1].line_num
        << std::endl;

      exit(EXIT_FAILURE);
    }

    // If no other tokens then <vars>
    if (!root->children.empty()) {
      process_semantics(root->children[0], var_count);
    }

    if (block_indent_scope > 0) {
      print_vars();
    }
  }
  // <block> -> start <vars> <stats> stop
  else if (label == "<block>") {
    unsigned int local_var_count = 0;

    // Change scope for current block
    base_scope = total_vars;

    if (block_indent_scope == 0) {
      print_vars();
    }

    // Begin block scope
    block_indent_scope++;

    // <vars> and <stats>
    iterate_children(root->children, local_var_count);

    // Remove a scope level once finished with block
    pop();

    // End block scope
    block_indent_scope--;
  }
  // <expr> -> <N> + <expr> | <N>
  else if (label == "<expr>") {
    // <N>
    if (root->consumed_tokens.empty()) {
      process_semantics(root->children[0], var_count);
    }
    // <N> + <expr>
    else {
      process_semantics(root->children[0], var_count);
      process_semantics(root->children[1], var_count);
    }
  }
  // <N> -> <A> / <N> | <A> * <N> | <A>
  else if (label == "<N>") {
    // <A>
    if (root->consumed_tokens.empty()) {
      process_semantics(root->children[0], var_count);
    }
    // <A> ? <N>
    else {
      process_semantics(root->children[0], var_count);
      process_semantics(root->children[1], var_count);
    }
  }
  // <A> -> <M> - <A> | <M>
  else if (label == "<A>") {
    // <M>
    if (root->consumed_tokens.empty()) {
      process_semantics(root->children[0], var_count);
    }
    // <M> - <A>
    else {
      process_semantics(root->children[0], var_count);
      process_semantics(root->children[1], var_count);
    }
  }
  // <M> -> . <M> | <R>
  else if (label == "<M>") {
    // <R>
    if (!root->consumed_tokens.empty()) {
      process_semantics(root->children[0], var_count);
    }
    // . <M>
    else {
      process_semantics(root->children[0], var_count);
    }
  }
  // <R> -> ( <expr> ) | Identifier | Integer
  else if (label == "<R>") {
    // ( <expr> )
    if (!root->children.empty()) {
      iterate_children(root->children, var_count);
    }
    // Only check if not empty
    // Identifier | Integer
    else {
      Token temp_tk = root->consumed_tokens[0];
      Token_Type temp_tk_id = temp_tk.token_ID;

      // Identifier
      if (temp_tk_id == TK_ID) {
        int position = check_vars(temp_tk.token_instance);

        if (position == -1) {
          std::cout << "Semantic Error: Usage of undeclared variable."
            << "\n\t Instance: " << temp_tk.token_instance
            << "\n\t Line: " << temp_tk.line_num
            << std::endl;

          exit(EXIT_FAILURE);
        }
      }
      // Integer
      // Notes say to disregard for now
    }
  }
// <stat> -> <in> ; | <out> ; | <block> | <if> ; | <loop> ; | <assign> ; | <goto> ; | <label> ;
  else if (label == "<stat>") {
    std::string current_label = root->children[0]->func_label;

    // Iterate over the children of stat blocks
    std::vector<std::string> valid_blocks = {
      "<in>",
      "<out>",
      "<block>",
      "<if>",
      "<loop>",
      "<assign>",
      "<goto>",
      "<label>",
    };

    for (auto item: valid_blocks) {
      if (item == current_label) {
        process_semantics(root->children[0], var_count);
        break;
      }
    }
  }
  // <in> -> listen Identifier
  else if (label == "<in>") {
    Token temp_tk = root->consumed_tokens[1];

    // Identifier
    int position = check_vars(temp_tk.token_instance);

    // If no instance cannot be found
    if (position == -1) {
      std::cout << "Semantic Error: Usage of undeclared variable."
        << "\n\t Instance: " << temp_tk.token_instance
        << "\n\t Line: " << temp_tk.line_num
        << std::endl;

      exit(EXIT_FAILURE);
    }
  }
  // <out> -> talk <expr>
  else if (label == "<out>") {
    iterate_children(root->children, var_count);
  }
  // <if> -> if [ <expr> <RO> <expr> ] then <stat>
  //          | if [ <expr> <RO> <expr> ] then <stat> else <stat>
  else if (label == "<if>") {
    iterate_children(root->children, var_count);
  }
  // <loop> -> while [ <expr> <RO> <expr> ] <stat>
  else if (label == "<loop>") {
    iterate_children(root->children, var_count);
  }
  // <assign> -> assign Identifier = <expr>
  else if (label == "<assign>") {
    Token temp_tk = root->consumed_tokens[1];

    // Identifier
    int position = check_vars(temp_tk.token_instance);

    // If no instance cannot be found
    if (position == -1) {
      std::cout << "Semantic Error: Usage of undeclared variable."
        << "\n\t Instance: " << temp_tk.token_instance
        << "\n\t Line: " << temp_tk.line_num
        << std::endl;

      exit(EXIT_FAILURE);
    }
  }
  // <label> -> label Identifier
  else if (label == "<label>") {
    // Identifier
    int position = find(root->consumed_tokens[1]);

    // If not found then it is valid
    if (position == -1 || position > var_count) {
      push(root->consumed_tokens[1]);

      var_count++;
    }
    // If found within the stack of currently stored
    else if (position < var_count) {
      std::cout << "Semantic Error: Identifier declared more than once."
        << "\n\t Instance: " << root->consumed_tokens[1].token_instance
        << "\n\t Line: " << root->consumed_tokens[1].line_num
        << std::endl;

      exit(EXIT_FAILURE);
    }

    // Take care of anything left over
    iterate_children(root->children, var_count);
  }
  // <goto> -> jump Identifier
  else if (label == "<goto>") {
    // Only process if the goto is already defined
    Token temp_tk = root->consumed_tokens[1];

    // Identifier
    int position = check_vars(temp_tk.token_instance);

    // If no instance cannot be found
    if (position == -1) {
      std::cout << "Semantic Error: Usage of undeclared variable."
        << "\n\t Instance: " << temp_tk.token_instance
        << "\n\t Line: " << temp_tk.line_num
        << std::endl;

      exit(EXIT_FAILURE);
    }

    // Otherwise take care of any other children
    iterate_children(root->children, var_count);
  }
  // Most things should be able to just keep recursively iterating their children
  // Not containing vars specifically
  else {
    iterate_children(root->children, var_count);
  }
}
