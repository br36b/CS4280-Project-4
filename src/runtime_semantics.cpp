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

// Store total amount of temp vars
static unsigned int total_temp_vars;

// Store counters for temp labels
static unsigned int total_temp_labels;

// Store stack of temp variables used
std::string temp_stack[MAX_SIZE];

const std::string LABEL_PREFIX = "L_";
const std::string VARIABLE_PREFIX = "T";

enum temp_type {
  LABEL,
  VARIABLE
};

// Store global file for output
std::string output_filename;
std::ofstream out_fp;

std::string generate_temp(int type) {
  std::string base;

  if (type == LABEL) {
    base += LABEL_PREFIX + std::to_string(total_temp_labels);

    total_temp_labels++;
  }
  else if (type == VARIABLE) {
    base += VARIABLE_PREFIX + std::to_string(total_temp_vars);
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

// Assist in writing all global variables/temporaries to assembly file
void write_global_vars() {
  out_fp << "\n";

  for (unsigned int i = 0; i < MAX_SIZE; i++) {
    if (temp_stack[i] == "") { continue; }
    else {
      // "Initialize" variables to 0
      write_asm(temp_stack[i], "0");
    }
  }
}

// Assist in handling relational operators
void write_RO(Token_Type tk, std::string t_var, std::string t_label) {
  // Each asm output should be the exit condition
  // Otherwise the code within each <stat> is executed

  // <> Must use BRZ since 3 > 3 is not greater
  // > is greater than
  // 4 - 3 = 1
  // True as long as result > 0
  if (tk == TK_GREATER_THAN) {
    write_asm("SUB", t_var);
    write_asm("BRZNEG", t_label);
  }
  // < is less than
  // 3 - 4 = 1
  // True as long as result < 0
  else if (tk == TK_LESS_THAN) {
    write_asm("SUB", t_var);
    write_asm("BRZPOS", t_label);
  }
  // { == } is NOT equal
  // 4 - 4 != 0
  // True as long as result is not 0
  else if (tk == TK_L_BRACE) {
    write_asm("SUB", t_var);
    write_asm("BRZERO", t_label);
  }
  // == is equal
  // 4 - 4 = 0
  // True as long as result is 0
  // Should only exit on variation where not 0
  else if (tk == TK_EQUALS_EQUALS) {
    write_asm("SUB", t_var);
    write_asm("BRPOS", t_label);
    write_asm("BRNEG", t_label);
  }
  // % returns true if the signs of the arguments are the same (with a 0 counting as both, so a -1 and a 0 would evaluate as true, so would a 1 and a 0)
    // Note: Just multiply them and look for sign changes
    // if same signs, a * b >= 0; -1 * -1 >= 0;
    // negating exit condition would be BRNEG since anything >= 0 is true
  else if (tk == TK_PERCENT) {
    write_asm("MULT", t_var);
    write_asm("BRNEG", t_label);
  }
}

// Helper functions to work with stack items
void push(Token tk) {
  // Make sure that there is still room in the stack
  if (total_vars >= MAX_SIZE) {
    std::cout << "\nSemantic Error: Max number of stack items exceeded. Limit 100. Total Items: "
      << total_vars << std::endl;

    s_cleanup();

    exit(EXIT_FAILURE);
  }

  // Make sure no duplicate vars are declared in the same scope
  for (unsigned int current_scope = base_scope; current_scope < total_vars; current_scope++) {

    if (tk_stack[current_scope].token_instance == tk.token_instance) {
      std::cout << "Semantic Error: There was a variable already declared in this scope. Variable: "
        << tk.token_instance << " on line " << tk.line_num << std::endl;

      s_cleanup();

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
  for (unsigned int current_scope = total_vars; current_scope > base_scope; current_scope--) {
    if (tk.token_instance == tk_stack[current_scope].token_instance) {
      // Calculate the distance from the top of the stack
      // Offset by one since arrays start at 0
      return (total_vars - 1) - current_scope;
    }
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
  output_filename = filename;
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

  /* std::cout << "Next Process Point: " << label << std::endl; */

  /* print_vars(); */

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


    // If not found then it is valid
    // n>=0 (the variable was found on the stack), then issue to the target
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

      s_cleanup();

      exit(EXIT_FAILURE);
    }

    // iterate over remaining children, if any
    iterate_children(root->children, var_count);
  }
  // <block> -> start <vars> <stats> stop
  else if (label == "<block>") {
    unsigned int local_var_count = 0;

    // Store scope for current block
    // Used to remove from stack once scope ends
    base_scope = total_vars;

    // <vars> and <stats>
    iterate_children(root->children, local_var_count);

    // Remove a scope level once finished with block
    pop();
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

          s_cleanup();

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

      s_cleanup();

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
    // [   0       1     2       3           4         ]
    // [ <expr>, <RO>, <expr>, <stat>, optional <stat> ]
    Token temp_tk = root->children[1]->consumed_tokens[0];
    Token_Type temp_tk_id = temp_tk.token_ID;

    std::string temp_var = generate_temp(VARIABLE);

    // Get value of second <expr>
    process_semantics(root->children[2], var_count);
    write_asm("STORE", temp_var);

    // Get value of first <expr>
    process_semantics(root->children[0], var_count);

    // evaluate <expr> <RO> <expr>
    // If True, then continue; if false, jump to ELSE
    //    continue section of code, evaluate <stat>
    //    more code
    //    ...
    //    jump L_ENDIF
    // ELSE Skip above code
    //    execute code here
    //    ...
    //    continue on
    // L_ENDIF

    bool has_else = root->children.size() == 5 ? true: false;

    std::string temp_end_if_label = generate_temp(LABEL);

    // Evaluate <RO> branches and adjust labels
    if (has_else) {
      std::string temp_else_label = generate_temp(LABEL);

      // Normal if then, but now else will be exit point
      write_RO(temp_tk_id, temp_var, temp_else_label);

      // statements inside if section
      // Should also jump to end if label when if expression is true
      process_semantics(root->children[3], var_count);
      write_asm("BR", temp_end_if_label);

      // Otherwise move onto the else <stat>
      // end of else will be next to end of general if label
      write_asm(temp_else_label + ":", "NOOP");
      process_semantics(root->children[4], var_count);
    }
    // Normal if then
    else {
      write_RO(temp_tk_id, temp_var, temp_end_if_label);
      process_semantics(root->children[3], var_count);
    }

    // Write the closing label position in both cases
    // Concludes the end of an if/if-else chain
    write_asm(temp_end_if_label + ":", "NOOP");
  }
  // <loop> -> while [ <expr> <RO> <expr> ] <stat>
  else if (label == "<loop>") {
    // [<expr>, <RO>, <expr>, <stat>]
    Token temp_tk = root->children[1]->consumed_tokens[0];
    Token_Type temp_tk_id = temp_tk.token_ID;

    // Get a temp var for storage
    std::string temp_var = generate_temp(VARIABLE);

    // Get temp labels
    std::string temp_start_label = generate_temp(LABEL);
    std::string temp_end_label = generate_temp(LABEL);

    // Declare start of loop label
    write_asm(temp_start_label + ":", "NOOP");

    // Evaluate second <expr> and store value
    process_semantics(root->children[2], var_count);
    write_asm("STORE", temp_var);

    // Evaluate other <expr>
    process_semantics(root->children[0], var_count);

    // Evaluate <RO>
    write_RO(temp_tk_id, temp_var, temp_end_label);

    // Iterate <stat>
    process_semantics(root->children[3], var_count);

    // Declare end of loop
    write_asm("BR", temp_start_label);
    write_asm(temp_end_label + ":", "NOOP");
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

      s_cleanup();

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
    std::string t_label = root->consumed_tokens[1].token_instance;

    // Identifier
    int position = find(temp_tk);

    // If not found then it is valid
    if (position == -1 || position > var_count) {
      temp_tk.token_instance = LABEL_PREFIX + t_label;
      push(temp_tk);

      // No children left over at this point
      // Initialize labels to NOOP
      write_asm(LABEL_PREFIX + t_label + ":", "NOOP");

      var_count++;
    }
    // If found within the stack of currently stored
    else if (position < var_count) {
      std::cout << "Semantic Error: Identifier declared more than once."
        << "\n\t Instance: " << t_label
        << "\n\t Line: " << temp_tk.line_num
        << std::endl;

      s_cleanup();

      exit(EXIT_FAILURE);
    }
  }
  // <goto> -> jump Identifier
  else if (label == "<goto>") {
    // Only process if the goto is already defined
    Token temp_tk = root->consumed_tokens[1];

    // Identifier
    int position = check_vars(LABEL_PREFIX + temp_tk.token_instance);

    // If no instance cannot be found
    if (position == -1) {
      std::cout << "Semantic Error: Usage of undeclared label identifier."
        << "\n\t Instance: " << temp_tk.token_instance
        << "\n\t Line: " << temp_tk.line_num
        << std::endl;

      s_cleanup();

      exit(EXIT_FAILURE);
    }
    // Otherwise allow the jump to occur
    else {
      write_asm("BR", LABEL_PREFIX + temp_tk.token_instance);
    }
  }
  // Most things should be able to just keep recursively iterating their children
  // Not containing vars specifically
  else {
    iterate_children(root->children, var_count);
  }
}

// Remove temp file
void s_cleanup() {
  // Close the temp stream
  out_fp.close();

  // Delete the temp file if there was an error
  std::string default_file_name = output_filename;
  std::remove(default_file_name.c_str());
}
