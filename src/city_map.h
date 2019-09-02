/** @file
 * Interface for operations on the collection of all cities in a road map.
 */

#ifndef MAP_CITY_MAP_H
#define MAP_CITY_MAP_H

#include <stdbool.h>
#include "global_declarations.h"

/// debug function, check invariants in a CityMap structure
bool cityMapTest(const CityMap *cityMap, Trie *trie);
/// get the length of the city map
size_t cityMapGetLength(const CityMap *cityMap);
/// destroy the structure
void cityMapDestroy(CityMap **pCityMap);
/// remove a number of most recently added cities
void cityMapTrim(CityMap *cityMap, size_t length);
/// add a city to the map
City *cityMapAddCity(CityInfo info, City *(*fun)(CityInfo, size_t));
/// get the suffix of the list of a given length
City *const *cityMapSuffix(CityMap *cityMap, size_t start);
/// create a CityMap structure
CityMap *cityMapInit(void);

#endif //MAP_CITY_MAP_H
