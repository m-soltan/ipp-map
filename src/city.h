#ifndef MAP_CITY_H
#define MAP_CITY_H

#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

bool cityConnectRoad(City *city, Road *road);
bool cityMakeRoad(City *city1, City *city2, Road *road);
Road *roadFind(City *city1, City *city2);
City *cityInit(CityMap *cityMap, const char *name, Road *road);

Road **cityPath(City *from, City *to, CityMap *cityMap, size_t *length);
size_t cityGetName(char *dest, const City *city);
size_t cityGetNameLength(const City *city);
size_t cityGetRoadCount(const City *city);
void cityBlock(City *city);
void cityDestroy(City **city, CityMap *cityMap);
void cityDetach(City *city, const Road *road);
void cityDetachLast(City *city);
void cityUnblock(City *city);

#endif //MAP_CITY_H
