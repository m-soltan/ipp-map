#ifndef MAP_CITY_MAP_H
#define MAP_CITY_MAP_H
#include <stdbool.h>
#include "global_declarations.h"

bool cityMakeSpace(City *city);

bool cityMapIsLast(const CityMap *cityMap, City *const *city);
City **cityMapAdd(CityMap *cityMap);
CityMap *cityMapInit(void);
size_t cityMapGetLength(const CityMap *cityMap);
void cityMapDestroy(CityMap **pCityMap);
void cityMapRemove(CityMap *cityMap);

bool roadAdjust(City *from, City *to, unsigned length);

#endif //MAP_CITY_MAP_H
