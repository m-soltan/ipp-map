#ifndef MAP_CITY_H
#define MAP_CITY_H
#include <stdbool.h>
#include "trie.h"

typedef struct City City;
typedef struct Road Road;
typedef struct RoadInfo RoadInfo;
typedef struct Trie Trie;

struct RoadInfo {
	const char *city1;
	const char *city2;
	int builtYear;
	unsigned length;
};

bool cityMakeSpace(City *city);
bool cityAddRoad(City *city, Road *road);
City *cityInit(Road *road, const char *name);

bool roadDestroy(Road *road);
bool roadExtend(Trie *t, City *city, RoadInfo info);
bool roadLink(City *city1, City *city2, unsigned length, int year);
bool roadInit(Trie *t, RoadInfo info);
bool roadUpdate(Road *road, int year);
Road *roadFind(const City *city1, const City *city2);
void cityDetach(City *city, const Road *road);

#endif //MAP_CITY_H
