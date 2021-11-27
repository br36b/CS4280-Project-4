Empty file is not valid and will be allowed to be progress past program initialization.

Program can be compiled using provided Makefile.

Usage: `./frontEnd [file]` or `./frontEnd < [file]`

Program will find the longest match of symbols that work. Upon a state change it will consider that a wrapped up token. This means that if a number collides with a letter, it will just split it into a int and begin working on an identifier/other token.

Comments work but will mess with the line numbers if too many invalid consecutive comment symbol errors occur. Will still either give invalid char or pair missing message. Remade to allow dangling comment at EOF.

Identifiers created using $, $ will not contribute to significant chars. Otherwise regular limit is followed.
