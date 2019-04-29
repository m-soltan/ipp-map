#ifndef MAP_CITY_H
#define MAP_CITY_H
#include <stdbool.h>
#include "global_declarations.h"

struct RoadInfo {
	const char *city1;
	const char *city2;
	int builtYear;
	unsigned length;
};

bool cityMakeSpace(City *city);
bool cityAddRoad(City *city, Road *road);
City *cityInit(CityMap *map, const char *name, Road *road);
City **cityPath(City *from, City *to, CityMap *map, size_t *length);
size_t cityGetNameLength(const City *city);
void cityDetach(City *city, const Road *road);
void cityGetName(char *dest, const City *city);

CityMap *cityMapInit();
void cityMapDestroy(CityMap **pCityMap);

bool roadAdjust(const City *from, const City *to);
bool roadDestroy(CityMap *cityMap, Road *road, Trunk *trunks[1000]);
bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info);
bool roadLink(City *city1, City *city2, unsigned length, int year);
bool roadInit(CityMap *m, Trie *t, RoadInfo info, Trunk *trunks[1000]);
bool roadUpdate(Road *road, int year);
int roadGetYear(const Road *road);
Road *roadFind(const City *city1, const City *city2);
size_t roadWrite(char *str, const City *city1, const City *city2);
unsigned roadBlock(City *from, City *to);
void roadTrunkAdd(Road *road, size_t trunk);
void roadUnblock(Road *road, unsigned length);

#endif //MAP_CITY_H
