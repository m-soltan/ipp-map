/** @file
 * Interface for operations on the roads in a road map.
 */

#ifndef MAP_ROAD_H
#define MAP_ROAD_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

/// create two cities and a road between them
bool roadLoneRoad(CityMap *cityMap, Trie *trie, RoadInfo roadInfo);
/// create one city and a road leading to it
bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info);
/// check if a road ends in a given city
bool roadHasCity(const Road *road, const City *city);
/// check if the two roads end in a common city
bool roadHasIntersection(const Road *road1, const Road *road2);
/// check if a Route with this id uses a given road
bool roadHasRoute(const Road *road, unsigned routeId);
/// write default values into a Road structure's fields
bool roadInitFields(Road *road, RoadInfo info, City *city1, City *city2);
/// connect two existing cities with a road
bool roadLink(RoadMap *roadMap, City *city1, City *city2, unsigned length, int year);
/// find a detour for every Route using this road
bool roadMoveTrunks(CityMap *cityMap, Trunk *trunks[ROUTE_LIMIT], Road *road);
/// prepare a road for holding information about the Routes using it
bool roadReserve(Road *road);
/// repair a road
bool roadUpdate(Road *road, int year);
/// get the year of a road's last repair or construction
int roadGetYear(const Road *road);
/// append a road's description to a string
long roadWrite(char *str, const Road *road, const City *city);
/// make a road invalid for searches performed on the map
unsigned roadBlock(Road *road);
/// get the Route count of a road
unsigned roadRouteCount(const Road *road);
/// get the length of a road
unsigned roadGetLength(const Road *road);
/// destroy all Routes through a given road
void roadDestroyTrunks(Trunk *trunks[ROUTE_LIMIT], Road *road);
/// remove a road from the records of adjacent cities
void roadDetach(Road *road, const City *city);
/// destroy a road
void roadFree(Road **pRoad);
/// get cities on both ends of road
void roadGetCities(Road *road, City **city1, City **city2);
/// get ids of all trunks using the road
void roadGetIds(const Road *road, unsigned *result);
/// add a road to a Route
void roadTrunkAdd(Road *road, unsigned trunkId);
/// remove a road from a Route
void roadTrunkRemove(Road *road, unsigned trunkId);
/// make a road valid for searches performed on the map
void roadUnblock(Road *road, unsigned length);
/// find a city shared by two roads
City *roadIntersect(Road *road1, Road *road2);
/// debug function, check invariant related to road usage by Routes
bool roadMapTestCount(const RoadMap *roadMap);
/// debug function, check trunk-related invariants in a road map
bool roadMapTestTrunk(const RoadMap *roadMap, const bool *trunks);
/// get the number of roads in a map
size_t roadMapGetLength(const RoadMap *roadMap);
/// destroy a RoadMap structure
void roadMapDestroy(RoadMap **pRoadMap);
/// remove the most recently added roads
void roadMapTrim(RoadMap *roadMap, size_t length);
/// initialize a RoadMap structure
RoadMap *roadMapInit(void);
/// create detours for all trunks affected by road removal
Trunk **rebuildTrunks(CityMap *cityMap, Road *road, Trunk *trunks[ROUTE_LIMIT]);


#endif // MAP_ROAD_H
