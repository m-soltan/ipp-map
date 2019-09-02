/** @file
 * Interface for operations on cities in a road map.
 */

#ifndef MAP_CITY_H
#define MAP_CITY_H

#include <stdbool.h>
#include <stdlib.h>

#include "global_declarations.h"

//! @cond
struct CityInfo {
	const char *name;
	CityMap *cityMap;
};
//! @endcond

/** @brief Find a road between cities.
 * @param city1 pointer to a city
 * @param city2 pointer to a city
 * @return Pointer to road if found, NULL otherwise
 */
Road *cityFindRoad(City *city1, City *city2);

/// connect the city and the road
bool cityConnectRoad(City *city, Road *road);
/// create a road between the cities
bool cityMakeRoad(City *city1, City *city2, Road *road);
/// get name of a city
const char *cityGetName(const City *city);
/// copy a name of a city
size_t cityCopyName(char *dest, const City *city);
/// return the length of the city name
size_t cityGetNameLength(const City *city);
/// return the number of roads in the city
size_t cityGetRoadCount(const City *city);
/// make the city inaccessible when searching for paths
void cityBlock(City *city);
/// destroy the city
void cityDestroy(City **pCity);
/// detach the road from the city
void cityDetach(City *city, const Road *road);
/// detach the last road from a city
void cityDetachLast(City *city);
/// make the city accessible again
void cityUnblock(City *city);
/// add a city to the map
City *cityAdd(CityMap *cityMap, const char *name, Road *road);
/// create a decoy city
City *cityDecoy(void);
/// find a path between two cities
Road **cityPath(City *from, City *to, CityMap *cityMap, size_t *length);

#endif //MAP_CITY_H
