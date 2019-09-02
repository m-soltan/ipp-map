/** @file
 * Interface for a trie structure mapping strings to cities.
 */

#ifndef MAP_TRIE_H
#define MAP_TRIE_H
#include <stdbool.h>
#include "global_declarations.h"

/// destroy a Trie structure
void trieDestroy(Trie **pTrie);
/// find a record in the structure and return it
City *trieFind(Trie *trie, const char *str);
/// insert every city from the list into the trie
bool trieAddFromList(Trie *trie, NameList list, City *const *cities);
/// insert into the Trie structure
bool trieInsert(Trie *trie, const char *str, City *city);
/// initialize a trie
Trie *trieInit(void);

#endif //MAP_TRIE_H
