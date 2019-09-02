/** @file
 * Parser for the textual program interface.
 */

#ifndef MAP_PARSER_H
#define MAP_PARSER_H

#include <stdbool.h>

// errors
/// return code when argument is invalid
#define INVALID_ARG 1
/// return code when allocation failed
#define OUT_OF_MEMORY (-1)

/// initialize the global map
bool setMap(void);
/// read a single line from stdin
char *getLine(void);
/// start parsing input
int runParser(void);
/// process the line and execute command
void parserRead(char *line);
/// print an error message to stderr
void writeError(void);

#endif // MAP_PARSER_H
