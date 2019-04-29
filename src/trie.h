#ifndef MAP_TRIE_H
#define MAP_TRIE_H
#include <stdbool.h>
#include "global_declarations.h"

Trie **trieInsert(Trie *trie, const char *str, City **city, bool *success);
City *trieFind(Trie *x, const char *str);
Trie *trieInit();
void trieDestroy(Trie **x);

#endif //MAP_TRIE_H
