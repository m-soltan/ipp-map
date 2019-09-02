/** @file
 * Interface for a structure storing information about Routes.
 */

#ifndef MAP_TRUNK_H
#define MAP_TRUNK_H

#include <stdbool.h>
#include "global_declarations.h"

/// check if a trunk goes through a given city
bool trunkHasCity(const Trunk *trunk, const City *city);
/// debug function, check invariants within a trunk
bool trunkTest(const Trunk *trunk);
/// provide a string description of a Route
char *trunkDescription(const Trunk *trunk);
/// get the length of a trunk
size_t trunkGetLength(const Trunk *trunk);
/// get an id of the trunk
unsigned trunkGetId(Trunk *trunk);
/// record usage of a trunk's roads
void trunkAttach(Trunk *trunk);
/// destroy a Trunk structure
void trunkDestroy(Trunk **pTrunk);
/// release used resources and set pointer to NULL
void trunkFree(Trunk **pTrunk);
/// replace a removed road in a trunk with a detour section
Trunk *trunkAddDetour(CityMap *cityMap, Trunk *trunk, Road *road);
/// create a Trunk from one city to another
Trunk *trunkBuild(City *from, City *to, CityMap *m, unsigned trunkId);
/// extend a trunk to reach a city
Trunk *trunkExtend(CityMap *cityMap, Trunk *trunk, City *c);
/// initialize a Trunk structure
Trunk *trunkMake(Trie *trie, unsigned id, NameList list);

#endif //MAP_TRUNK_H
