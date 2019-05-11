#ifndef MAP_ROAD_H
#define MAP_ROAD_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

// TODO: move to road.c
struct Road {
	City *city1, *city2;
	size_t *routes;
	unsigned length;
	int year;
	size_t routeCount, routeMax;
};

bool roadReserve(Road *road, size_t length);
City *roadGetOther(Road *road, City *city);
size_t roadGetFree(const Road *road);
size_t roadRouteCount(const Road *road);
unsigned roadGetLength(const Road *road);

#endif // MAP_ROAD_H
