#ifndef MAP_CITY_MAP_H
#define MAP_CITY_MAP_H

#include <stdbool.h>
#include "global_declarations.h"

bool cityMapIsLast(const CityMap *cityMap, City *const *city);
bool cityMapTest(const CityMap *cityMap, Trie *trie);
size_t cityMapGetLength(const CityMap *cityMap);
void cityMapDestroy(CityMap **pCityMap);
void cityMapTrim(CityMap *cityMap, size_t length);
City *cityMapAddCity(CityInfo info, City *(*fun)(CityInfo, size_t));
City *cityMapGetAt(CityMap *cityMap, size_t index);
City *const *cityMapSuffix(CityMap *cityMap, size_t start);
CityMap *cityMapInit(void);

#endif //MAP_CITY_MAP_H
