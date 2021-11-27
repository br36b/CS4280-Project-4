/*
 * Name: Bryan Rojas
 * CS4280
 * Date: 11/13/21
 * Purpose: Build off previous project to now parse through scanner tokens from file/keyboard input and identify tokens into output
*/

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdio.h>

#include "parser.h"
#include "tree_traversal.h"

void create_file_from_input(std::string);
void attempt_to_open_file(std::ofstream &, std::string);
void load_input_fp(std::ifstream &, std::string);

void cleanup();

const std::string TEMP_DATA_PREFIX = "temp_dat123";
const std::string INPUT_FILE_SUFFIX = ".fl2021";

int main(int argc, char *argv[]) {
  // String constants for file names/ending
  std::string base_filename;

  // Before doing anything make sure no excess params
  if (argc > 2) {
    std::cout << "Excess arguments given. Exiting.\n" << std::endl;
    exit(EXIT_FAILURE);
  }
  // No input file, read from keyboard
  else if (argc == 1) {
    std::cout << "No file provided. Taking input (CTRL-D to end): " << std::endl;

    // Set default file and create it with data inputted
    base_filename = TEMP_DATA_PREFIX;
    create_file_from_input(base_filename + INPUT_FILE_SUFFIX);
  }
  // Input file provided
  else if (argc == 2) {
    std::cout << "File provided. Verifying. " << std::endl;

    // Take the arg and store it
    base_filename = argv[1];

    int file_length = base_filename.length();

    // If it has an extension
    // .fl2021 will be 7 chars + 1 letter
    // If it has a dot, then it is a wrong length for an extension
    // Given that 8 will be the minimum
    if (base_filename.find(".") != std::string::npos) {
      /* std::cout << base_filename << " " << file_length << std::endl; */

      // By this point it has a dot and should be at least 8 chars
      if (base_filename.find("fl2021") != std::string::npos) {
        if (file_length < 8) {
          std::cout << "File is missing a name for this extension.\n" << std::endl;
          exit(EXIT_FAILURE);
        }
      }
      // Otherwise it is an incorrect format that does not contain it
      else {
        std::cout << "File is an incorrect format. Requires *.fl2021 if provided.\n" << std::endl;
        exit(EXIT_FAILURE);
      }

      // Substring removal of file ending since it is implied in program
      size_t last_index = base_filename.find_last_of(".");
      base_filename = base_filename.substr(0, last_index);
    }
    // Otherwise it doesn't contain any file ending, so it is implied to add the extension
    // Which will work out on the build_tree() process
  }

  // Construct the entire filename into designated format
  // *.fl2021
  const std::string FINAL_FILENAME = base_filename + INPUT_FILE_SUFFIX;

  // Hold a file for inputting into respective ending locations
  std::ifstream temp_stream;
  load_input_fp(temp_stream, FINAL_FILENAME);

  // Begin parser
  Node *root = parser(temp_stream);

  if (root == nullptr) {
    std::cout << "Parser failed to load data." << std::endl;
    cleanup();

    exit(EXIT_FAILURE);
  }

  // Use preorder traversal once complete
  std::cout << "\nOutputting Pre-Order Traversal" << std::endl;
  print_pre_order(root);

  // Close temp stream
  temp_stream.close();

  cleanup();

  // Just for exit formatting
  std::cout << std::endl;

  return 0;
}


// Will generate a temp_file in the directory.
void create_file_from_input(std::string filename) {
  // Take and store some input temporarily
  std::ofstream out_stream;
  attempt_to_open_file(out_stream, filename);

  std::string input;

  while (getline(std::cin, input)) {
    // Write to file, separate with newline for breaking
      // Swapped from just `cin >> input` to `getline`
      // This was so redirections could be handled properly
      // Otherwise it eats the newline char and scanner never sees them
    out_stream << input << "\n";
  }

  // Close the file after input is done
  out_stream.close();
}

// Helper to verify and store the pointer of a file
void attempt_to_open_file(std::ofstream &temp, std::string filename) {
  // Attempt to open path given
  temp.open(filename.c_str());

  // Exit if no file could be created for output
  if (temp.fail()) {
    std::cout << "Failed to create a file for data output. Exiting.\n" << std::endl;

    exit(EXIT_FAILURE);
  }
}

// Take file and get it ready for data reading
void load_input_fp(std::ifstream &temp, std::string filename) {
  temp.open(filename.c_str());

  // Check to see if file can be read from
  if (temp.fail()) {
    std::cout << "Failed to load file for data input. Exiting.\n" << std::endl;

    exit(EXIT_FAILURE);
  }

  // Check to see if the file contains data
  if (temp.peek() == std::ifstream::traits_type::eof()) {
    std::cout << "No data was found in the file.\n" << std::endl;

    temp.close();
    exit(EXIT_FAILURE);
  }
}

// Remove temp file
void cleanup() {
  // Delete the temp file if it exists
  std::string default_file_name = TEMP_DATA_PREFIX + INPUT_FILE_SUFFIX;
  std::remove(default_file_name.c_str());
}
