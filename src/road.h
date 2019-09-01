#ifndef MAP_ROAD_H
#define MAP_ROAD_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

bool cityMapLoneRoad(CityMap *cityMap, Trie *trie, RoadInfo roadInfo);

bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info);
bool roadHasCity(const Road *road, const City *city);
bool roadHasIntersection(const Road *road1, const Road *road2);
bool roadHasRoute(const Road *road, unsigned routeId);
bool roadInitFields(Road *road, RoadInfo info, City *city1, City *city2);
bool roadLink(RoadMap *roadMap, City *city1, City *city2, unsigned length, int year);
bool roadMoveTrunks(CityMap *cityMap, Trunk *trunks[ROUTE_LIMIT], Road *road);
bool roadReserve(Road *road);
bool roadUpdate(Road *road, int year);
int roadGetYear(const Road *road);
long roadWrite(char *str, const Road *road, const City *city);
unsigned roadBlock(Road *road);
unsigned roadRouteCount(const Road *road);
unsigned roadGetLength(const Road *road);
void roadDestroyTrunks(Trunk *trunks[ROUTE_LIMIT], Road *road);
void roadDetach(Road *road, const City *city);
void roadDisconnect(Road *road);
void roadFree(Road **pRoad);
void roadGetCities(Road *road, City **city1, City **city2);
void roadGetIds(const Road *road, unsigned *result);
void roadTrunkAdd(Road *road, unsigned trunkId);
void roadTrunkRemove(Road *road, unsigned trunkId);
void roadUnblock(Road *road, unsigned length);
City *roadIntersect(Road *road1, Road *road2);

bool roadMapTestCount(const RoadMap *roadMap);
bool roadMapTestTrunk(const RoadMap *roadMap, const bool *trunks);
size_t roadMapGetLength(const RoadMap *roadMap);
void roadMapDestroy(RoadMap **pRoadMap);
void roadMapTrim(RoadMap *roadMap, size_t length);
Road *const *roadMapGetSuffix(RoadMap *roadMap, size_t start);
RoadMap *roadMapInit(void);
Trunk **rebuildTrunks(CityMap *cityMap, Road *road, Trunk *trunks[ROUTE_LIMIT]);


#endif // MAP_ROAD_H
