#ifndef MAP_TRUNK_H
#define MAP_TRUNK_H

#include <stdbool.h>
#include "global_declarations.h"

bool trunkHasCity(const Trunk *trunk, const City *city);
bool trunkTest(const Trunk *trunk);
char *trunkDescription(const Trunk *trunk);
size_t trunkGetLength(const Trunk *trunk);
unsigned trunkGetId(Trunk *trunk);
void trunkAttach(Trunk *trunk);
void trunkDestroy(Trunk **pTrunk);
void trunkFree(Trunk **pTrunk);
Trunk *trunkAddDetour(CityMap *cityMap, Trunk *trunk, Road *road);
Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned trunkId);
Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *c);
Trunk *trunkMake(Trie *trie, unsigned id, NameList list);

#endif //MAP_TRUNK_H
