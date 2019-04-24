#ifndef MAP_TRIE_H
#define MAP_TRIE_H
#include <stdbool.h>
#include "city.h"

typedef struct City City;
typedef struct Trie Trie;

Trie **trieInsert(Trie *trie, const char *str, City **city, bool *success);
City *trieFind(Trie *x, const char *str);
Trie *trieInit();
void trieDestroy(Trie **x);

#endif //MAP_TRIE_H
