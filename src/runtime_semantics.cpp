#include <iostream>
#include <fstream>
#include <string>

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

// Store total amount of temp vars
static unsigned int total_temp_vars;

// Store counters for temp labels
static unsigned int total_temp_labels;

// Store stack of temp variables used
std::string temp_stack[MAX_SIZE];

enum temp_type {
  LABEL,
  VARIABLE
};

// Store global file for output
std::ofstream out_fp;

std::string generate_temp(int type) {
  std::string base;

  if (type == LABEL) {
    base += "L" + std::to_string(total_temp_labels);

    total_temp_labels++;
  }
  else if (type == VARIABLE) {
    base += "T" + std::to_string(total_temp_vars);
    temp_stack[total_temp_vars] = base;

    total_temp_vars++;
  }

  return base;
}

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

// Assist in formatting assembly file
void write_asm(std::string statement, std::string misc_param="") {
  out_fp << statement;

  // If not empty add extra spacing
  if (misc_param != "") {
    out_fp << " " << misc_param;
  }

  // Terminate each line with newline
  out_fp << "\n";
}

// Assist in writing all global variables to assembly file
void write_global_vars() {
  for (unsigned int i = 0; i < MAX_SIZE; i++) {
    if (temp_stack[i] == "") { continue; }
    else {
      // "Initialize" variables to 0
      write_asm(temp_stack[i], "0");
    }
  }
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

  // Output push instances to file
  write_asm("PUSH");

  // Increment count to resemble total variables in stack
  total_vars++;
}

// Remove a token from the stack
void pop() {
  // Loop to remove current scope tokens
  for (unsigned int current_scope = total_vars; current_scope > base_scope; current_scope--) {
    // One less variable is in the stack
    total_vars--;

    // Output pop instances to file
    write_asm("POP");

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

// Initialize base variables for assembly output
void initialize_semantics(Node * root, std::string filename) {
  // Set the file pointer up
  // File has been verified externally prior to call
  out_fp.open(filename);

  // Begin recursive chain
  process_semantics(root);
}

// Handle main recursive check
// var_count is defaulted to 0 in header
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

    // At the end of the traversal, print STOP to target
    write_asm("STOP");

    // Follow with global variables
    write_global_vars();
  }
  // <vars> -> empty | declare Identifier = Integer ; <vars>
  else if (label == "<vars>") {
    // Identifier
    int position = find(root->consumed_tokens[1]);

    base_scope = total_vars;

    // If not found then it is valid
    if (position == -1 || position > var_count) {
      std::string integer_value = root->consumed_tokens[3].token_instance;

      push(root->consumed_tokens[1]);

      // Fetch and add variable to TOS
      write_asm("LOAD", integer_value);
      write_asm("STACKW", "0");

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
      iterate_children(root->children, var_count);
    }
    // <N> + <expr>
    else {
      // <expr>
      process_semantics(root->children[1], var_count);

      // Get a temp var for storage
      std::string temp_var = generate_temp(VARIABLE);
      write_asm("STORE", temp_var);

      // <N>
      process_semantics(root->children[0], var_count);
      write_asm("ADD", temp_var);
    }
  }
  // <N> -> <A> / <N> | <A> * <N> | <A>
  else if (label == "<N>") {
    // <A>
    if (root->consumed_tokens.empty()) {
      iterate_children(root->children, var_count);
    }
    // <A> ? <N>
    else {
      process_semantics(root->children[1], var_count);

      // Get a temp var for storage
      std::string temp_var = generate_temp(VARIABLE);
      write_asm("STORE", temp_var);

      process_semantics(root->children[0], var_count);

      // Branch for symbols
      Token_Type temp_tk = root->consumed_tokens[0].token_ID;

      // /
      if (temp_tk == TK_SLASH) {
        write_asm("DIV", temp_var);
      }
      // *
      else if (temp_tk == TK_STAR) {
        write_asm("MULT", temp_var);
      }
    }
  }
  // <A> -> <M> - <A> | <M>
  else if (label == "<A>") {
    // <M>
    if (root->consumed_tokens.empty()) {
      iterate_children(root->children, var_count);
    }
    // <M> - <A>
    else {
      process_semantics(root->children[1], var_count);

      // Get a temp var for storage
      std::string temp_var = generate_temp(VARIABLE);
      write_asm("STORE", temp_var);

      process_semantics(root->children[0], var_count);
      write_asm("SUB", temp_var);
    }
  }
  // <M> -> . <M> | <R>
  else if (label == "<M>") {
    // <R>
    if (root->consumed_tokens.empty()) {
      iterate_children(root->children, var_count);
    }
    // . <M>
    else {
      iterate_children(root->children, var_count);

      Token_Type temp_tk = root->consumed_tokens[0].token_ID;

      // . to negate
      if (temp_tk == TK_PERIOD) {
        write_asm("MULT", "-1");
      }
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

        // If not found
        if (position == -1) {
          std::cout << "Semantic Error: Usage of undeclared variable."
            << "\n\t Instance: " << temp_tk.token_instance
            << "\n\t Line: " << temp_tk.line_num
            << std::endl;

          exit(EXIT_FAILURE);
        }

        // Otherwise read the value at position
        write_asm("STACKR", std::to_string(position));
      }
      // Integer
      else if (temp_tk_id == TK_INT) {
        write_asm("LOAD", temp_tk.token_instance);
      }
    }
  }
  // <stat> -> <in> ; | <out> ; | <block> | <if> ; | <loop> ; | <assign> ; | <goto> ; | <label> ;
  // TODO: Probably remove since no direct writing here
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

    // Get a temp var for storage
    std::string temp_var = generate_temp(VARIABLE);

    // listen reads input and stores in identifier
    write_asm("READ", temp_var);
    write_asm("LOAD", temp_var);
    write_asm("STACKW", std::to_string(position));
  }
  // <out> -> talk <expr>
  else if (label == "<out>") {
    iterate_children(root->children, var_count);

    // Get a temp var for storage
    std::string temp_var = generate_temp(VARIABLE);

    // talk outputs the given calculated expression
    write_asm("STORE", temp_var);
    write_asm("WRITE", temp_var);
  }
  // <if> -> if [ <expr> <RO> <expr> ] then <stat>
  //          | if [ <expr> <RO> <expr> ] then <stat> else <stat>
  else if (label == "<if>") {
    iterate_children(root->children, var_count);

    // TODO: Figure out how to deal with optionals
  }
  // <loop> -> while [ <expr> <RO> <expr> ] <stat>
  else if (label == "<loop>") {
    // [<expr>, <RO>, <expr>]
    Token temp_tk = root->children[1]->consumed_tokens[0];
    Token_Type temp_tk_id = temp_tk.token_ID;

    // Get a temp var for storage
    std::string temp_var = generate_temp(VARIABLE);

    // Get temp labels
    std::string temp_begin = generate_temp(LABEL);
    std::string temp_end = generate_temp(LABEL);

    // Declare start of loop
    write_asm(temp_begin, "NOOP");

    // Evaluate <expr> and store value
    process_semantics(root->children[2], var_count);
    write_asm("STORE", temp_var);

    // Evaluate other <expr>
    process_semantics(root->children[0], var_count);

    /*
    > is greater than
    < is less than
    { == }  is NOT equal
    == is equal
    % returns true if the signs of the arguments are the same (with a 0 counting as both, so a -1 and a 0 would evaluate as true, so would a 1 and a 0)

    */

    // TODO: DEBUG CONDITIONALS BECAUSE LOGIC WEIRD
    // Evaluate based on relational operator
    if (temp_tk_id == TK_GREATER_THAN) {
      write_asm("SUB", temp_var);
      /* write_asm("BR") */
    }
  }
  // <assign> -> assign Identifier = <expr>
  else if (label == "<assign>") {
    Token temp_tk = root->consumed_tokens[1];

    // <expr>
    iterate_children(root->children, var_count);

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
    // If found then write value
    else {
      write_asm("STACKW", std::to_string(position));
    }
  }
  // <label> -> label Identifier
  else if (label == "<label>") {
    Token temp_tk = root->consumed_tokens[1];
    std::string label = root->consumed_tokens[1].token_instance;

    // Identifier
    int position = find(temp_tk);

    // If not found then it is valid
    if (position == -1 || position > var_count) {
      push(root->consumed_tokens[1]);

      // No children left over at this point
      // Initialize labels to NOOP
      write_asm(label + ":", "NOOP");

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
    // Otherwise allow the jump to occur
    else {
      write_asm("BR", temp_tk.token_instance);
    }
  }
  // Most things should be able to just keep recursively iterating their children
  // Not containing vars specifically
  else {
    iterate_children(root->children, var_count);
  }
}
