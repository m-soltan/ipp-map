#ifndef MAP_TRUNK_H
#define MAP_TRUNK_H

#include <stdbool.h>
#include "global_declarations.h"

bool trunkHasCity(const Trunk *trunk, const City *city);
const char *trunkDescription(const Trunk *trunk);
Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned number);
Trunk *trunkDetour(CityMap *cMap, Trunk *trunk);
Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *c);
unsigned *trunkBlock(Trunk *trunk);
void trunkAttach(Trunk *trunk);
void trunkFree(Trunk **pTrunk);
void trunkUnblock(Trunk *trunk, unsigned *lengths);


#endif //MAP_TRUNK_H
