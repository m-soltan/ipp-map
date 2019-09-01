#ifndef MAP_CITY_H
#define MAP_CITY_H

#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

struct CityInfo {
	const char *name;
	CityMap *cityMap;
};

Road *cityFindRoad(City *city1, City *city2);

bool cityConnectRoad(City *city, Road *road);
bool cityMakeRoad(City *city1, City *city2, Road *road);
const char *cityGetName(const City *city);
size_t cityCopyName(char *dest, const City *city);
size_t cityGetNameLength(const City *city);
size_t cityGetRoadCount(const City *city);
void cityBlock(City *city);
void cityDestroy(City **pCity);
void cityDetach(City *city, const Road *road);
void cityDetachLast(City *city);
void cityUnblock(City *city);
City *cityAdd(CityMap *cityMap, const char *name, Road *road);
City *cityDecoy(void);
Road **cityPath(City *from, City *to, CityMap *cityMap, size_t *length);

#endif //MAP_CITY_H
