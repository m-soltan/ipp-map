#ifndef MAP_TRIE_H
#define MAP_TRIE_H
#include <stdbool.h>
#include "global_declarations.h"

void trieDestroy(Trie **pTrie);

City *trieFind(Trie *trie, const char *str);
bool trieAddFromList(Trie *trie, NameList list, City *const *cities);

bool trieInsert(Trie *trie, const char *str, City *city);
Trie *trieInit(void);

#endif //MAP_TRIE_H
