#ifndef MAP_CITY_MAP_H
#define MAP_CITY_MAP_H
#include <stdbool.h>
#include "global_declarations.h"

struct RoadInfo {
	const char *city1;
	const char *city2;
	int builtYear;
	unsigned length;
};

bool cityAddRoadOld(City *city, Road *road);
bool cityMakeSpace(City *city);

bool cityMapIsLast(const CityMap *cityMap, City *const *city);
City **cityMapAdd(CityMap *cityMap);
CityMap *cityMapInit(void);
size_t cityMapGetLength(const CityMap *cityMap);
void cityMapDestroy(CityMap **pCityMap);
void cityMapRemove(CityMap *cityMap);

bool roadAdjust(const City *from, const City *to, size_t length);
bool roadDestroy(CityMap *cityMap, Road *road, Trunk *trunks[1000]);
bool roadExtend(CityMap *m, Trie *t, City *city, RoadInfo info);
bool roadLink(City *city1, City *city2, unsigned length, int year);

bool roadInit(CityMap *m, Trie *t, RoadInfo info);

bool roadUpdate(Road *road, int year);
int roadGetYear(const Road *road);
Road *roadFind(const City *city1, const City *city2);
size_t roadWrite(char *str, const Road *road, const City *city);
unsigned roadBlock(Road *road);
void roadGetCities(Road *road, City **city1, City **city2);
void roadTrunkAdd(Road *road, size_t trunk);
void roadUnblock(Road *road, unsigned length);

#endif //MAP_CITY_MAP_H
