#ifndef __STRING_HPP__
#define __STRING_HPP__

// ----------------------------
// ----- WORK IN PROGRESS -----
// ----------------------------
// I currently do not see how strings are really usefull.
// They probably should make it easier to use Allocators and string.



// Requirements:
// -------------
// Strings need to be usable with allocators.
// It should be easy to work with strings.
//
// Easy:
// str("Something") + " " + str(15) would be cool
// str("Something") would be nice
// str(15.17f);
//
// We want string because working with char* is tedious
// char** filenames = malloc(sizeof(char*) * 5);
// size_t len = strlen(asdf);
// filenames[2] = malloc(sizeof(char) * len);
// strcpy(filenames[2], asdf);
//
// String* arr = malloc(sizeof(String) * 5);
// test[0].init(asdf); // Saves 2 lines
//
// Maybe String.format(
// setStringAllocator
// pushStringAllocator
// popStringAllocator
//
// This should work
// String s = "asdf";
// printf("%s", &s);






#endif
