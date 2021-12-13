Storage=Local

All given test files passed. Added some if-else and RO symbol testing.

Fixed an issue with <M> breakdown in parser. Should work properly now when branching unary symbols and expressions.

Made labels and variable identifiers different in context. Previously disallowed them to have the same identifier.
Prefixed with `L_Identifier`.

Empty file is not valid and will not be allowed to be progress past program initialization.

Program can be compiled using provided Makefile.

Usage: `./compfs [file]` or `./compfs < [file]`

Program will find the longest match of symbols that work. Upon a state change it will consider that a wrapped up token. This means that if a number collides with a letter, it will just split it into a int and begin working on an identifier/other token.

Comments work but will mess with the line numbers if too many invalid consecutive comment symbol errors occur. Will still either give invalid char or pair missing message. Remade to allow dangling comment at EOF.

Identifiers created using $, $ will not contribute to significant chars. Otherwise regular limit is followed.
