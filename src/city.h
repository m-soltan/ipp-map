#ifndef MAP_CITY_H
#define MAP_CITY_H
#include <stdbool.h>
#include "trie.h"

typedef struct City City;
typedef struct CityMap CityMap;
typedef struct Road Road;
typedef struct RoadInfo RoadInfo;
typedef struct Trie Trie;

struct RoadInfo {
	const char *city1;
	const char *city2;
	int builtYear;
	unsigned length;
};

// TODO
void debug(CityMap *m, Trie *t, const char *x, const char *y);

bool cityMakeSpace(City *city);
CityMap *cityMapInit();
bool cityAddRoad(City *city, Road *road);
City *cityInit(CityMap *map, const char *name, Road *road);

bool roadDestroy(Road *road);
bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info);
bool roadLink(City *city1, City *city2, unsigned length, int year);
bool roadInit(CityMap *m, Trie *t, RoadInfo info);
bool roadUpdate(Road *road, int year);
Road *roadFind(const City *city1, const City *city2);
void cityDetach(City *city, const Road *road);

#endif //MAP_CITY_H
