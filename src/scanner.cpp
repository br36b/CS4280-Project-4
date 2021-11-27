#include <iostream>
#include <map>

#include "scanner.h"

// start stop loop while for label exit listen talk program if then assign declare jump else =====> 16
// =  >  < ==  : :=  +  -  *  /   %  . (  ) , { } ; [ ] =======> 20
// letter digit EOF WS ====> 4

// 0, 1, ... states/rows
// -1, -2, ... errors
// 1001, 1002, ... final states for tokens
/* Things to recognize
 *   Basic Data (Case sensitive)
 *    Alphabet letters (upper and lower)
 *    Digits
 *    White Space
 *   Basic Tokens
 *    Identifiers/Keywords
 *    Integers
 *    Also symbols
*/

// 20 symbols counted from unique symbols etc but some require combination of symbols
  // -2 combos (==, :=)
  // = 18
  // + 1 $ for identifiers
  // + 1 for digits
  // = 20
  // + 3 EOF UppercaseLetter LowercaseLetter
  // + 1 WS
  // = 24 total symbols (cols)
// Rows 20 initial symbols
  // +1 identifier (clarify keywords later on)
  // +1 integer
  // +1 neutral state
  // = 23 total states (rows)
const unsigned int ROW_NUM = 23;
const unsigned int COL_NUM = 24;

// For alternating error messages
const int DEFAULT_ERROR_VALUE = -2;
const int CASE_SENSITIVE_ERROR = -3;
const unsigned int MAX_TOKEN_LENGTH = 8;

// Identifiers MUST begin with a lowercase letter or $
  // 8 total digits/letters for length
  // letter case matters,
int FSATable[ROW_NUM][COL_NUM] = {
  //   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23
  //   WS   LL    UL     D     $     =     >     <     :     +    -      *     /     %     .     (     )     ,     {     }     ;     [     ]   EOF
  {    0,    1,   -3,    2,    1,    3,    4,    5,    7,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   -1 }, // s0   Initial Symbols States
  { 1000,    1,    1,    1, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000 }, // s1   TK_ID
  { 1001, 1001, 1001,    2, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001, 1001 }, // s2   TK_INT
  { 1002, 1002, 1002, 1002, 1002,    6, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002, 1002 }, // s3   TK_EQUALS
  { 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003, 1003 }, // s4   TK_GREATER_THAN
  { 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004, 1004 }, // s5   TK_LESS_THAN
  { 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005, 1005 }, // s6   TK_EQUALS_EQUALS
  { 1006, 1006, 1006, 1006, 1006,    8, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006, 1006 }, // s7   TK_COLON
  { 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1007 }, // s8   TK_COLON_EQUALS
  { 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008, 1008 }, // s9   TK_PLUS
  { 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009, 1009 }, // s10  TK_MINUS
  { 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010 }, // s11  TK_STAR
  { 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011, 1011 }, // s12  TK_SLASH
  { 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012, 1012 }, // s13  TK_PERCENT
  { 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013, 1013 }, // s14  TK_PERIOD
  { 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014, 1014 }, // s15  TK_L_PAREN
  { 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015, 1015 }, // s16  TK_R_PAREN
  { 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016, 1016 }, // s17  TK_COMMA
  { 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017, 1017 }, // s18  TK_L_BRACE
  { 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018, 1018 }, // s19  TK_R_BRACE
  { 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1019 }, // s20  TK_SEMICOLON
  { 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020, 1020 }, // s21  TK_L_BRACKET
  { 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1021 }, // s22  TK_R_BRACKET
};

// Store Symbol Columns
// Should be able to search for symbol

// Adding these for non-char symbols
const unsigned int WS_COL = 0;
const unsigned int LOWERCASE_LETTER_COL = 1;
const unsigned int UPPERCASE_LETTER_COL = 2;
const unsigned int DIGIT_COL = 3;
const unsigned int EOF_COL = 23;

// Pair symbols with their respective row in the FSA table
std::map<char, int> symbol_columns = {
  { '$', 4 },     // $, just for identifiers it seems
  { '=', 5 },     // =
  { '>', 6 },     // >
  { '<', 7 },     // <
  { ':', 8 },     // :
  { '+', 9 },     // +
  { '-', 10 },    // -
  { '*', 11 },    // *
  { '/', 12 },    // /
  { '%', 13 },    // %

  { '.', 14 },    // .
  { '(', 15 },    // (
  { ')', 16 },    // )
  { ',', 17 },    // ,
  { '{', 18 },    // {
  { '}', 19 },    // }
  { ';', 20 },    // ;
  { '[', 21 },    // [
  { ']', 22 },    // ]
};

// Store reserved words
// Suggested to just take identifiers tokens and verifiy against keyword list
std::map<std::string, Token_Type> reserved_keywords = {
  { "start"   , TK_START },       // start
  { "stop"    , TK_STOP },        // stop
  { "loop"    , TK_LOOP },        // loop
  { "while"   , TK_WHILE },       // while
  { "for"     , TK_FOR },         // for
  { "label"   , TK_LABEL },       // label
  { "exit"    , TK_EXIT },        // exit
  { "listen"  , TK_LISTEN },      // listen
  { "talk"    , TK_TALK },        // talk
  { "program" , TK_PROGRAM },     // program
  { "if"      , TK_IF },          // if
  { "then"    , TK_THEN },        // then
  { "assign"  , TK_ASSIGN },      // assign
  { "declare" , TK_DECLARE },     // declare
  { "jump"    , TK_JUMP },        // jump
  { "else"    , TK_ELSE },        // else
};

// Store states to pair their token values
std::map<int, Token_Type> final_token_states = {
  // Errors
  { -2, TK_ERROR }, // Will also be used to cover cases of bad identifer

  // EOF
  { -1, TK_EOF },

  // Identifiers/Integer
  { 1000, TK_ID },
  { 1001, TK_INT },

  // Operators and delimiters
  // Note: Do not use assign for = since it's a keyword
  { 1002, TK_EQUALS },            // =
  { 1003, TK_GREATER_THAN },      // >
  { 1004, TK_LESS_THAM },         // <
  { 1005, TK_EQUALS_EQUALS },     // ==
  { 1006, TK_COLON },             // :
  { 1007, TK_COLON_EQUALS },      // :=
  { 1008, TK_PLUS },              // +
  { 1009, TK_MINUS },             // -
  { 1010, TK_STAR },              // *
  { 1011, TK_SLASH },             // /
  { 1012, TK_PERCENT },           // %

  { 1013, TK_PERIOD },            // .
  { 1014, TK_L_PAREN },           // (
  { 1015, TK_R_PAREN },           // )
  { 1016, TK_COMMA },             // ,
  { 1017, TK_L_BRACE },           // {
  { 1018, TK_R_BRACE },           // }
  { 1019, TK_SEMICOLON },         // ;
  { 1020, TK_L_BRACKET },         // [
  { 1021, TK_R_BRACKET },         // ]
};

// Function to return the column using constants/symbols of FSA table
int find_col(char c) {

  // Verify states 1-3 first since they are constants
  // (WS, UL, LL, D)
  if (isspace(c)) {
    return WS_COL;
  }
  // Check if a letter was passed
  else if (isalpha(c)) {
    // Since this is case sensitive, differentiate between lower/upper
    // Check if it is lowercase
    if (islower(c)) {
      return LOWERCASE_LETTER_COL;
    }
    // Otherwise it should be uppercase
    else {
      return UPPERCASE_LETTER_COL;
    }
  }
  // Check for digits for integers
  else if (isdigit(c)) {
    return DIGIT_COL;
  }

  // Once those are checked char symbols can be checked to see if they match
  auto search = symbol_columns.find(c);

  // No match was found
  if (search == symbol_columns.end()) {
    return DEFAULT_ERROR_VALUE;
  }
  // A match was found
  else {
    return symbol_columns[c];
  }
}

// Remove anything between && symbols
// At the moment will just eat everything until end of line or file
bool remove_comments(std::ifstream &in_fp, unsigned int &line_num, char &current_char) {
  /* std::cout << "Comment Detected" << std::endl; */

  // Verify pair of &&
  // First one is pre-checked by calling this function
  char next_char;

  // Therefore we should have a second
  // Otherwise it is invalid, eat entire line
  // Check before eating it
  // &&
  //  ^
  if (in_fp.peek() != '&') {
    // Eat the rest of the line, keep newlines for automatic increment
    while (!in_fp.eof() && in_fp.peek() != '\n') {
      /* std::cout << "DC: " << current_char << " " << std::string(1, in_fp.peek()) << std::endl; */
      in_fp.get(current_char);
    }

    if (in_fp.peek() != '\n') {
      in_fp.get(current_char);
    }

    std::cout << "\nSCANNER ERROR: L" << line_num
      << ": Invalid comment. Missing starting pair of '&'" << std::endl;


    return false;
  }
  // Safe to take a character
  // Otherwise take the next
  // should now be &&
  //                ^
  // Hold forward looking char
  else {
    in_fp.get(next_char);
  }
  // should now be &&c or a whitespace
  //                 ^
  in_fp.get(next_char);
  in_fp.get(next_char);

  // Exit if there are no more characters (dangling comment support at end of file)
  // Explicitly for a solo && at the end of a file
  // EOF must be placed here because it only triggers once a read happens past end of file
  if (in_fp.eof()) {
    return true;
  }

  // Undo attempt to trigger eof
  in_fp.putback(next_char);

  // A valid pair will be next to each other with same symbol
  do {
    /* std::cout << "CC: " << next_char << " " << std::string(1, in_fp.peek()) << std::endl; */

    // If there is no match found by EOF, it failed
    if (in_fp.eof()) {
      std::cout << "\nSCANNER ERROR: L" << line_num
        << ": Invalid comment. EOF reached. Missing ending pair of '&'" << std::endl;

      return false;
    }

    // If it hits a line ending then it likely failed
    if (next_char == '\n') {
      std::cout << "\nSCANNER ERROR: L" << line_num
        << ": Invalid comment. New line hit. Missing ending pair of '&'" << std::endl;

      return false;
    }

    // Keep taking characters until a match is found
    current_char = next_char;
    in_fp.get(next_char);
  } while (!(next_char == '&' && in_fp.peek() == '&'));

  // Should only get here once loop condition is fulfilled
  // Eat the two matching &&
  in_fp.get(current_char);
  in_fp.get(current_char);

  // Note: Must store this to be used for state lookup

  return true;
}

// Tester will ask scanner for one token at a time
Token scanner(std::ifstream &in_fp, unsigned int &line_num) {
  char temp_char;

  std::string instance;

  /* std::cout << "Scanning..." << std::endl; */

  // s1 = 0
  int current_state = 0;
  int next_state = 0;
  int symbol_col = 0;

  bool is_valid_comment;


  // Non-final states error states => 0 < x < 1000
  // "while state is not final"
  while (-1 < current_state && current_state < 1000) {
    // Get the char
    in_fp.get(temp_char);

    if (temp_char == '&') {
      is_valid_comment = remove_comments(in_fp, line_num, temp_char);

      // If there was an error with a comment return an error token
      if (!is_valid_comment) {
        return Token(TK_ERROR, "Invalid Comment", line_num);
      }
    }


    /* std::cout << temp_char << std::endl; */

    // Set as EOF if present
    if (in_fp.eof()) {
      symbol_col = EOF_COL;
    }
    // Otherwise find the column of the symbol
    else {
      symbol_col = find_col(temp_char);

      if (symbol_col == DEFAULT_ERROR_VALUE) {
        std::cout << "\nSCANNER ERROR: L" << line_num
          << ": Invalid character: '" << temp_char << "'"
          << std::endl;

        return Token(TK_ERROR, std::string(1, temp_char), line_num);
      }
    }

    next_state = FSATable[current_state][symbol_col];

    // If it is still building and valid
    if (-1 < next_state && next_state < 1000) {
      /* Identifiers
          begin with a lower case letter or a $
          continue with any number of letters (uppercase or lowercase) or digits, 8 significant total
          you may assume no identifier is longer than 8 characters (in testing)
        Integers
          any sequence of decimal digits, no sign, no decimal point, up to 8 significant
       *
      */

      // Make sure that spaces are not contributing to a token
      // get() will eat them
      if (!isspace(temp_char)) {
        instance.push_back(temp_char);
      }
      // Make sure that they do not exceed the max length
        // Check to see if it is true if $ used as ident
        // This assumes that $ is not significant as a char
      if ((instance.length() <= MAX_TOKEN_LENGTH)
          || (instance[0] == '$' && (instance.length() - 1 <= MAX_TOKEN_LENGTH))) {

        // Increment line number for token output
        if (temp_char == '\n') {
          line_num++;
          /* std::cout << "LINE NUM: " << line_num << std::endl; */
        }

        // Swap states
        current_state = next_state;
      }
      // If it exceeds the limit then return an error
      else {
        std::cout << "\nSCANNER ERROR: L" << line_num
          << " Invalid length for ident/int: " << instance
          << std::endl;

        return Token(TK_ERROR, instance, line_num);
      }
    }
    // Otherwise the Token has reached a final state
    else {
      // Send EOF Desc
      if (next_state == -1) {
        line_num--; // Offset from comment newlines
        return Token(TK_EOF, "End of File", line_num);
      }
      // Send error desc
      else if (next_state == DEFAULT_ERROR_VALUE) {
        std::cout << "\nSCANNER ERROR: L" << line_num
          << " Invalid character " << temp_char
          << " in " << instance
          << std::endl;

        return Token(TK_ERROR, instance, line_num);
      }
      else if (next_state == CASE_SENSITIVE_ERROR) {
        std::cout << "\nSCANNER ERROR: L" << line_num
          << ": Invalid identifier start character: '" << temp_char << "'"
          << std::endl;

        return Token(TK_ERROR, std::string(1, temp_char), line_num);
      }

      // Because this is a final state make sure to avoid eating space
      // Could end up eating a newline
      in_fp.unget();

      // Search states mapped to find type
      auto search_final_state = final_token_states.find(next_state);

      // If no match then there was an error
      if (search_final_state == final_token_states.end()) {
        std::cout << "\nSCANNER ERROR: L" << line_num
          << " Invalid token " << instance
          << std::endl;

        return Token(TK_ERROR, instance, line_num);
      }
      // Otherwise a match was found, check if it's a keyword
      auto search_keywords = reserved_keywords.find(instance);

      // Not required if not found, otherwise just set to id/other
      if (search_keywords != reserved_keywords.end()) {
        return Token(reserved_keywords[instance], instance, line_num);
      }

      /* std::cout << "Final Token Found "; */
      /* std::cout << next_state << " L" << line_num << std::endl; */

      return Token(final_token_states[next_state], instance, line_num);
    }
  }

  std::cout << "C: " << temp_char << std::endl;

  // Default error state
  return Token(TK_ERROR, "\nSCANNER ERROR: Critial error found.", line_num);
}
