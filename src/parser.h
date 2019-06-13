#ifndef MAP_PARSER_H
#define MAP_PARSER_H


#include <stdbool.h>

// errors
#define INVALID_ARG 1
#define OUT_OF_MEMORY -1

bool setMap();
char *getLine(void);
int runParser(void);
void parserExamples(void);
void parserRead(char *line);
void writeError(void);


#endif // MAP_PARSER_H
