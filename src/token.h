#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <map>

// Enum values for each token
enum Token_Type {
  // End of file and error tokens
  TK_EOF,
  TK_ERROR,

  // Identifier
  TK_ID,

  // Integer
  TK_INT,

  // Keywords
  TK_START,     // start
  TK_STOP,      // stop
  TK_LOOP,      // loop
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_LABEL,     // label
  TK_EXIT,      // exit
  TK_LISTEN,    // listen
  TK_TALK,      // talk
  TK_PROGRAM,   // program
  TK_IF,        // if
  TK_THEN,      // then
  TK_ASSIGN,    // assign
  TK_DECLARE,   // declare
  TK_JUMP,      // jump
  TK_ELSE,      // else

  // Operators and delimiters
  // Note: Do not use assign for = since it's a keyword
  TK_EQUALS,        // =
  TK_GREATER_THAN,  // >
  TK_LESS_THAM,     // <
  TK_EQUALS_EQUALS, // == give literal name for now
  TK_COLON,         // :
  TK_COLON_EQUALS,  // :=  Assignment op?
  TK_PLUS,          // +
  TK_MINUS,         // -
  TK_STAR,          // *
  TK_SLASH,         // /
  TK_PERCENT,       // %

  TK_PERIOD,        // .
  TK_L_PAREN,       // (
  TK_R_PAREN,       // )
  TK_COMMA,         // ,
  TK_L_BRACE,       // {
  TK_R_BRACE,       // }
  TK_SEMICOLON,     // ;
  TK_L_BRACKET,     // [
  TK_R_BRACKET,     // ]
};

// Unused for now
struct Line_Data {
  unsigned int line_num;
  unsigned int char_num;

  Line_Data() {
    this->line_num = 0;
    this->char_num = 0;
  }
};

struct Token {
  Token_Type token_ID;
  std::string token_instance;
  unsigned int line_num;

  // Set default state
  Token() {
    this->token_ID = TK_ERROR; // Error by default
    this->token_instance = "";
    this->line_num = 0;
  }

  // Set all elements of token in scanner tokens
  Token(Token_Type tk_type, std::string instance, unsigned int current_line) {
    this->token_ID = tk_type;
    this->token_instance = instance;
    this->line_num = current_line;
  }
};

#endif
