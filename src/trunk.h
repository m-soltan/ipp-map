#ifndef MAP_TRUNK_H
#define MAP_TRUNK_H

#include <stdbool.h>
#include "global_declarations.h"

bool trunkHasCity(const Trunk *trunk, const City *city);
char *trunkDescription(const Trunk *trunk);
size_t trunkGetLength(const Trunk *trunk);
unsigned trunkGetId(Trunk *trunk);
void trunkAttach(Trunk *trunk);
void trunkDestroy(Trunk **pTrunk);
void trunkBlock(Trunk *trunk);
void trunkFree(Trunk **pTrunk);
void trunkUnblock(Trunk *trunk);
Trunk *trunkAddDetour(CityMap *cityMap, Trunk *trunk, Road *road);
Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned trunkId);
Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *c);
Trunk *trunkMake(unsigned id, City *first, City *last, Road *const *roads, size_t roadCount);

#endif //MAP_TRUNK_H
